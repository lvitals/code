#include <stdlib.h>
#include <string.h>
#include "enc.h"
#include "enc_data.h"
#include "mem.h"

/* --- the list ------------------------------------------------------------
 *
 * In the order it is shown (that of edit, from ICU: the names of MIME
 * first).  own: one of the editor's (ENC_*); else -1, how it is
 * converted, and the table of enc_data.c it takes (0: the one of its
 * label).  One of these is ENC_N + its index here. */
enum {
	K_OWN, K_SB, K_MB, K_GB18030, K_UTF16, K_UTF32, K_UTF32BE, K_UTF32LE,
	K_UTF7, K_2022JP, K_2022KR, K_HZ
};

static const struct {
	const char *label;
	signed char own, kind;
	const char *table;
} defs[] = {
	{ "UTF-8", ENC_UTF8, K_OWN, 0 },
	{ "UTF-8 BOM", ENC_UTF8_BOM, K_OWN, 0 },
	{ "UTF-16", -1, K_UTF16, 0 },
	{ "UTF-16BE", ENC_UTF16BE, K_OWN, 0 },
	{ "UTF-16LE", ENC_UTF16LE, K_OWN, 0 },
	{ "UTF-32", -1, K_UTF32, 0 },
	{ "UTF-7", -1, K_UTF7, 0 },
	{ "ISO-8859-1", ENC_LATIN1, K_OWN, 0 },
	{ "US-ASCII", -1, K_SB, 0 },
	{ "GB18030", -1, K_GB18030, 0 },
	{ "ISO-8859-2", -1, K_SB, 0 },
	{ "ISO-8859-3", -1, K_SB, 0 },
	{ "ISO-8859-4", -1, K_SB, 0 },
	{ "ISO-8859-5", -1, K_SB, 0 },
	{ "ISO-8859-6", -1, K_SB, 0 },
	{ "ISO-8859-7", -1, K_SB, 0 },
	{ "ISO-8859-8", -1, K_SB, 0 },
	{ "ISO-8859-9", -1, K_SB, 0 },
	{ "ISO-8859-10", -1, K_SB, 0 },
	{ "ISO-8859-13", -1, K_SB, 0 },
	{ "ISO-8859-15", ENC_LATIN9, K_OWN, 0 },
	{ "Shift_JIS", -1, K_MB, 0 },
	{ "EUC-JP", -1, K_MB, 0 },
	{ "Big5", -1, K_MB, 0 },
	{ "GB2312", -1, K_MB, 0 },
	{ "EUC-KR", -1, K_MB, 0 },
	{ "IBM850", -1, K_SB, 0 },
	{ "cp851", -1, K_SB, 0 },
	{ "IBM857", -1, K_SB, 0 },
	{ "IBM00858", -1, K_SB, 0 },
	{ "IBM860", -1, K_SB, 0 },
	{ "IBM861", -1, K_SB, 0 },
	{ "IBM862", -1, K_SB, 0 },
	{ "IBM863", -1, K_SB, 0 },
	{ "IBM864", -1, K_SB, 0 },
	{ "IBM865", -1, K_SB, 0 },
	{ "IBM866", -1, K_SB, 0 },
	{ "IBM868", -1, K_SB, 0 },
	{ "IBM869", -1, K_SB, 0 },
	{ "KOI8-R", -1, K_SB, 0 },
	{ "KOI8-U", -1, K_SB, 0 },
	{ "windows-1250", -1, K_SB, 0 },
	{ "windows-1251", -1, K_SB, 0 },
	{ "windows-1252", ENC_CP1252, K_OWN, 0 },
	{ "windows-1253", -1, K_SB, 0 },
	{ "windows-1254", -1, K_SB, 0 },
	{ "windows-1255", -1, K_SB, 0 },
	{ "windows-1256", -1, K_SB, 0 },
	{ "windows-1257", -1, K_SB, 0 },
	{ "windows-1258", -1, K_SB, 0 },
	{ "macintosh", -1, K_SB, 0 },
	{ "TIS-620", -1, K_SB, 0 },
	{ "windows-874", -1, K_SB, 0 },
	{ "Big5-HKSCS", -1, K_MB, 0 },
	{ "ISO-2022-JP", -1, K_2022JP, "EUC-JP" },
	{ "ISO-2022-KR", -1, K_2022KR, "EUC-KR" },
	{ "GBK", -1, K_MB, 0 },
	{ "windows-949", -1, K_MB, 0 },
	{ "windows-950", -1, K_MB, 0 },
	{ "windows-31j", -1, K_MB, 0 },
	{ "HZ-GB-2312", -1, K_HZ, "GB2312" },
	{ "IBM037", -1, K_SB, 0 },
	{ "IBM273", -1, K_SB, 0 },
	{ "IBM277", -1, K_SB, 0 },
	{ "IBM278", -1, K_SB, 0 },
	{ "IBM280", -1, K_SB, 0 },
	{ "IBM284", -1, K_SB, 0 },
	{ "IBM285", -1, K_SB, 0 },
	{ "IBM297", -1, K_SB, 0 },
	{ "IBM420", -1, K_SB, 0 },
	{ "IBM424", -1, K_SB, 0 },
	{ "IBM437", -1, K_SB, 0 },
	{ "IBM500", -1, K_SB, 0 },
	{ "IBM775", -1, K_SB, 0 },
	{ "IBM870", -1, K_SB, 0 },
	{ "IBM871", -1, K_SB, 0 },
	{ "IBM918", -1, K_SB, 0 },
	{ "IBM1026", -1, K_SB, 0 },
	{ "IBM1047", -1, K_SB, 0 },
	{ "IBM01140", -1, K_SB, 0 },
	{ "IBM01141", -1, K_SB, 0 },
	{ "IBM01142", -1, K_SB, 0 },
	{ "IBM01143", -1, K_SB, 0 },
	{ "IBM01144", -1, K_SB, 0 },
	{ "IBM01145", -1, K_SB, 0 },
	{ "IBM01146", -1, K_SB, 0 },
	{ "IBM01147", -1, K_SB, 0 },
	{ "IBM01148", -1, K_SB, 0 },
	{ "IBM01149", -1, K_SB, 0 },
	{ "UTF-32BE", -1, K_UTF32BE, 0 },
	{ "UTF-32LE", -1, K_UTF32LE, 0 },
	{ "ISO-8859-11", -1, K_SB, 0 },
	{ "ISO-8859-14", -1, K_SB, 0 },
	{ "ISO-8859-16", -1, K_SB, 0 },
	{ "JOHAB", -1, K_MB, 0 },
	{ "x-mac-cyrillic", -1, K_SB, 0 },
	{ "x-mac-ce", -1, K_SB, 0 },
	{ "x-mac-ukrainian", -1, K_SB, 0 },
	{ "ARMSCII-8", -1, K_SB, 0 },
	{ "Georgian-PS", -1, K_SB, 0 },
	{ "PT154", -1, K_SB, 0 },
	{ "KOI8-RU", -1, K_SB, 0 },
	{ "VISCII", -1, K_SB, 0 },
	{ "TCVN", -1, K_SB, 0 },
	{ "cp737", -1, K_SB, 0 },
	{ "cp856", -1, K_SB, 0 },
	{ "cp1125", -1, K_SB, 0 }
};
#define NDEFS	(int)(sizeof defs / sizeof defs[0])

/* an encoding is a char of the buf: all of them fit */
typedef char enc_fits[ENC_N + sizeof defs / sizeof defs[0] <= 127 ? 1 : -1];

const char *
enc_name(int e)
{
	int i;

	if(ENC_EXT(e))
		return e - ENC_N < NDEFS ? defs[e - ENC_N].label : "?";
	for(i = 0; i < NDEFS; i++)
		if(defs[i].own == e)
			return defs[i].label;
	return "?";
}

int
enc_list(int *ids, int max)
{
	int i, n = 0;

	for(i = 0; i < NDEFS && n < max; i++)
		ids[n++] = defs[i].own >= 0 ? defs[i].own : ENC_N + i;
	return n;
}

/* the characters of 80h-9Fh in Windows-1252 (the C1 controls where it
 * has none) */
static const unsigned short cp1252[32] = {
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
	0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
};

/* where ISO-8859-15 differs from ISO-8859-1: the byte, its character */
static const unsigned short latin9[8][2] = {
	{ 0xA4, 0x20AC }, { 0xA6, 0x0160 }, { 0xA8, 0x0161 }, { 0xB4, 0x017D },
	{ 0xB8, 0x017E }, { 0xBC, 0x0152 }, { 0xBD, 0x0153 }, { 0xBE, 0x0178 }
};

/* --- what a file is in --------------------------------------------------- */

int
enc_bom(int e, const char **bom)
{
	switch(e) {
	case ENC_UTF8_BOM:
		*bom = "\xEF\xBB\xBF";
		return 3;
	case ENC_UTF16LE:
		*bom = "\xFF\xFE";
		return 2;
	case ENC_UTF16BE:
		*bom = "\xFE\xFF";
		return 2;
	}
	*bom = "";
	return 0;
}

int
enc_sniff_bom(const char *p, pos_t n, int *skip)
{
	const unsigned char *u = (const unsigned char *)p;

	*skip = 0;
	if(n >= 3 && u[0] == 0xEF && u[1] == 0xBB && u[2] == 0xBF) {
		*skip = 3;
		return ENC_UTF8_BOM;
	}
	if(n >= 2 && u[0] == 0xFF && u[1] == 0xFE) {
		*skip = 2;
		return ENC_UTF16LE;
	}
	if(n >= 2 && u[0] == 0xFE && u[1] == 0xFF) {
		*skip = 2;
		return ENC_UTF16BE;
	}
	return ENC_AUTO;
}

int
enc_sniff_utf16(const char *p, pos_t n)
{
	pos_t i, pairs = n / 2, ze = 0, zo = 0;

	if(pairs < 2)
		return ENC_AUTO;
	for(i = 0; i < pairs * 2; i += 2) {
		ze += p[i] == 0;
		zo += p[i + 1] == 0;
	}
	/* text with most characters ASCII: one byte of each pair zero, the
	   other seldom (a low byte of 00h: Ā, Ȁ, ...) */
	if(zo * 10 >= pairs * 4 && ze * 10 <= zo)
		return ENC_UTF16LE;
	if(ze * 10 >= pairs * 4 && zo * 10 <= ze)
		return ENC_UTF16BE;
	return ENC_AUTO;
}

int
u8_check(struct u8check *c, const char *p, pos_t n, int last)
{
	const unsigned char *u = (const unsigned char *)p, *e = u + n;
	unsigned b;

	while(u < e) {
		b = *u;
		if(c->need) {
			if((int)b < c->lo || (int)b > c->hi)
				return 0;
			c->lo = 0x80;
			c->hi = 0xBF;
			c->need--;
			u++;
			continue;
		}
		if(b < 0x80) {	/* ASCII: eight at a time when they all are */
			while(e - u >= 8) {
				unsigned long long w;

				memcpy(&w, u, 8);
				if(w & 0x8080808080808080ULL)
					break;
				u += 8;
			}
			while(u < e && *u < 0x80)
				u++;
			continue;
		}
		c->lo = 0x80;
		c->hi = 0xBF;
		if(b >= 0xC2 && b <= 0xDF)
			c->need = 1;
		else if(b >= 0xE0 && b <= 0xEF) {
			c->need = 2;
			if(b == 0xE0)
				c->lo = 0xA0;	/* not overlong */
			else if(b == 0xED)
				c->hi = 0x9F;	/* not a surrogate */
		} else if(b >= 0xF0 && b <= 0xF4) {
			c->need = 3;
			if(b == 0xF0)
				c->lo = 0x90;
			else if(b == 0xF4)
				c->hi = 0x8F;	/* up to 10FFFFh */
		} else
			return 0;
		u++;
	}
	return !(last && c->need);
}

/* --- decoding ------------------------------------------------------------ */

static int
put8(char *s, unsigned long c)
{
	if(c < 0x80) {
		s[0] = (char)c;
		return 1;
	}
	if(c < 0x800) {
		s[0] = (char)(0xC0 | c >> 6);
		s[1] = (char)(0x80 | (c & 0x3F));
		return 2;
	}
	if(c < 0x10000) {
		s[0] = (char)(0xE0 | c >> 12);
		s[1] = (char)(0x80 | (c >> 6 & 0x3F));
		s[2] = (char)(0x80 | (c & 0x3F));
		return 3;
	}
	s[0] = (char)(0xF0 | c >> 18);
	s[1] = (char)(0x80 | (c >> 12 & 0x3F));
	s[2] = (char)(0x80 | (c >> 6 & 0x3F));
	s[3] = (char)(0x80 | (c & 0x3F));
	return 4;
}

/* U+FFFD for bytes that are none: counted */
static int
put_bad(struct dec *d, char *s)
{
	d->bad++;
	return put8(s, 0xFFFD);
}

static unsigned long
byte_char(int e, unsigned b)
{
	int i;

	if(b < 0x80)
		return b;
	if(e == ENC_CP1252 && b < 0xA0)
		return cp1252[b - 0x80];
	if(e == ENC_LATIN9)
		for(i = 0; i < 8; i++)
			if(latin9[i][0] == b)
				return latin9[i][1];
	return b;
}

/* a UTF-16 unit: a character out, or a surrogate kept */
static pos_t
unit16(struct dec *d, unsigned long u, char *out)
{
	pos_t o = 0;

	if(d->hi) {
		if(u >= 0xDC00 && u <= 0xDFFF) {
			o = put8(out, 0x10000 + ((d->hi - 0xD800) << 10) + (u - 0xDC00));
			d->hi = 0;
			return o;
		}
		o = put_bad(d, out);	/* a high surrogate alone */
		d->hi = 0;
	}
	if(u >= 0xD800 && u <= 0xDBFF)
		d->hi = u;
	else if(u >= 0xDC00 && u <= 0xDFFF)
		o += put_bad(d, out + o);
	else
		o += put8(out + o, u);
	return o;
}

/* --- the others: from the tables (enc_data.c), or computed --------------- */

#define KIND(e)		(defs[(e) - ENC_N].kind)

static signed char tix[NDEFS];	/* the table of each: 1 + its index; 0
				   not looked for yet; -1 none */

/* the index of the table of e in encd_sb (K_SB) or encd_mb: -1 none */
static int
table_ix(int e)
{
	int i = e - ENC_N, j;
	const char *name = defs[i].table ? defs[i].table : defs[i].label;

	if(tix[i] == 0) {
		tix[i] = -1;
		if(defs[i].kind == K_SB) {
			for(j = 0; j < encd_nsb; j++)
				if(strcmp(encd_sb[j].label, name) == 0)
					tix[i] = (signed char)(j + 1);
		} else
			for(j = 0; j < encd_nmb; j++)
				if(strcmp(encd_mb[j].label, name) == 0)
					tix[i] = (signed char)(j + 1);
	}
	return tix[i] > 0 ? tix[i] - 1 : -1;
}

static const struct encd_sb *
sb_of(int e)
{
	int j = KIND(e) == K_SB ? table_ix(e) : -1;

	return j < 0 ? 0 : &encd_sb[j];
}

static const struct encd_mb *
mb_of(int e)
{
	int j = KIND(e) != K_SB ? table_ix(e) : -1;

	return j < 0 ? 0 : &encd_mb[j];
}

/* the row of t after the prefix p0 (p1, two bytes) */
static const struct encd_row *
find_row(const struct encd_mb *t, int npre, unsigned p0, unsigned p1)
{
	unsigned long want = (unsigned long)npre << 16 | p0 << 8 | p1, k;
	unsigned lo = t->r0, hi = t->r0 + t->nr, m;

	while(lo < hi) {
		m = (lo + hi) / 2;
		k = (unsigned long)encd_rows[m].npre << 16 | (unsigned)encd_rows[m].pre0 << 8 |
		    encd_rows[m].pre1;
		if(k == want)
			return &encd_rows[m];
		if(k < want)
			lo = m + 1;
		else
			hi = m;
	}
	return 0;
}

static const struct encd_x *
find_x(const struct encd_mb *t, unsigned long key)
{
	unsigned lo = t->x0, hi = t->x0 + t->nx, m;

	while(lo < hi) {
		m = (lo + hi) / 2;
		if(encd_x[m].key == key)
			return &encd_x[m];
		if(encd_x[m].key < key)
			lo = m + 1;
		else
			hi = m;
	}
	return 0;
}

/* key is one read, not written (another is written for it) */
static int
decode_only(unsigned d0, unsigned nd, unsigned long key)
{
	unsigned lo = d0, hi = d0 + nd, m;

	while(lo < hi) {
		m = (lo + hi) / 2;
		if(encd_donly[m] == key)
			return 1;
		if(encd_donly[m] < key)
			lo = m + 1;
		else
			hi = m;
	}
	return 0;
}

/* GB18030's four bytes p (n of them so far): as mb_try */
static int
gb4(const unsigned char *p, int n, unsigned long *c)
{
	unsigned long i;
	int lo = 0, hi = encd_ngb18030, m;

	if(n == 2)
		return 0;
	if(p[2] < 0x81 || p[2] > 0xFE)
		return -1;
	if(n == 3)
		return 0;
	if(p[3] < 0x30 || p[3] > 0x39)
		return -1;
	i = (((unsigned long)(p[0] - 0x81) * 10 + (p[1] - 0x30)) * 126 + (p[2] - 0x81)) * 10 +
	    (p[3] - 0x30);
	if(p[0] >= 0x90) {	/* beyond the BMP: in order */
		if(i - 189000 > 0xFFFFF)
			return -1;
		*c = 0x10000 + i - 189000;
		return 1;
	}
	if(i >= 39420)
		return -1;
	while(hi - lo > 1) {	/* the run it is in */
		m = (lo + hi) / 2;
		if(encd_gb18030[m].idx <= i)
			lo = m;
		else
			hi = m;
	}
	if(encd_gb18030[lo].c == ENCD_GBNONE)
		return -1;
	*c = encd_gb18030[lo].c + (i - encd_gb18030[lo].idx);
	return 1;
}

/* the n bytes p in t: 1, a character (to *c; a second to *c2, or 0); 0
 * cut short (more to come); -1 none */
static int
mb_try(const struct encd_mb *t, int gb, const unsigned char *p, int n, unsigned long *c,
    unsigned long *c2)
{
	const struct encd_row *r;
	const struct encd_x *x;
	unsigned v = encd_pool[t->sb + p[0]], last = p[n - 1];
	unsigned long key;

	*c2 = 0;
	if(n == 1) {
		if(v == ENCD_NONE)
			return -1;
		if(v == ENCD_LEAD)
			return 0;
		*c = v;
		return 1;
	}
	if(v != ENCD_LEAD)
		return -1;
	if(gb && p[1] >= 0x30 && p[1] <= 0x39)
		return gb4(p, n, c);
	if(n > 3)
		return -1;
	r = find_row(t, n - 1, p[0], n > 2 ? p[1] : 0);
	if(r && last >= r->lo && last <= r->hi &&
	   (v = encd_pool[r->off + last - r->lo]) != ENCD_NONE) {
		if(v != ENCD_X) {
			*c = v;
			return 1;
		}
		key = n == 2 ? ENCD_KEY(2, (unsigned long)p[0] << 8 | p[1]) :
		    ENCD_KEY(3, (unsigned long)p[0] << 16 | (unsigned long)p[1] << 8 | p[2]);
		if((x = find_x(t, key)) == 0)
			return -1;
		*c = x->c;
		*c2 = x->c2;
		return 1;
	}
	if(n == 2 && find_row(t, 2, p[0], p[1]))
		return 0;	/* EUC-JP's 8F: three bytes */
	return -1;
}

/* the first k bytes kept, gone */
static void
drop(struct dec *d, int k)
{
	d->npend -= k;
	memmove(d->pend, d->pend + k, (size_t)d->npend);
}

/* the bytes kept, read as far as they go (last: to the end; one cut
 * short is U+FFFD): what they got */
static pos_t
mb_feed(const struct encd_mb *t, int gb, struct dec *d, int last, char *out)
{
	unsigned long c, c2;
	pos_t o = 0;
	int r;

	while(d->npend) {
		r = mb_try(t, gb, d->pend, d->npend, &c, &c2);
		if(r == 0 && !last)
			break;
		if(r > 0) {
			o += put8(out + o, c);
			if(c2)
				o += put8(out + o, c2);
			d->npend = 0;
			break;
		}
		o += put_bad(d, out + o);	/* the first byte: wrong; the
						   rest read again */
		drop(d, 1);
	}
	return o;
}

/* UTF-16 and UTF-32: a byte; a unit when it is whole (st: 0 the first,
 * where a BOM may be; 1 big endian; 2 little endian) */
static pos_t
wide_byte(struct dec *d, int k, unsigned b, char *out)
{
	const unsigned char *p = d->pend;
	int w = k == K_UTF16 ? 2 : 4, be;
	unsigned long u;

	d->pend[d->npend++] = (unsigned char)b;
	if(d->npend < w)
		return 0;
	d->npend = 0;
	be = k == K_UTF32BE || (k != K_UTF32LE && d->st != 2);
	if(w == 2)
		u = be ? (unsigned long)p[0] << 8 | p[1] : (unsigned long)p[1] << 8 | p[0];
	else if(be)
		u = (unsigned long)p[0] << 24 | (unsigned long)p[1] << 16 |
		    (unsigned long)p[2] << 8 | p[3];
	else
		u = (unsigned long)p[3] << 24 | (unsigned long)p[2] << 16 |
		    (unsigned long)p[1] << 8 | p[0];
	if(d->st == 0 && (k == K_UTF16 || k == K_UTF32)) {
		d->st = 1;	/* big endian, but for its BOM */
		if(u == 0xFEFF)
			return 0;
		if(u == (w == 2 ? 0xFFFEUL : 0xFFFE0000UL)) {
			d->st = 2;
			return 0;
		}
	}
	if(w == 2)
		return unit16(d, u, out);
	return u > 0x10FFFF || (u >= 0xD800 && u <= 0xDFFF) ? put_bad(d, out) : put8(out, u);
}

static const char b64c[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int
b64v(unsigned b)
{
	const char *q;

	return b && (q = strchr(b64c, (int)b)) != 0 ? (int)(q - b64c) : -1;
}

/* UTF-7: a byte (st: 0 direct; 1 base64; 2 just after '+') */
static pos_t
utf7_byte(struct dec *d, unsigned b, char *out)
{
	pos_t o = 0;
	int v;

	if(d->st == 2) {
		if(b == '-') {	/* +- is + */
			d->st = 0;
			return put8(out, '+');
		}
		d->st = 1;
		d->acc = 0;
		d->nbits = 0;
	}
	if(d->st == 1) {
		if((v = b64v(b)) >= 0) {
			d->acc = (d->acc << 6 | (unsigned long)v) & 0xFFFFFF;
			if((d->nbits += 6) >= 16) {
				d->nbits -= 16;
				o += unit16(d, d->acc >> d->nbits & 0xFFFF, out);
			}
			return o;
		}
		d->st = 0;	/* its end; a '-' that ends it is not shown */
		if(b == '-')
			return o;
	}
	if(b == '+') {
		d->st = 2;
		return o;
	}
	return o + (b < 0x80 ? put8(out + o, b) : put_bad(d, out + o));
}

/* the escapes of ISO-2022-JP (the mode each sets: 0 ASCII, 1 JIS-Roman,
 * 2 JIS X 0208) and of ISO-2022-KR (its header) */
static const char *const jp_esc[] = { "\x1b(B", "\x1b(J", "\x1b$@", "\x1b$B" };
static const char jp_mode[] = { 0, 1, 2, 2 };
static const char *const kr_esc[] = { "\x1b$)C" };

/* the n bytes p, as one of the escapes s: its index; -1 the start of
 * one; -2 none */
static int
esc_match(const unsigned char *p, int n, const char *const *s, int ns)
{
	int i, len, part = 0;

	for(i = 0; i < ns; i++) {
		len = (int)strlen(s[i]);
		if(memcmp(p, s[i], (size_t)(n < len ? n : len)) == 0) {
			if(n >= len)
				return i;
			part = 1;
		}
	}
	return part ? -1 : -2;
}

/* ISO-2022-JP, ISO-2022-KR and HZ: a byte (st: the mode; two bytes a
 * character in 2 (JP) or 1 (KR: after SO; HZ: after ~{), from the table
 * of t with the high bits set) */
static pos_t
iso_byte(struct dec *d, int k, const struct encd_mb *t, unsigned b, char *out)
{
	unsigned char *p = d->pend, q[2];
	unsigned long c, c2;
	pos_t o = 0;
	int m;

	p[d->npend++] = (unsigned char)b;
	while(d->npend) {
		if(k == K_HZ && p[0] == '~') {
			if(d->npend == 1)
				return o;
			if(p[1] == '{' || p[1] == '}')
				d->st = p[1] == '{';
			else if(p[1] == '~')
				o += put8(out + o, '~');
			else if(p[1] != '\n') {	/* ~ newline: a line goes on */
				o += put_bad(d, out + o);
				drop(d, 1);
				continue;
			}
			drop(d, 2);
			continue;
		}
		if(k != K_HZ && p[0] == 0x1B) {
			m = k == K_2022JP ? esc_match(p, d->npend, jp_esc, 4) :
			    esc_match(p, d->npend, kr_esc, 1);
			if(m == -1)
				return o;
			if(m >= 0) {
				if(k == K_2022JP)
					d->st = jp_mode[m];
				drop(d, (int)strlen(k == K_2022JP ? jp_esc[m] : kr_esc[m]));
				continue;
			}
			o += put_bad(d, out + o);
			drop(d, 1);
			continue;
		}
		if(k == K_2022KR && (p[0] == 0x0E || p[0] == 0x0F)) {
			d->st = p[0] == 0x0E;	/* SO, SI */
			drop(d, 1);
			continue;
		}
		if(d->st == (k == K_2022JP ? 2 : 1) && p[0] >= 0x21 && p[0] <= 0x7E) {
			if(d->npend == 1)
				return o;
			q[0] = (unsigned char)(p[0] | 0x80);
			q[1] = (unsigned char)(p[1] | 0x80);
			if(t && p[1] >= 0x21 && p[1] <= 0x7E && mb_try(t, 0, q, 2, &c, &c2) > 0) {
				o += put8(out + o, c);
				drop(d, 2);
				continue;
			}
			o += put_bad(d, out + o);
			drop(d, 1);
			continue;
		}
		c = p[0];
		if(c >= 0x80)
			o += put_bad(d, out + o);
		else {
			if(k == K_2022JP && d->st == 1 && (c == 0x5C || c == 0x7E))
				c = c == 0x5C ? 0xA5 : 0x203E;	/* JIS-Roman: ¥, ‾ */
			o += put8(out + o, c);
		}
		drop(d, 1);
	}
	return o;
}

static pos_t
dec_ext(struct dec *d, const char *in, pos_t n, char *out, int last)
{
	const unsigned char *u = (const unsigned char *)in, *e = u + n;
	int k = KIND(d->enc);
	const struct encd_sb *s = sb_of(d->enc);
	const struct encd_mb *t = mb_of(d->enc);
	pos_t o = 0;
	unsigned v;

	if((k == K_MB || k == K_GB18030) && t == 0)
		k = K_SB;	/* no table: ASCII */
	for(; u < e; u++)
		switch(k) {
		case K_SB:
			v = s ? encd_pool[s->sb + *u] : *u < 0x80 ? *u : ENCD_NONE;
			o += v == ENCD_NONE ? put_bad(d, out + o) : put8(out + o, v);
			break;
		case K_MB:
		case K_GB18030:
			d->pend[d->npend++] = *u;
			o += mb_feed(t, k == K_GB18030, d, 0, out + o);
			break;
		case K_UTF7:
			o += utf7_byte(d, *u, out + o);
			break;
		case K_2022JP:
		case K_2022KR:
		case K_HZ:
			o += iso_byte(d, k, t, *u, out + o);
			break;
		default:
			o += wide_byte(d, k, *u, out + o);
		}
	if(last) {
		if(k == K_MB || k == K_GB18030)
			o += mb_feed(t, k == K_GB18030, d, 1, out + o);
		else if(d->npend)	/* cut short */
			o += put_bad(d, out + o);
		if(d->hi)
			o += put_bad(d, out + o);
		d->hi = 0;
		d->npend = 0;
		d->st = 0;
	}
	return o;
}
pos_t
dec_run(struct dec *d, const char *in, pos_t n, char *out, int last)
{
	const unsigned char *u = (const unsigned char *)in, *e = u + n;
	pos_t o = 0;
	int be = d->enc == ENC_UTF16BE;

	if(ENC_EXT(d->enc))
		return dec_ext(d, in, n, out, last);
	if(!ENC_UTF16(d->enc)) {
		for(; u < e; u++) {
			if(*u < 0x80)
				out[o++] = (char)*u;
			else {
				if(*u < 0xA0)
					d->c1 = 1;
				o += put8(out + o, byte_char(d->enc, *u));
			}
		}
		return o;
	}
	if(d->npend && u < e) {
		d->pend[1] = *u++;
		o += unit16(d, be ? (unsigned long)d->pend[0] << 8 | d->pend[1] :
		    (unsigned long)d->pend[1] << 8 | d->pend[0], out + o);
		d->npend = 0;
	}
	for(; e - u >= 2; u += 2)
		o += unit16(d, be ? (unsigned long)u[0] << 8 | u[1] :
		    (unsigned long)u[1] << 8 | u[0], out + o);
	if(u < e) {
		d->pend[0] = *u;
		d->npend = 1;
	}
	if(last) {
		if(d->hi)
			o += put_bad(d, out + o);
		if(d->npend)
			o += put_bad(d, out + o);
		d->hi = 0;
		d->npend = 0;
	}
	return o;
}

/* --- encoding ------------------------------------------------------------ */

static int ext_has(int e, unsigned long c);

int
enc_has(int e, unsigned long c)
{
	int i;

	if(ENC_EXT(e))
		return ext_has(e, c);
	if(!ENC_8BIT(e) || c < 0x80)
		return 1;
	if(e == ENC_CP1252) {
		if(c >= 0xA0 && c <= 0xFF)
			return 1;
		for(i = 0; i < 32; i++)
			if(cp1252[i] == c)
				return 1;
		return 0;
	}
	if(e == ENC_LATIN9) {
		for(i = 0; i < 8; i++) {
			if(latin9[i][1] == c)
				return 1;
			if(latin9[i][0] == c)
				return 0;	/* its byte is another's */
		}
	}
	return c <= 0xFF;
}

static unsigned
char_byte(int e, unsigned long c)
{
	int i;

	if(e == ENC_CP1252 && c > 0xFF)
		for(i = 0; i < 32; i++)
			if(cp1252[i] == c)
				return 0x80 + i;
	if(e == ENC_LATIN9 && c > 0xFF)
		for(i = 0; i < 8; i++)
			if(latin9[i][1] == c)
				return latin9[i][0];
	return (unsigned)c & 0xFF;
}

/* the length of the character a lead byte starts */
static int
seqlen(unsigned b)
{
	return b < 0xC0 ? 1 : b < 0xE0 ? 2 : b < 0xF0 ? 3 : 4;
}

static pos_t
put_char(struct encd *e, unsigned long c, char *out)
{
	unsigned long h, l;

	if(ENC_8BIT(e->enc)) {
		out[0] = (char)char_byte(e->enc, c);
		return 1;
	}
	if(c >= 0x10000) {
		h = 0xD800 + ((c - 0x10000) >> 10);
		l = 0xDC00 + ((c - 0x10000) & 0x3FF);
		put_char(e, h, out);
		put_char(e, l, out + 2);
		return 4;
	}
	if(e->enc == ENC_UTF16BE) {
		out[0] = (char)(c >> 8);
		out[1] = (char)(c & 0xFF);
	} else {
		out[0] = (char)(c & 0xFF);
		out[1] = (char)(c >> 8);
	}
	return 2;
}

static unsigned long
get_char(const unsigned char *s, int n)
{
	unsigned long c;
	int i;

	if(n == 1)
		return s[0];
	c = s[0] & (0x7F >> n);
	for(i = 1; i < n; i++)
		c = c << 6 | (s[i] & 0x3F);
	return c;
}

/* --- the others, written -------------------------------------------------
 *
 * A table read the other way: its characters and their bytes, by
 * character, made when one is first written (or checked), the last four
 * kept.  The decode only ones are left out. */

struct rv {
	unsigned long c, key;
};

static struct {
	const void *tab;
	struct rv *v;
	int n;
} rvc[4];
static int rvnext;

static int
by_c(const void *a, const void *b)
{
	const struct rv *x = a, *y = b;

	if(x->c != y->c)
		return x->c < y->c ? -1 : 1;
	return x->key < y->key ? -1 : x->key > y->key;
}

/* the table of sb (one byte) or mb, the other way: 0 if out of memory */
static struct rv *
rev_of(const struct encd_sb *sb, const struct encd_mb *mb, int *n)
{
	const void *tab = sb ? (const void *)sb : (const void *)mb;
	unsigned base = sb ? sb->sb : mb->sb, d0 = sb ? sb->d0 : mb->d0, nd = sb ? sb->nd : mb->nd;
	const struct encd_row *r;
	const struct encd_x *x;
	unsigned long key;
	struct rv *v;
	unsigned i, j, max = 256, v16;
	int k = 0, m;

	for(m = 0; m < 4; m++)
		if(rvc[m].tab == tab) {
			*n = rvc[m].n;
			return rvc[m].v;
		}
	if(mb)
		for(i = mb->r0; i < mb->r0 + mb->nr; i++)
			max += encd_rows[i].hi - encd_rows[i].lo + 1u;
	if((v = mem_alloc(max * sizeof *v)) == 0)
		return 0;
	for(i = 0; i < 256; i++) {
		v16 = encd_pool[base + i];
		key = ENCD_KEY(1, i);
		if(v16 < 0xFFFE && !decode_only(d0, nd, key)) {
			v[k].c = v16;
			v[k++].key = key;
		}
	}
	if(mb)
		for(i = mb->r0; i < mb->r0 + mb->nr; i++) {
			r = &encd_rows[i];
			for(j = r->lo; j <= r->hi; j++) {
				v16 = encd_pool[r->off + j - r->lo];
				key = r->npre == 1 ? ENCD_KEY(2, (unsigned long)r->pre0 << 8 | j) :
				    ENCD_KEY(3, (unsigned long)r->pre0 << 16 | (unsigned long)r->pre1 << 8 | j);
				if(v16 == ENCD_NONE || decode_only(d0, nd, key))
					continue;
				if(v16 == ENCD_X) {	/* beyond 16 bits; two: not so */
					if((x = find_x(mb, key)) == 0 || x->c2)
						continue;
					v[k].c = x->c;
				} else
					v[k].c = v16;
				v[k++].key = key;
			}
		}
	qsort(v, (size_t)k, sizeof *v, by_c);
	for(i = m = 0; (int)i < k; i++)	/* one for each character: the first */
		if(m == 0 || v[m - 1].c != v[i].c)
			v[m++] = v[i];
	mem_free(rvc[rvnext].v);
	rvc[rvnext].tab = tab;
	rvc[rvnext].v = v;
	rvc[rvnext].n = m;
	rvnext = (rvnext + 1) % 4;
	*n = m;
	return v;
}

/* the bytes of c in sb or mb, as a key: 0 if it has none */
static unsigned long
rev_key(const struct encd_sb *sb, const struct encd_mb *mb, unsigned long c)
{
	struct rv *v;
	int n, lo = 0, hi, m;

	if(!sb && !mb)
		return c < 0x80 ? ENCD_KEY(1, c) : 0;	/* US-ASCII with no table */
	if((v = rev_of(sb, mb, &n)) == 0)
		return 0;
	for(hi = n; lo < hi;) {
		m = (lo + hi) / 2;
		if(v[m].c == c)
			return v[m].key;
		if(v[m].c < c)
			lo = m + 1;
		else
			hi = m;
	}
	return 0;
}

/* the two bytes of c in a 94 x 94 set (JIS X 0208, KS X 1001, GB 2312:
 * both from A1h, in the EUC table t): its key, or 0 */
static unsigned long
set94(const struct encd_mb *t, unsigned long c)
{
	unsigned long key = t ? rev_key(0, t, c) : 0;

	return key >> 24 == 2 && (key >> 8 & 0xFF) >= 0xA1 && (key & 0xFF) >= 0xA1 ? key : 0;
}

/* GB18030's four bytes of c (not in its table): their number, 0 none */
static int
gb4_put(unsigned long c, char *out)
{
	unsigned long i = 0, end;
	int m, found = 0;

	if(c >= 0x10000 && c <= 0x10FFFF) {
		i = c - 0x10000 + 189000;
		found = 1;
	}
	for(m = 0; !found && m < encd_ngb18030; m++) {
		end = m + 1 < encd_ngb18030 ? encd_gb18030[m + 1].idx : 39420;
		if(encd_gb18030[m].c != ENCD_GBNONE && c >= encd_gb18030[m].c &&
		   c - encd_gb18030[m].c < end - encd_gb18030[m].idx) {
			i = encd_gb18030[m].idx + (c - encd_gb18030[m].c);
			found = 1;
		}
	}
	if(!found)
		return 0;
	out[3] = (char)(0x30 + i % 10);
	i /= 10;
	out[2] = (char)(0x81 + i % 126);
	i /= 126;
	out[1] = (char)(0x30 + i % 10);
	out[0] = (char)(0x81 + i / 10);
	return 4;
}

static pos_t
put_key(unsigned long key, char *out)
{
	int n = (int)(key >> 24), i;

	for(i = 0; i < n; i++)
		out[i] = (char)(key >> 8 * (n - 1 - i) & 0xFF);
	return n;
}

/* UTF-7: direct, the characters of RFC 2152's set D and the blanks */
static int
u7direct(unsigned long c)
{
	return c && c < 0x80 && ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
	    (c >= '0' && c <= '9') || strchr("'(),-./:? \t\r\n", (int)c));
}

/* UTF-7: the end of base64 (the bits left, then '-') */
static pos_t
u7end(struct encd *e, char *out)
{
	pos_t o = 0;

	if(e->st) {
		if(e->nbits)
			out[o++] = b64c[(e->acc << (6 - e->nbits)) & 63];
		out[o++] = '-';
	}
	e->st = 0;
	e->nbits = 0;
	e->acc = 0;
	return o;
}

static pos_t
u7unit(struct encd *e, unsigned long u, char *out)
{
	pos_t o = 0;

	e->acc = (e->acc << 16 | u) & 0xFFFFFF;
	e->nbits += 16;
	while(e->nbits >= 6) {
		e->nbits -= 6;
		out[o++] = b64c[e->acc >> e->nbits & 63];
	}
	return o;
}

/* 4 bytes of c, as UTF-32: big endian or not */
static void
put32(char *out, unsigned long c, int be)
{
	int i;

	for(i = 0; i < 4; i++)
		out[i] = (char)(c >> 8 * (be ? 3 - i : i) & 0xFF);
}

static pos_t
put_ext(struct encd *e, unsigned long c, char *out)
{
	int k = KIND(e->enc);
	const struct encd_sb *s = sb_of(e->enc);
	const struct encd_mb *t = mb_of(e->enc);
	unsigned long key;
	pos_t o = 0;

	switch(k) {
	case K_SB:
	case K_MB:
	case K_GB18030:
		if((key = rev_key(s, t, c)) != 0)
			return put_key(key, out);
		if(k == K_GB18030 && (o = gb4_put(c, out)) != 0)
			return o;
		/* not in it (buf_save checked: none) */
		key = rev_key(s, t, '?');
		return key ? put_key(key, out) : (out[0] = '?', 1);
	case K_UTF16:
		if(e->st == 0) {	/* its BOM, big endian */
			out[o++] = (char)0xFE;
			out[o++] = (char)0xFF;
			e->st = 1;
		}
		if(c >= 0x10000) {
			key = 0xD800 + ((c - 0x10000) >> 10);
			out[o++] = (char)(key >> 8);
			out[o++] = (char)(key & 0xFF);
			c = 0xDC00 + ((c - 0x10000) & 0x3FF);
		}
		out[o++] = (char)(c >> 8);
		out[o++] = (char)(c & 0xFF);
		return o;
	case K_UTF32:
		if(e->st == 0) {
			put32(out, 0xFEFF, 1);
			o = 4;
			e->st = 1;
		}
		put32(out + o, c, 1);
		return o + 4;
	case K_UTF32BE:
	case K_UTF32LE:
		put32(out, c, k == K_UTF32BE);
		return 4;
	case K_UTF7:
		if(u7direct(c)) {
			o = u7end(e, out);
			out[o++] = (char)c;
			return o;
		}
		if(c == '+' && !e->st) {
			out[0] = '+';
			out[1] = '-';
			return 2;
		}
		if(!e->st) {
			out[o++] = '+';
			e->st = 1;
		}
		if(c >= 0x10000) {
			o += u7unit(e, 0xD800 + ((c - 0x10000) >> 10), out + o);
			c = 0xDC00 + ((c - 0x10000) & 0x3FF);
		}
		return o + u7unit(e, c, out + o);
	case K_2022JP:
		key = c < 0x80 ? 0 : set94(t, c);
		if(c == 0xA5 || c == 0x203E) {	/* JIS-Roman */
			if(e->st != 1) {
				memcpy(out, jp_esc[1], 3);
				o = 3;
				e->st = 1;
			}
			out[o++] = c == 0xA5 ? 0x5C : 0x7E;
			return o;
		}
		if(key) {
			if(e->st != 2) {
				memcpy(out, jp_esc[3], 3);
				o = 3;
				e->st = 2;
			}
			out[o++] = (char)(key >> 8 & 0x7F);
			out[o++] = (char)(key & 0x7F);
			return o;
		}
		if(e->st) {
			memcpy(out, jp_esc[0], 3);
			o = 3;
			e->st = 0;
		}
		out[o++] = c < 0x80 ? (char)c : '?';
		return o;
	case K_2022KR:
		if(!e->acc) {	/* its header, at the start */
			memcpy(out, kr_esc[0], 4);
			o = 4;
			e->acc = 1;
		}
		key = c < 0x80 ? 0 : set94(t, c);
		if(key) {
			if(!e->st)
				out[o++] = 0x0E;	/* SO */
			e->st = 1;
			out[o++] = (char)(key >> 8 & 0x7F);
			out[o++] = (char)(key & 0x7F);
			return o;
		}
		if(e->st)
			out[o++] = 0x0F;	/* SI */
		e->st = 0;
		out[o++] = c < 0x80 ? (char)c : '?';
		return o;
	case K_HZ:
		key = c < 0x80 ? 0 : set94(t, c);
		if(key) {
			if(!e->st) {
				out[o++] = '~';
				out[o++] = '{';
			}
			e->st = 1;
			out[o++] = (char)(key >> 8 & 0x7F);
			out[o++] = (char)(key & 0x7F);
			return o;
		}
		if(e->st) {
			out[o++] = '~';
			out[o++] = '}';
		}
		e->st = 0;
		if(c == '~')
			out[o++] = '~';
		out[o++] = c < 0x80 ? (char)c : '?';
		return o;
	}
	return 0;
}

static int
ext_has(int e, unsigned long c)
{
	int k = KIND(e);
	const struct encd_mb *t = mb_of(e);

	switch(k) {
	case K_SB:
	case K_MB:
		return rev_key(sb_of(e), t, c) != 0;
	case K_2022JP:	/* an ESC of the text would be read as one of its */
		return c < 0x80 ? c != 0x1B : c == 0xA5 || c == 0x203E || set94(t, c) != 0;
	case K_2022KR:
		return c < 0x80 ? c != 0x0E && c != 0x0F && c != 0x1B : set94(t, c) != 0;
	case K_HZ:
		return c < 0x80 || set94(t, c) != 0;
	}
	return 1;	/* GB18030 and the UTFs: all of Unicode */
}

static pos_t
enc_ext(struct encd *e, const char *in, pos_t n, char *out)
{
	const unsigned char *u = (const unsigned char *)in, *end = u + n;
	pos_t o = 0;
	int k;

	while(e->npend && u < end) {	/* the end of a character cut short */
		e->pend[e->npend++] = *u++;
		if(e->npend == seqlen(e->pend[0])) {
			o += put_ext(e, get_char(e->pend, e->npend), out + o);
			e->npend = 0;
		}
	}
	while(u < end) {
		k = seqlen(*u);
		if(end - u < k) {
			memcpy(e->pend, u, (size_t)(end - u));
			e->npend = (int)(end - u);
			break;
		}
		o += put_ext(e, get_char(u, k), out + o);
		u += k;
	}
	return o;
}

pos_t
enc_end(struct encd *e, char *out)
{
	pos_t o = 0;

	if(ENC_EXT(e->enc))
		switch(KIND(e->enc)) {
		case K_UTF7:
			o = u7end(e, out);
			break;
		case K_2022JP:
			if(e->st) {
				memcpy(out, jp_esc[0], 3);
				o = 3;
			}
			break;
		case K_2022KR:
			if(e->st)
				out[o++] = 0x0F;
			break;
		case K_HZ:
			if(e->st) {
				out[o++] = '~';
				out[o++] = '}';
			}
			break;
		}
	e->st = 0;
	e->acc = 0;
	e->nbits = 0;
	e->npend = 0;
	return o;
}
pos_t
enc_run(struct encd *e, const char *in, pos_t n, char *out)
{
	const unsigned char *u = (const unsigned char *)in, *end = u + n;
	pos_t o = 0;
	int k;

	if(ENC_EXT(e->enc))
		return enc_ext(e, in, n, out);
	if(!ENC_8BIT(e->enc) && !ENC_UTF16(e->enc)) {
		memcpy(out, in, (size_t)n);
		return n;
	}
	while(e->npend && u < end) {	/* the end of a character cut short */
		e->pend[e->npend++] = *u++;
		if(e->npend == seqlen(e->pend[0])) {
			o += put_char(e, get_char(e->pend, e->npend), out + o);
			e->npend = 0;
		}
	}
	while(u < end) {
		if(*u < 0x80 && ENC_8BIT(e->enc)) {
			out[o++] = (char)*u++;
			continue;
		}
		k = seqlen(*u);
		if(end - u < k) {
			memcpy(e->pend, u, (size_t)(end - u));
			e->npend = (int)(end - u);
			break;
		}
		o += put_char(e, get_char(u, k), out + o);
		u += k;
	}
	return o;
}

/* --- what a file not in UTF-8 is in --------------------------------------
 *
 * Its first bytes decoded in each candidate: one that finds bytes that
 * are none in it is out; of the others, the one whose characters are
 * most those of its language (the most frequent of Chinese, simplified
 * and traditional, of Korean; Japanese: its kana; Russian: its most
 * frequent lowercase letters).  A text of the West (Latin letters with
 * accents: a byte above 7Fh alone, or two, between ASCII) is none of
 * them: the 8-bit guess of the caller. */

/* the most frequent characters (code points, in order) */
static const unsigned short top_zhs[128] = {
	0x4E00, 0x4E09, 0x4E0A, 0x4E0B, 0x4E0D, 0x4E0E, 0x4E1A, 0x4E24, 0x4E2A, 0x4E2D,
	0x4E3A, 0x4E3B, 0x4E48, 0x4E4B, 0x4E5F, 0x4E86, 0x4E8B, 0x4E8C, 0x4E8E, 0x4E9B,
	0x4EBA, 0x4ECE, 0x4ED6, 0x4EE5, 0x4EEC, 0x4F1A, 0x4F53, 0x4F5C, 0x4F60, 0x516C,
	0x5173, 0x5176, 0x519B, 0x51FA, 0x5206, 0x5230, 0x524D, 0x52A8, 0x5341, 0x53BB,
	0x53C8, 0x53D1, 0x53EA, 0x53EF, 0x540C, 0x540E, 0x548C, 0x56FD, 0x5728, 0x5730,
	0x5916, 0x591A, 0x5927, 0x5929, 0x597D, 0x5982, 0x5B50, 0x5B66, 0x5B9A, 0x5B9E,
	0x5BB6, 0x5BF9, 0x5C06, 0x5C0F, 0x5C31, 0x5DE5, 0x5DF2, 0x5E74, 0x5F00, 0x5F53,
	0x5F88, 0x5F97, 0x5FC3, 0x610F, 0x6210, 0x6211, 0x6240, 0x628A, 0x6587, 0x65B0,
	0x65B9, 0x65E0, 0x65E5, 0x65F6, 0x662F, 0x6700, 0x6709, 0x672C, 0x673A, 0x6765,
	0x6837, 0x6B64, 0x6CA1, 0x6CD5, 0x70B9, 0x7136, 0x73B0, 0x751F, 0x7528, 0x7535,
	0x7684, 0x770B, 0x7740, 0x77E5, 0x79CD, 0x7ECF, 0x8005, 0x800C, 0x80FD, 0x81EA,
	0x884C, 0x8981, 0x89C1, 0x8BF4, 0x8D77, 0x8FC7, 0x8FD8, 0x8FD9, 0x8FDB, 0x9053,
	0x90A3, 0x90FD, 0x91CC, 0x91CD, 0x957F, 0x95EE, 0x9762, 0x9AD8
};
static const unsigned short top_zht[128] = {
	0x4E00, 0x4E09, 0x4E0A, 0x4E0B, 0x4E0D, 0x4E2D, 0x4E3B, 0x4E4B, 0x4E5F, 0x4E86,
	0x4E8B, 0x4E8C, 0x4E9B, 0x4EBA, 0x4ED6, 0x4EE5, 0x4F5C, 0x4F60, 0x4F86, 0x500B,
	0x5011, 0x5169, 0x516C, 0x5176, 0x51FA, 0x5206, 0x5230, 0x524D, 0x52D5, 0x5341,
	0x53BB, 0x53C8, 0x53EA, 0x53EF, 0x540C, 0x548C, 0x554F, 0x570B, 0x5728, 0x5730,
	0x5916, 0x591A, 0x5927, 0x5929, 0x597D, 0x5982, 0x5B50, 0x5B78, 0x5B9A, 0x5BB6,
	0x5BE6, 0x5C07, 0x5C0D, 0x5C0F, 0x5C31, 0x5DE5, 0x5DF2, 0x5E74, 0x5F88, 0x5F8C,
	0x5F97, 0x5F9E, 0x5FC3, 0x610F, 0x6210, 0x6211, 0x6240, 0x628A, 0x6587, 0x65B0,
	0x65B9, 0x65BC, 0x65E5, 0x662F, 0x6642, 0x6700, 0x6703, 0x6709, 0x672C, 0x696D,
	0x6A23, 0x6A5F, 0x6B64, 0x6C92, 0x6CD5, 0x70BA, 0x7121, 0x7136, 0x73FE, 0x751F,
	0x7528, 0x7576, 0x767C, 0x7684, 0x770B, 0x77E5, 0x7A2E, 0x7D93, 0x8005, 0x800C,
	0x80FD, 0x81EA, 0x8207, 0x8457, 0x884C, 0x88E1, 0x8981, 0x898B, 0x8AAA, 0x8D77,
	0x8ECD, 0x9019, 0x9032, 0x904E, 0x9053, 0x9084, 0x90A3, 0x90FD, 0x91CD, 0x9577,
	0x958B, 0x95DC, 0x96FB, 0x9762, 0x9AD4, 0x9AD8, 0x9EBC, 0x9EDE
};
static const unsigned short top_ko[102] = {
	0xAC00, 0xAC1C, 0xAC70, 0xAC83, 0xAC8C, 0xACBD, 0xACC4, 0xACE0, 0xACF5, 0xACFC,
	0xAD00, 0xAD6C, 0xAD6D, 0xADF8, 0xAE30, 0xAE4C, 0xB098, 0xB0B4, 0xB294, 0xB2C8,
	0xB2E4, 0xB300, 0xB354, 0xB358, 0xB370, 0xB3C4, 0xB3D9, 0xB418, 0xB41C, 0xB420,
	0xB4E4, 0xB54C, 0xB610, 0xB77C, 0xB85C, 0xB97C, 0xB9AC, 0xB9C8, 0xB9CC, 0xBA70,
	0xBA74, 0xBB34, 0xBB38, 0xBB3C, 0xBCF4, 0xBD80, 0xBE44, 0xC0AC, 0xC0C1, 0xC0DD,
	0xC11C, 0xC131, 0xC138, 0xC18C, 0xC218, 0xC2A4, 0xC2B5, 0xC2DC, 0xC2E0, 0xC2E4,
	0xC544, 0xC5B4, 0xC5C5, 0xC5D0, 0xC5EC, 0xC5F0, 0xC624, 0xC640, 0xC694, 0xC6B0,
	0xC6D0, 0xC704, 0xC73C, 0xC740, 0xC744, 0xC758, 0xC774, 0xC778, 0xC77C, 0xC788,
	0xC790, 0xC7A5, 0xC800, 0xC801, 0xC804, 0xC815, 0xC81C, 0xC870, 0xC8FC, 0xC911,
	0xC9C0, 0xD130, 0xD2B8, 0xD558, 0xD559, 0xD55C, 0xD569, 0xD574, 0xD588, 0xD589,
	0xD654, 0xD68C
};
static const unsigned short top_ru[15] = {
	0x0430, 0x0432, 0x0434, 0x0435, 0x0438, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
	0x043F, 0x0440, 0x0441, 0x0442, 0x0443
};

static int
in_top(const unsigned short *t, int n, unsigned long c)
{
	int lo = 0, hi = n, m;

	while(lo < hi) {
		m = (lo + hi) / 2;
		if(t[m] == c)
			return 1;
		if(t[m] < c)
			lo = m + 1;
		else
			hi = m;
	}
	return 0;
}

#define NTOP(t)		((int)(sizeof t / sizeof t[0]))

enum { L_JA, L_ZHS, L_ZHT, L_KO, L_RU };

/* how much c is one of lang's (0 not at all) */
static int
weight(int lang, unsigned long c)
{
	int cjk = c >= 0x4E00 && c <= 0x9FFF, punct = (c >= 0x3000 && c <= 0x303F) ||
	    (c >= 0xFF01 && c <= 0xFF5E);

	switch(lang) {
	case L_JA:
		return c >= 0x3041 && c <= 0x309F ? 6 : c >= 0x30A0 && c <= 0x30FF ? 4 :
		    cjk || punct ? 2 : 0;
	case L_ZHS:
		return in_top(top_zhs, NTOP(top_zhs), c) ? 8 : cjk || punct ? 2 : 0;
	case L_ZHT:
		return in_top(top_zht, NTOP(top_zht), c) ? 8 : cjk || punct ? 2 : 0;
	case L_KO:
		return in_top(top_ko, NTOP(top_ko), c) ? 8 : c >= 0xAC00 && c <= 0xD7A3 ? 2 :
		    punct ? 2 : cjk ? 1 : 0;
	}
	return in_top(top_ru, NTOP(top_ru), c) ? 1 : 0;	/* L_RU: a hit */
}

/* the n bytes p in e: the weight of their characters not ASCII for lang
 * (to *score), how many (*nch), the bytes that are none (*bad) */
static void
score_in(int e, int lang, const char *p, pos_t n, long *score, long *nch, long *bad)
{
	static char out[DEC_ROOM(4096)];
	const unsigned char *u, *end;
	struct dec d;
	pos_t i, k, o;
	unsigned long c;
	int len, q;

	memset(&d, 0, sizeof d);
	d.enc = e;
	*score = *nch = 0;
	for(i = 0; i < n; i += k) {
		k = n - i < 4096 ? n - i : 4096;
		o = dec_run(&d, p + i, k, out, 0);	/* the end may be cut short */
		for(u = (const unsigned char *)out, end = u + o; u < end; u += len) {
			len = *u < 0x80 ? 1 : *u < 0xE0 ? 2 : *u < 0xF0 ? 3 : 4;
			if(len == 1)
				continue;
			c = *u & (0x7F >> len);
			for(q = 1; q < len; q++)
				c = c << 6 | (u[q] & 0x3F);
			(*nch)++;
			*score += weight(lang, c);
		}
	}
	*bad = d.bad;
}

int
enc_detect(const char *p, pos_t n)
{
	static const struct {
		const char *name;
		int lang;
	} cands[] = {
		{ "Shift_JIS", L_JA }, { "windows-31j", L_JA }, { "EUC-JP", L_JA },
		{ "GB18030", L_ZHS }, { "Big5", L_ZHT }, { "windows-950", L_ZHT },
		{ "EUC-KR", L_KO }, { "windows-949", L_KO },
		{ "windows-1251", L_RU }, { "KOI8-R", L_RU }, { "IBM866", L_RU }
	};
	const unsigned char *u = (const unsigned char *)p;
	long high = 0, inruns = 0, run = 0, score, nch, bad, best = -1, bestn = 1;
	int i, e, found = ENC_AUTO;
	pos_t j;

	/* the bytes above 7Fh: in runs of three or more (the words of CJK,
	   of Cyrillic), or alone, two (the accents of the West) */
	for(j = 0; j <= n; j++) {
		if(j < n && u[j] >= 0x80) {
			high++;
			run++;
			continue;
		}
		if(run >= 3)
			inruns += run;
		run = 0;
	}
	if(high == 0 || inruns * 2 < high)
		return ENC_AUTO;
	for(i = 0; i < (int)(sizeof cands / sizeof cands[0]); i++) {
		if((e = enc_find(cands[i].name)) == ENC_AUTO)
			continue;
		score_in(e, cands[i].lang, p, n, &score, &nch, &bad);
		if(nch == 0 || bad * 100 > nch)
			continue;	/* bytes that are none: not it */
		if(cands[i].lang == L_RU) {	/* hits of 15 letters: 4 in 10 */
			if(score * 10 < nch * 4)
				continue;
			score *= 8;	/* as a weight of the others */
		} else if(score < nch * 3)
			continue;	/* not its characters enough */
		/* the best, by the weight of a character (the first of equals) */
		if(best < 0 || score * bestn > best * nch) {
			best = score;
			bestn = nch;
			found = e;
		}
	}
	return found;
}

int
enc_find(const char *name)
{
	int i;

	for(i = 0; i < NDEFS; i++)
		if(strcmp(defs[i].label, name) == 0)
			return defs[i].own >= 0 ? defs[i].own : ENC_N + i;
	return ENC_AUTO;
}
