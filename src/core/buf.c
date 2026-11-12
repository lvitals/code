#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "buf.h"

struct buf *B;
#define LOST		((struct undo *)B)	/* a saved no undo reaches */

#define CHUNK		((pos_t)65536)	/* the most bytes of a piece */
#define BLOCK		((pos_t)1 << 20)	/* the add buffer grows by */
#define MAP_MIN		((pos_t)32 << 20)	/* a file mapped, not read */
#define SAMPLE		((pos_t)65536)	/* what decides LF or CRLF there */
#define NL_UNK		POS_NONE	/* newlines not counted yet */
#define DEL_COUNT	((pos_t)1 << 20)	/* the most deleted bytes counted */

/* --- the pieces: a treap by position ------------------------------------ */

struct piece {
	struct piece *l, *r;
	const char *p;		/* its bytes */
	pos_t len, sum;		/* their count; with the subtree */
	pos_t nl, nlsum;	/* its newlines (NL_UNK); those counted of the
				   subtree */
	pos_t unk;		/* the bytes of the subtree not counted */
	unsigned long pri;	/* above those of its subtree */
};

/* the bytes the pieces refer to: a mapped file, or a block of the add
 * buffer (only appended to: what is there never changes) */
struct pblock {
	struct pblock *next;
	char *p;
	pos_t size, used;
	char mapped;
};

#define SUM(t)		((t) ? (t)->sum : 0)
#define NLS(t)		((t) ? (t)->nlsum : 0)
#define UNK(t)		((t) ? (t)->unk : 0)

static struct piece *spare;	/* the nodes reserved for the next edit */
static int nspare;

static unsigned long
rnd(void)
{
	static unsigned long s = 2463534242UL;

	s ^= (s << 13) & 0xFFFFFFFFUL;
	s ^= s >> 17;
	s ^= (s << 5) & 0xFFFFFFFFUL;
	return s;
}

/* --- eight bytes at a time (SWAR) ---------------------------------------- */

#define ONES		((uint64_t)0x0101010101010101ULL)
#define LOW7		((uint64_t)0x7F7F7F7F7F7F7F7FULL)
#define HIGH		((uint64_t)0x8080808080808080ULL)

static uint64_t
load8(const char *p)
{
	uint64_t w;

	memcpy(&w, p, 8);
	return w;
}

/* the bytes of w that are zero: 80h in them, 0 in the others */
static uint64_t
zeros8(uint64_t w)
{
	return ~(((w & LOW7) + LOW7) | w | LOW7);
}

/* the sum of the bytes of a (each at most 255 / 8 apart: 16-bit lanes) */
static pos_t
bytesum(uint64_t a)
{
	a = (a & 0x00FF00FF00FF00FFULL) + ((a >> 8) & 0x00FF00FF00FF00FFULL);
	return (pos_t)((a * 0x0001000100010001ULL) >> 48);
}

/* the bytes of [p, p + n) that some test of eight marks (80h each) */
#define SWARCOUNT(p, n, mark, one)	do {				\
	const char *e_ = (p) + (n);					\
	uint64_t acc_;							\
	int k_;								\
	while(e_ - (p) >= 8) {						\
		acc_ = 0;						\
		for(k_ = 0; k_ < 255 && e_ - (p) >= 8; k_++, (p) += 8)	\
			acc_ += (mark(load8(p))) >> 7;			\
		r_ += bytesum(acc_);					\
	}								\
	for(; (p) < e_; (p)++)						\
		r_ += one(*(p));					\
} while(0)

#define NLMARK(w)	zeros8((w) ^ (ONES * '\n'))
#define NLONE(c)	((c) == '\n')
/* a continuation byte (10xxxxxx): not a character of its own */
#define CONTMARK(w)	((w) & HIGH & ~((w) << 1))
#define CONTONE(c)	(((unsigned char)(c) & 0xC0) == 0x80)

static pos_t
memcount(const char *p, pos_t n)
{
	pos_t r_ = 0;

	SWARCOUNT(p, n, NLMARK, NLONE);
	return r_;
}

static pos_t
contcount(const char *p, pos_t n)
{
	pos_t r_ = 0;

	SWARCOUNT(p, n, CONTMARK, CONTONE);
	return r_;
}

/* the printable ASCII bytes from p (20h to 7Eh), up to n */
static pos_t
plainlen(const char *p, pos_t n)
{
	const char *s = p, *e = p + n;
	uint64_t w;
	unsigned char c;

	for(; e - p >= 8; p += 8) {
		w = load8(p);
		/* a byte >= 80h, < 20h, or 7Fh */
		if((w & HIGH) | zeros8(w & (ONES * 0xE0)) | zeros8(w ^ (ONES * 0x7F)))
			break;
	}
	for(; p < e && (c = (unsigned char)*p) >= 0x20 && c < 0x7F; p++)
		;
	return (pos_t)(p - s);
}

static void
update(struct piece *t)
{
	t->sum = t->len + SUM(t->l) + SUM(t->r);
	t->unk = (t->nl == NL_UNK ? t->len : 0) + UNK(t->l) + UNK(t->r);
	t->nlsum = (t->nl == NL_UNK ? 0 : t->nl) + NLS(t->l) + NLS(t->r);
}

static pos_t
piece_nl(struct piece *t)
{
	if(t->nl == NL_UNK)
		t->nl = memcount(t->p, t->len);
	return t->nl;
}

/* the newlines of the subtree, all counted */
static void
force(struct piece *t)
{
	if(t == 0 || t->unk == 0)
		return;
	force(t->l);
	force(t->r);
	piece_nl(t);
	update(t);
}

static void
free_tree(struct piece *t)
{
	if(t == 0)
		return;
	free_tree(t->l);
	free_tree(t->r);
	free(t);
}

static struct piece *
node(const char *p, pos_t len, pos_t nl)
{
	struct piece *t = spare;

	spare = t->l;
	nspare--;
	t->l = t->r = 0;
	t->p = p;
	t->len = len;
	t->nl = nl;
	t->pri = rnd();
	update(t);
	return t;
}

/* a and b side by side (all of a before b) */
static struct piece *
merge(struct piece *a, struct piece *b)
{
	if(a == 0)
		return b;
	if(b == 0)
		return a;
	if(a->pri >= b->pri) {
		a->r = merge(a->r, b);
		update(a);
		return a;
	}
	b->l = merge(a, b->l);
	update(b);
	return b;
}

/* t as its first k bytes (a) and the rest (b); a piece cut in two takes
 * one node of the spare ones */
static void
split(struct piece *t, pos_t k, struct piece **a, struct piece **b)
{
	struct piece *n;
	pos_t ls, c;

	if(t == 0) {
		*a = *b = 0;
		return;
	}
	ls = SUM(t->l);
	if(k <= ls) {
		split(t->l, k, a, &t->l);
		update(t);
		*b = t;
	} else if(k >= ls + t->len) {
		split(t->r, k - ls - t->len, &t->r, b);
		update(t);
		*a = t;
	} else {
		k -= ls;
		n = node(t->p + k, t->len - k, NL_UNK);
		if(t->nl != NL_UNK) {	/* the shorter side is counted */
			if(k < t->len / 2) {
				c = memcount(t->p, k);
				n->nl = t->nl - c;
				t->nl = c;
			} else {
				n->nl = memcount(t->p + k, t->len - k);
				t->nl -= n->nl;
			}
		}
		n->pri = t->pri;
		n->r = t->r;
		t->r = 0;
		t->len = k;
		update(t);
		update(n);
		*a = t;
		*b = n;
	}
}

/* t with its last piece n bytes longer (nl newlines more) */
static void
grow_last(struct piece *t, pos_t n, pos_t nl)
{
	if(t->r)
		grow_last(t->r, n, nl);
	else {
		t->len += n;
		if(t->nl != NL_UNK)
			t->nl += nl;
	}
	update(t);
}

static struct piece *
last_piece(struct piece *t)
{
	while(t && t->r)
		t = t->r;
	return t;
}

/* the piece of byte i (i < the length): where it starts */
static struct piece *
locate(pos_t i, pos_t *start)
{
	struct piece *t = B->root;
	pos_t base = 0, ls;

	while(t) {
		ls = SUM(t->l);
		if(i < ls)
			t = t->l;
		else if(i < ls + t->len) {
			*start = base + ls;
			return t;
		} else {
			base += ls + t->len;
			i -= ls + t->len;
			t = t->r;
		}
	}
	return 0;
}

int
buf_at(pos_t i)
{
	struct piece *t;
	pos_t s;

	if(i >= B->len || (t = locate(i, &s)) == 0)
		return 0;
	B->cp = t->p;
	B->cs = s;
	B->cl = t->len;
	return (unsigned char)t->p[i - s];
}

/* the bytes from i to the end of its piece: their count, at *p */
static pos_t
span(pos_t i, const char **p)
{
	if(i - B->cs >= B->cl) {
		if(i >= B->len)
			return 0;
		buf_at(i);
	}
	*p = B->cp + (i - B->cs);
	return B->cs + B->cl - i;
}

/* the bytes before i in the piece of byte i - 1: their count, at *p */
static pos_t
rspan(pos_t i, const char **p)
{
	if(i == 0)
		return 0;
	if(i - 1 - B->cs >= B->cl)
		buf_at(i - 1);
	*p = B->cp;
	return i - B->cs;
}

/* --- the memory ---------------------------------------------------------- */

static void
droplist(struct undo **l)
{
	struct undo *r, *n;

	for(r = *l; r; r = n) {
		n = r->next;
		if(r == B->saved)
			B->saved = LOST;
		free_tree(r->other);
		free(r);
	}
	*l = 0;
}

/* the oldest undo goes, for memory; 0 if there was none */
static int
dropoldest(void)
{
	struct undo **pp = &B->undo;

	if(*pp == 0) {
		if(B->redo == 0)
			return 0;
		droplist(&B->redo);
		return 1;
	}
	while((*pp)->next)
		pp = &(*pp)->next;
	if(*pp == B->saved)
		B->saved = *pp == B->undo ? 0 : LOST;	/* 0: it is the text as saved */
	free_tree((*pp)->other);
	free(*pp);
	*pp = 0;
	return 1;
}

static void *
alloc(size_t n)
{
	void *p;

	while((p = malloc(n)) == 0)
		if(!dropoldest())
			return 0;
	return p;
}

/* n spare nodes at least */
static int
reserve(int n)
{
	struct piece *t;

	while(nspare < n) {
		if((t = alloc(sizeof *t)) == 0)
			return 0;
		t->l = spare;
		spare = t;
		nspare++;
	}
	return 1;
}

/* the n bytes of s in the add buffer: where they are */
static const char *
store(const char *s, pos_t n)
{
	struct pblock *b = B->blocks;
	pos_t size;
	char *d;

	if(b == 0 || b->mapped || b->size - b->used < n) {
		size = n > BLOCK ? n : BLOCK;
		if((b = alloc(sizeof *b)) == 0)
			return 0;
		if((b->p = alloc((size_t)size)) == 0) {
			free(b);
			return 0;
		}
		b->size = size;
		b->used = 0;
		b->mapped = 0;
		b->next = B->blocks;
		B->blocks = b;
	}
	d = b->p + b->used;
	memcpy(d, s, (size_t)n);
	b->used += n;
	return d;
}

/* the n bytes at d (just stored) after the pieces of a: its last piece
 * grown when they follow it, else new ones (reserved: n / CHUNK + 1) */
static struct piece *
append(struct piece *a, const char *d, pos_t n, pos_t nl)
{
	struct piece *l = last_piece(a);
	pos_t k;

	if(n == 0)
		return a;
	if(l && l->p + l->len == d && d != B->blocks->p && l->len + n <= CHUNK) {
		grow_last(a, n, nl);
		return a;
	}
	for(; n; d += k, n -= k) {
		k = n < CHUNK ? n : CHUNK;
		a = merge(a, node(d, k, memcount(d, k)));
	}
	return a;
}

/* --- the text and its undo ----------------------------------------------- */

static void
invalidate(pos_t off)
{
	B->cl = 0;
	B->ln_off = POS_NONE;
	if(B->ls_hi != POS_NONE) {
		if(off < B->ls_lo)
			B->ls_hi = POS_NONE;
		else if(off < B->ls_hi)
			B->ls_hi = off;	/* no newline still before off */
	}
}

void
buf_init(void)
{
	memset(B, 0, sizeof *B);
	B->crlf = (char)file_crlf;
	B->tabsize = 4;
	B->ls_hi = POS_NONE;
	B->ln_off = POS_NONE;
}

void
buf_free(void)
{
	struct pblock *b, *n;
	char t, ts;

	droplist(&B->undo);
	droplist(&B->redo);
	free_tree(B->root);
	for(b = B->blocks; b; b = n) {
		n = b->next;
		if(b->mapped)
			file_unmap(b->p, b->size);
		else
			free(b->p);
		free(b);
	}
	t = B->tabs;		/* the indentation the document had: kept */
	ts = B->tabsize;
	buf_init();
	B->tabs = t;
	B->tabsize = ts;
}

void
buf_copy(pos_t off, pos_t n, char *dst)
{
	const char *p;
	pos_t k;

	while(n > 0 && (k = span(off, &p)) > 0) {
		if(k > n)
			k = n;
		memcpy(dst, p, (size_t)k);
		dst += k;
		off += k;
		n -= k;
	}
}

void
buf_touch(void)
{
	B->saved = LOST;
}

void
buf_begin(void)
{
	B->seq++;
	B->merge = 0;
}

int
buf_replace(pos_t off, pos_t ndel, const char *s, pos_t n, pos_t cur, int kind)
{
	struct undo *u, *r = 0;
	struct piece *a, *m, *c;
	const char *d = 0;
	pos_t nl;
	int merged;

	if(ndel == 0 && n == 0)
		return 1;
	if(off > B->len || ndel > B->len - off)
		return 0;
	nl = memcount(s, n);
	/* the memory first: once the text is cut, nothing can fail */
	if((n && (d = store(s, n)) == 0) || !reserve(2 + (int)(n / CHUNK) + 1))
		return 0;
	r = alloc(sizeof *r);	/* 0: no undo for it */
	droplist(&B->redo);
	u = B->undo;
	merged = kind == U_TYPE && B->merge && u && u->kind == U_TYPE && u != B->saved &&
	    ndel == 0 && u->off + u->nins == off;
	split(B->root, off, &a, &m);
	split(m, ndel, &m, &c);
	B->root = merge(append(a, d, n, nl), c);
	B->len = B->len - ndel + n;
	if(ndel > DEL_COUNT)
		B->dlines = DL_MANY;
	else {
		force(m);
		B->dlines = (long)nl - (long)NLS(m);
	}
	invalidate(off);
	if(merged) {
		free(r);
		u->nins += n;
	} else if(r) {
		if(kind == U_TYPE)
			B->seq++;
		r->off = off;
		r->ndel = ndel;
		r->nins = n;
		r->cur = cur;
		r->seq = B->seq;
		r->kind = (char)kind;
		r->other = m;
		r->next = B->undo;
		B->undo = r;
		m = 0;
	} else
		B->saved = LOST;	/* no undo: the save is past reach */
	free_tree(m);
	B->merge = kind == U_TYPE && !(n == 1 && s[0] == '\n');
	return 1;
}

pos_t
buf_undo(int redo)
{
	struct undo **from = redo ? &B->redo : &B->undo;
	struct undo **to = redo ? &B->undo : &B->redo;
	struct undo *r;
	struct piece *a, *m, *c;
	pos_t cur = POS_NONE, n, k;
	unsigned seq;

	B->merge = 0;
	if(*from == 0)
		return POS_NONE;
	seq = (*from)->seq;
	while((r = *from) != 0 && r->seq == seq) {
		n = redo ? r->ndel : r->nins;	/* what goes, what comes */
		k = redo ? r->nins : r->ndel;
		if(!reserve(2))
			break;
		split(B->root, r->off, &a, &m);
		split(m, n, &m, &c);
		B->root = merge(merge(a, r->other), c);
		B->len = B->len - n + k;
		r->other = m;
		invalidate(r->off);
		cur = redo ? r->off + r->nins : r->cur;
		*from = r->next;
		r->next = *to;
		*to = r;
	}
	B->dlines = DL_MANY;
	return cur;
}

/* --- the file ------------------------------------------------------------ */

/* the indentation of the text, as guessed from its first lines */
static void
detect(void)
{
	pos_t i = 0, len = buf_len();
	int lines = 0, tabl = 0, spl = 0, sp, k, max = 1, size = 4;
	int hist[7];

	if(len > SAMPLE * 16)
		len = SAMPLE * 16;
	memset(hist, 0, sizeof hist);
	while(i < len && lines < 1000) {
		if(BCH(i) == '\t')
			tabl++;
		else {
			for(sp = 0; sp < 9 && i + sp < len && BCH(i + sp) == ' '; sp++)
				;
			if(sp >= 2 && sp <= 8) {
				spl++;
				hist[sp - 2]++;
				if(sp & 4)
					hist[0]++;
				if(sp == 6 || sp == 8)
					hist[sp / 2 - 2]++;
			}
		}
		i = buf_lineend(i) + 1;
		lines++;
	}
	if(tabl + spl == 0)
		return;		/* no indentation: the document's stays */
	B->detected = 1;
	B->tabs = tabl > spl;
	if(B->tabs)
		return;		/* as wide as the document's (the settings') */
	for(k = 0; k < 7; k++)
		if(hist[k] >= max) {
			max = hist[k];
			size = k + 2;
		}
	B->tabsize = (char)size;
}

/* the reading of a file: CR LF becomes LF, a CR alone stays */
struct conv {
	char cr;
	pos_t lf, crlf;
};

static int
convert(struct conv *v, const char *in, pos_t n, char *out, pos_t *o)
{
	pos_t i;
	char c;

	*o = 0;
	for(i = 0; i < n; i++) {
		c = in[i];
		if(v->cr) {
			v->cr = 0;
			if(c == '\n') {
				v->crlf++;
				out[(*o)++] = '\n';
				continue;
			}
			out[(*o)++] = '\r';
		}
		if(c == '\r') {
			v->cr = 1;
			continue;
		}
		if(c == '\n')
			v->lf++;
		out[(*o)++] = c;
	}
	return 1;
}

/* the n bytes of s at the end of the text, while loading */
static int
load_add(const char *s, pos_t n)
{
	const char *d;

	if(n == 0)
		return 1;
	if((d = store(s, n)) == 0 || !reserve((int)(n / CHUNK) + 1))
		return 0;
	B->root = append(B->root, d, n, memcount(s, n));
	B->len += n;
	return 1;
}

/* a file read: its bytes decoded (but UTF-8), then its newlines */
struct ld {
	struct conv v;
	struct dec d;
	struct u8check u;
	char check;		/* UTF-8 guessed: checked as it is read */
	char guess8;		/* an 8-bit one guessed: Windows-1252, or
				   ISO-8859-1 when no byte says otherwise */
	char bom;		/* the file started with its BOM */
};

#define ISUTF8(e)	((e) == ENC_UTF8 || (e) == ENC_UTF8_BOM)
enum { L_RETRY = -1 };	/* not UTF-8 after all: read again */

/* n bytes of the file (no more than CHUNK); last: the end of it */
static int
feed(struct ld *L, const char *in, pos_t n, int last)
{
	static char dbuf[DEC_ROOM(CHUNK)], obuf[DEC_ROOM(CHUNK)];
	pos_t o;

	if(L->check && !u8_check(&L->u, in, n, last))
		return L_RETRY;
	if(!ISUTF8(L->d.enc)) {
		n = dec_run(&L->d, in, n, dbuf, last);
		in = dbuf;
	}
	convert(&L->v, in, n, obuf, &o);
	if(!load_add(obuf, o))
		return B_NOMEM;
	if(last && L->v.cr && !load_add("\r", 1))
		return B_NOMEM;
	return B_OK;
}

/* the end of a read: what the file was in, its newlines */
static void
loaded(struct ld *L)
{
	struct conv *v = &L->v;

	B->enc = (char)(L->guess8 && !L->d.c1 ? ENC_LATIN1 : L->d.enc);
	B->nobom = ENC_UTF16(B->enc) && !L->bom;
	B->crlf = v->lf + v->crlf == 0 ? (char)file_crlf : v->crlf > (v->lf + v->crlf) / 2;
}

/* the encoding of a file, from its first n bytes at p (asked for: enc;
 * else guessed), the bytes of its BOM to *skip */
static void
pick(struct ld *L, const char *p, pos_t n, int enc, int *skip)
{
	int e = enc_sniff_bom(p, n, skip);

	if(enc != ENC_AUTO && e != enc)
		*skip = 0;	/* only its own BOM is skipped */
	L->bom = *skip > 0;
	if(enc != ENC_AUTO)
		L->d.enc = enc;
	else if(e != ENC_AUTO)
		L->d.enc = e;
	else if((e = enc_sniff_utf16(p, n)) != ENC_AUTO)
		L->d.enc = e;
	else {
		L->d.enc = ENC_UTF8;
		L->check = 1;
	}
}

static char keep_crlf;		/* buf_load_search: CR LF not converted, and
				   no snapshot of a file mapped */

/* a mapped file in UTF-8 from skip: its bytes are the pieces, nothing is
 * copied; one with CR LF newlines is converted (copied), but for a
 * search */
static int
load_mapped(char *m, pos_t size, pos_t skip)
{
	static char out[CHUNK];
	struct conv v;
	struct pblock *b;
	pos_t i, k, o, n = size - skip;
	const char *t = m + skip;

	memset(&v, 0, sizeof v);
	convert(&v, t, n < SAMPLE ? n : SAMPLE, out, &o);
	if(!keep_crlf && v.crlf > (v.lf + v.crlf) / 2) {
		memset(&v, 0, sizeof v);
		for(i = 0; i < n; i += k) {
			k = n - i < CHUNK ? n - i : CHUNK;
			convert(&v, t + i, k, out, &o);
			if(!load_add(out, o)) {
				file_unmap(m, size);
				return B_NOMEM;
			}
		}
		if(v.cr && !load_add("\r", 1)) {
			file_unmap(m, size);
			return B_NOMEM;
		}
		file_unmap(m, size);
		B->crlf = 1;
		return B_OK;
	}
	if((b = alloc(sizeof *b)) == 0) {
		file_unmap(m, size);
		return B_NOMEM;
	}
	b->p = m;
	b->size = b->used = size;
	b->mapped = 1;
	b->next = B->blocks;
	B->blocks = b;
	B->mapped = 1;
	for(i = 0; i < n; i += k) {
		k = n - i < CHUNK ? n - i : CHUNK;
		if(!reserve(1))
			return B_NOMEM;
		B->root = merge(B->root, node(t + i, k, NL_UNK));
		B->len += k;
	}
	B->crlf = v.lf + v.crlf == 0 ? (char)file_crlf : 0;
	return B_OK;
}

/* a large file, mapped: UTF-8 is guessed from its first bytes alone (the
 * rest is not read); one in another encoding is decoded (copied) */
static int
load_map(char *m, pos_t size, int enc)
{
	struct ld L;
	pos_t i, k, s = size < SAMPLE ? size : SAMPLE;
	int skip, r = B_OK;

	memset(&L, 0, sizeof L);
	pick(&L, m, s, enc, &skip);
	if(L.check && !u8_check(&L.u, m, s, s == size)) {
		/* CJK, Cyrillic: from its first bytes; else an 8-bit one */
		if((L.d.enc = enc_detect(m, s)) == ENC_AUTO) {
			L.d.enc = ENC_CP1252;
			L.guess8 = 1;
		}
	}
	if(ISUTF8(L.d.enc)) {
		B->enc = (char)L.d.enc;
		return load_mapped(m, size, skip);
	}
	L.check = 0;
	for(i = skip; i < size && r == B_OK; i += k) {
		k = size - i < CHUNK ? size - i : CHUNK;
		r = feed(&L, m + i, k, 0);
	}
	if(r == B_OK)
		r = feed(&L, "", 0, 1);
	file_unmap(m, size);
	loaded(&L);
	return r;
}

/* a file read a piece at a time */
static int
load_read(int h, int enc, int guess8)
{
	static char in[CHUNK];
	struct ld L;
	int n, skip, first = 1, r = B_OK;

	memset(&L, 0, sizeof L);
	L.d.enc = enc == ENC_AUTO ? ENC_UTF8 : enc;	/* an empty file */
	L.guess8 = (char)guess8;
	while(r == B_OK && (n = file_read(h, in, (int)sizeof in)) > 0) {
		skip = 0;
		if(first)
			pick(&L, in, (pos_t)n, enc, &skip);
		first = 0;
		r = feed(&L, in + skip, (pos_t)(n - skip), 0);
	}
	if(r == B_OK)
		r = feed(&L, "", 0, 1);
	loaded(&L);
	return r;
}

int
buf_load(const char *name, int enc)
{
	static char sample[SAMPLE];
	pos_t size = POS_NONE;
	char *m;
	int h, r, n, e;

	h = file_open(name, 0);
	if(h < 0)
		return B_NOFILE;
	buf_free();
	B->stamp = file_stamp(name);
	size = file_size(h);
	if(size != POS_NONE && size >= MAP_MIN && (m = file_map(h, size, !keep_crlf)) != 0) {
		file_close(h);
		r = load_map(m, size, enc);
		detect();
		return r;
	}
	r = load_read(h, enc, 0);
	file_close(h);
	if(r == L_RETRY) {	/* not UTF-8: CJK, Cyrillic (from its first
				   bytes); else an 8-bit encoding of the West */
		buf_free();
		if((h = file_open(name, 0)) < 0)
			return B_NOFILE;
		n = file_read(h, sample, (int)sizeof sample);
		file_close(h);
		e = n > 0 ? enc_detect(sample, (pos_t)n) : ENC_AUTO;
		if((h = file_open(name, 0)) < 0)
			return B_NOFILE;
		r = load_read(h, e == ENC_AUTO ? ENC_CP1252 : e, e == ENC_AUTO);
		file_close(h);
	}
	detect();
	return r;
}

int
buf_load_search(const char *name)
{
	int r;

	keep_crlf = 1;
	r = buf_load(name, ENC_AUTO);
	keep_crlf = 0;
	return r;
}

/* the writing of a file: its bytes encoded (but UTF-8) */
static struct encd E;

static int
put_out(int h, const char *p, int n)
{
	static char ebuf[4 * 2 * 4096 + 8];
	pos_t k;

	if(!ISUTF8(E.enc)) {
		k = enc_run(&E, p, n, ebuf);
		p = ebuf;
		n = (int)k;
	}
	return n == 0 || file_write(h, p, n) == n;
}

/* the pieces of t, in order, to the file */
static int
save_tree(int h, struct piece *t)
{
	static char out[2 * 4096];
	const char *p, *e;
	int o;

	for(; t; t = t->r) {
		if(!save_tree(h, t->l))
			return 0;
		p = t->p;
		e = p + t->len;
		if(!B->crlf) {
			for(; p < e; p += o) {
				o = e - p < 4096 ? (int)(e - p) : 4096;
				if(!put_out(h, p, o))
					return 0;
			}
			continue;
		}
		while(p < e) {	/* LF as CR LF */
			for(o = 0; p < e && o < 4096; p++) {
				if(*p == '\n')
					out[o++] = '\r';
				out[o++] = *p;
			}
			if(!put_out(h, out, o))
				return 0;
		}
	}
	return 1;
}

pos_t buf_bad;

/* the first character of the text not in e (or not UTF-8): POS_NONE if
 * none is */
static pos_t
unencodable(int e)
{
	pos_t i = 0, len = B->len;
	struct u8check u;
	char s[4], ascii[128];
	unsigned c;
	unsigned long cp;
	int n, k;

	for(k = 0; k < 128; k++)	/* not all of ASCII is in each (EBCDIC) */
		ascii[k] = (char)(!ENC_EXT(e) || enc_has(e, (unsigned long)k));
	while(i < len) {
		if((c = (unsigned)BCH(i)) < 0x80) {
			if(!ascii[c])
				return i;
			i++;
			continue;
		}
		n = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
		if(i + n > len)
			return i;
		for(k = 0; k < n; k++)
			s[k] = (char)BCH(i + k);
		memset(&u, 0, sizeof u);
		if(!u8_check(&u, s, n, 1))
			return i;
		cp = c & (0x7F >> n);
		for(k = 1; k < n; k++)
			cp = cp << 6 | ((unsigned char)s[k] & 0x3F);
		if(!enc_has(e, cp))
			return i;
		i += n;
	}
	return POS_NONE;
}

int
buf_save(const char *name)
{
	const char *bom;
	int h, ok, nb;

	if(!ISUTF8(B->enc) && (buf_bad = unencodable(B->enc)) != POS_NONE)
		return B_ENCODE;
	h = file_open(name, 1);
	if(h < 0)
		return B_NOFILE;
	memset(&E, 0, sizeof E);
	E.enc = B->enc;
	nb = B->nobom ? 0 : enc_bom(B->enc, &bom);
	ok = nb == 0 || file_write(h, bom, nb) == nb;
	ok = ok && save_tree(h, B->root);
	{	/* the end of a stateful one (ISO-2022-JP, UTF-7, HZ) */
		char end[16];
		int ne = (int)enc_end(&E, end);

		ok = ok && (ne == 0 || file_write(h, end, ne) == ne);
	}
	if(file_close(h) < 0)
		ok = 0;
	if(!ok)
		return B_WRITE;
	B->stamp = file_stamp(name);
	B->saved = B->undo;
	B->merge = 0;
	return B_OK;
}

/* --- the characters ------------------------------------------------------ */

pos_t
buf_next(pos_t off)
{
	pos_t i = off, len = buf_len();

	if(i >= len)
		return len;
	i++;
	if(BCH(i - 1) >= 0xC0)
		while(i < len && i < off + 4 && (BCH(i) & 0xC0) == 0x80)
			i++;
	return i;
}

pos_t
buf_prev(pos_t off)
{
	pos_t i = off;

	if(i == 0)
		return 0;
	i--;
	while(i > 0 && i + 3 > off && (BCH(i) & 0xC0) == 0x80)
		i--;
	if(buf_next(i) != off)
		return off - 1;	/* not UTF-8: a byte alone */
	return i;
}

rune
buf_char(pos_t off)
{
	unsigned c = BCH(off);
	pos_t i, len = buf_len();
	int n;
	rune cp;

	if(c < 0x80)
		return c;
	if(c >= 0xF0 && c < 0xF5) {
		n = 3;
		cp = c & 7;
	} else if(c >= 0xE0) {
		n = 2;
		cp = c & 15;
	} else if(c >= 0xC2) {
		n = 1;
		cp = c & 31;
	} else
		return 0xFFFD;
	for(i = off + 1; n; n--, i++) {
		if(i >= len || (BCH(i) & 0xC0) != 0x80)
			return 0xFFFD;
		cp = cp << 6 | (BCH(i) & 0x3F);
	}
	return cp;
}

int
isword(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
	    (c >= '0' && c <= '9') || c == '_' || c >= 0x80;
}

static int
class(int c)
{
	if(c == '\n')
		return 3;
	if(c == ' ' || c == '\t')
		return 0;
	return isword(c) ? 1 : 2;
}

/* the class of the character before i (back) or at i, 4 past the text */
static int
side(pos_t i, int back)
{
	if(back ? i == 0 : i >= buf_len())
		return 4;
	return class(BCH(back ? i - 1 : i));
}

pos_t
buf_word(pos_t off, int dir)
{
	pos_t i = off;
	int k, back = dir < 0, step = back ? -1 : 1;

	/* the blanks, then a run of one class; a newline alone */
	while(side(i, back) == 0)
		i += step;
	k = side(i, back);
	if(k == 4)
		return i;
	if(k == 3)
		return i != off ? i : i + step;
	while(side(i, back) == k)
		i += step;
	return i;
}

/* --- the lines ----------------------------------------------------------- */

static const char *
rmemchr(const char *s, int c, pos_t n)
{
	const char *p = s + n;

	while(p > s)
		if(*--p == (char)c)
			return p;
	return 0;
}

/* the first byte after the last newline of [lim, off): POS_NONE if none */
static pos_t
scan_back(pos_t off, pos_t lim)
{
	const char *p, *q;
	pos_t n;

	while(off > lim) {
		n = rspan(off, &p);
		if(off - n < lim) {
			p += lim - (off - n);
			n = off - lim;
		}
		if((q = rmemchr(p, '\n', n)) != 0)
			return off - n + (pos_t)(q - p) + 1;
		off -= n;
	}
	return POS_NONE;
}

/* A long line is not read again: the start of the last line asked for is
 * kept with how far it is known to go on (an edit after it cuts that) */
pos_t
buf_linestart(pos_t off)
{
	pos_t p;

	if(off > B->len)
		off = B->len;
	if(B->ls_hi != POS_NONE && off >= B->ls_lo) {
		if(off <= B->ls_hi)
			return B->ls_lo;
		if((p = scan_back(off, B->ls_hi)) == POS_NONE) {
			B->ls_hi = off;
			return B->ls_lo;
		}
	} else if((p = scan_back(off, 0)) == POS_NONE)
		p = 0;
	B->ls_lo = p;
	B->ls_hi = off;
	return p;
}

pos_t
buf_lineend(pos_t off)
{
	const char *p, *q;
	pos_t n;

	while((n = span(off, &p)) > 0) {
		if((q = memchr(p, '\n', (size_t)n)) != 0)
			return off + (pos_t)(q - p);
		off += n;
	}
	return B->len;
}

pos_t
buf_count(pos_t a, pos_t b, pos_t max)
{
	const char *p, *q, *e;
	pos_t n, k = 0;

	while(a < b && k < max && (n = span(a, &p)) > 0) {
		if(n > b - a)
			n = b - a;
		for(e = p + n; k < max && (q = memchr(p, '\n', (size_t)(e - p))) != 0; p = q + 1)
			k++;
		a += n;
	}
	return k;
}

/* the newlines before byte k: the subtrees before it counted (once) */
static pos_t
nl_before(pos_t k)
{
	struct piece *t = B->root;
	pos_t n = 0, ls;

	while(t) {
		ls = SUM(t->l);
		if(k <= ls) {
			t = t->l;
			continue;
		}
		force(t->l);
		n += NLS(t->l);
		k -= ls;
		if(k < t->len) {
			if(t->nl != NL_UNK && k > t->len / 2)
				return n + t->nl - memcount(t->p + k, t->len - k);
			return n + memcount(t->p, k);
		}
		n += piece_nl(t);
		k -= t->len;
		t = t->r;
	}
	return n;
}

pos_t
buf_lineno(pos_t off)
{
	pos_t ls = buf_linestart(off);

	if(ls != B->ln_off) {
		B->ln_no = nl_before(ls);
		B->ln_off = ls;
	}
	return B->ln_no;
}

pos_t
buf_lineoff(pos_t line)
{
	struct piece *t = B->root;
	pos_t k = line, base = 0, nl;
	const char *p, *q;

	if(line == 0)
		return 0;
	if(line == B->ln_no && B->ln_off != POS_NONE)
		return B->ln_off;
	while(t) {
		force(t->l);
		if(k <= NLS(t->l)) {
			t = t->l;
			continue;
		}
		k -= NLS(t->l);
		base += SUM(t->l);
		nl = piece_nl(t);
		if(k <= nl) {	/* the k-th newline of the piece */
			for(p = t->p; (q = memchr(p, '\n', (size_t)(t->p + t->len - p))) != 0; p = q + 1)
				if(--k == 0) {
					B->ln_off = base + (pos_t)(q - t->p) + 1;
					B->ln_no = line;
					return B->ln_off;
				}
			break;
		}
		k -= nl;
		base += t->len;
		t = t->r;
	}
	return buf_linestart(B->len);	/* past the last: the last */
}

pos_t
buf_lines(void)
{
	force(B->root);
	return NLS(B->root) + 1;
}

int
buf_lines_known(void)
{
	return UNK(B->root) == 0;
}

pos_t
buf_counted(void)
{
	return B->len - UNK(B->root);
}

/* the bytes not counted before byte k: what nl_before would count */
static pos_t
unk_before(pos_t k)
{
	struct piece *t = B->root;
	pos_t u = 0, ls;

	while(t && t->unk) {
		ls = SUM(t->l);
		if(k <= ls) {
			t = t->l;
			continue;
		}
		u += UNK(t->l);
		k -= ls;
		if(k < t->len)
			return u + (t->nl == NL_UNK ? k : 0);
		if(t->nl == NL_UNK)
			u += t->len;
		k -= t->len;
		t = t->r;
	}
	return u;
}

int
buf_lineno_try(pos_t off, pos_t budget, pos_t *n)
{
	pos_t ls = buf_linestart(off);

	if(ls != B->ln_off && unk_before(ls) > budget)
		return 0;
	*n = buf_lineno(off);
	return 1;
}

int
buf_count_try(pos_t a, pos_t b, pos_t budget, pos_t *n)
{
	if(b <= a) {
		*n = 0;
		return 1;
	}
	if(b - a <= budget) {
		*n = buf_count(a, b, POS_NONE);
		return 1;
	}
	if(unk_before(b) > budget)
		return 0;
	*n = nl_before(b) - nl_before(a);
	return 1;
}

pos_t
buf_chars(pos_t a, pos_t b)
{
	const char *p;
	pos_t n, k = 0;

	while(a < b && (n = span(a, &p)) > 0) {
		if(n > b - a)
			n = b - a;
		k += n - contcount(p, n);
		a += n;
	}
	return k;
}

pos_t
buf_plain(pos_t a, pos_t b)
{
	const char *p;
	pos_t n, k, s = a;

	while(a < b && (n = span(a, &p)) > 0) {
		if(n > b - a)
			n = b - a;
		k = plainlen(p, n);
		a += k;
		if(k < n)
			break;
	}
	return a - s;
}

static void
count_step(struct piece *t, pos_t *budget)
{
	if(t == 0 || t->unk == 0 || *budget == 0)
		return;
	count_step(t->l, budget);
	if(*budget && t->nl == NL_UNK) {
		piece_nl(t);
		*budget = t->len >= *budget ? 0 : *budget - t->len;
	}
	count_step(t->r, budget);
	update(t);
}

int
buf_count_step(pos_t budget)
{
	count_step(B->root, &budget);
	return !buf_lines_known();
}

/* --- find ---------------------------------------------------------------- */

static int
fold(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

int
buf_match(pos_t off, const char *s, int flags)
{
	pos_t n = strlen(s), i, len = buf_len();
	int c, d;

	if(n == 0 || off + n > len)
		return 0;
	for(i = 0; i < n; i++) {
		c = BCH(off + i);
		d = (unsigned char)s[i];
		if(c != d && ((flags & FIND_CASE) || fold(c) != fold(d)))
			return 0;
	}
	if(flags & FIND_WORD) {
		if(off > 0 && isword(BCH(off - 1)) && isword((unsigned char)s[0]))
			return 0;
		if(off + n < len && isword(BCH(off + n)) && isword((unsigned char)s[n - 1]))
			return 0;
	}
	return 1;
}

/* the first match that starts in [i, end): by the pieces, the first byte
 * looked for with memchr (both cases of a letter, in windows) */
static int
find_in(pos_t i, pos_t end, const char *s, int flags, pos_t *at)
{
	int c0 = (unsigned char)s[0], lc = fold(c0), uc = lc - 32, two;
	const char *p, *q, *e, *w, *q1, *q2;
	pos_t n;

	/* a letter, without Match Case: either case */
	two = !(flags & FIND_CASE) && lc >= 'a' && lc <= 'z';
	while(i < end && (n = span(i, &p)) > 0) {
		if(n > end - i)
			n = end - i;
		for(e = p + n, q = p; q < e; q++) {
			if(!two) {
				if((q = memchr(q, c0, (size_t)(e - q))) == 0)
					break;
			} else {
				for(;;) {	/* the nearer of the two, 4 KB at a time */
					w = e - q > 4096 ? q + 4096 : e;
					q1 = memchr(q, lc, (size_t)(w - q));
					q2 = memchr(q, uc, (size_t)((q1 ? q1 : w) - q));
					if(q2 || q1) {
						q = q2 ? q2 : q1;
						break;
					}
					if((q = w) >= e)
						break;
				}
				if(q >= e)
					break;
			}
			if(buf_match(i + (pos_t)(q - p), s, flags)) {
				*at = i + (pos_t)(q - p);
				return 1;
			}
		}
		i += n;
	}
	return 0;
}

int
buf_find_range(pos_t from, pos_t to, const char *s, int flags, pos_t *at)
{
	if(s[0] == 0 || to > buf_len())
		return 0;
	return find_in(from, to, s, flags, at);
}

int
buf_find(pos_t off, const char *s, int flags, pos_t *at)
{
	if(s[0] == 0)
		return 0;
	return find_in(off, buf_len(), s, flags, at) || find_in(0, off, s, flags, at);
}
