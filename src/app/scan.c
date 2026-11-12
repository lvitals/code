/* The text of the project's files searched for a pattern, on threads
 * (finder.h).  The finder gives each file it finds (scan_feed); a thread
 * on each core takes one, reads it a block at a time, as grep does, and
 * gives back its lines that match, the file at once (scan_take).
 *
 * A file is only read: opened to read, a block at a time (never whole,
 * never mapped), never written.  One in UTF-8 (ASCII too) is searched as
 * its bytes are; one in another encoding (a BOM, UTF-16 without one) is
 * decoded a block at a time into UTF-8 (enc.h); one that turns out not to
 * be UTF-8 is searched again as Windows-1252, as the editor would open it
 * (a large one, as the editor, by its first block alone: never again).
 * The lines are searched whole: what follows the last newline of a block
 * waits for the next one.  Not searched: a binary file, as grep has it (a
 * zero byte in a block read, and not UTF-16): left when one is seen.
 *
 * What the threads share is under one lock: the pattern and its number
 * (gen: one more for each search; a thread with an older one drops what
 * it found), the files given and not taken, and the results not taken.
 * Each thread has its own compiled pattern: a struct re matches one text
 * at a time. */
#include <stdlib.h>
#include <string.h>
#include "finder.h"
#include "../core/regex.h"

#define THREADS_MAX	8
#define QUEUE_MAX	256		/* files given, not taken yet */
#define BLOCK		(256 * 1024)	/* read at a time */
#define BINARY_SAMPLE	8192
#define LONG_LINE	(8 << 20)	/* a line longer is searched in pieces */
#define LARGE		((pos_t)32 << 20)	/* a file this large has its encoding
					   guessed from its first block, as the
					   editor does (buf.c: it maps it) */
#define FILE_HITS_MAX	100000		/* the lines of a file given, at most */
#define WORK		(4L << 20)	/* steps of a regular expression between
					   two looks: is the search still wanted */
#define LINE_WORK	(256L << 20)	/* steps on one line with no match: the
					   line is passed over (a .* on a line of
					   megabytes) */
#define META		".[]()*+?{}|^$\\"

struct hit {
	pos_t line, col;
};

/* the lines of a file that match, given back */
struct batch {
	struct batch *next;
	int file, n;
	struct hit *h;
};

struct job {
	int file;
	char *path;
};

/* what a thread has of its own */
struct worker {
	unsigned gen;			/* of its pattern */
	struct re *re;			/* or 0: a text */
	int bad;			/* its pattern could not be compiled: no match */
	char lit[EDL_MAX];		/* the text: all the pattern, or the start of re */
	size_t litn;
	int fold;			/* any case */
	char *raw;			/* a block read */
	char *text;			/* the lines being searched, in UTF-8 */
	size_t cap, len;
	struct hit *hits;
	int nhits, hcap;
};

/* shared: under lock */
static struct plat_lock *lock;
static struct plat_thread *threads[THREADS_MAX];
static int nthreads;
static unsigned gen;
static char pattern[EDL_MAX];
static int stopping;
static struct job queue[QUEUE_MAX];
static int qhead, qn;
static int active;			/* files being searched */
static struct batch *done, **done_end = &done;

/* the search g still the one */
static int
current(unsigned g)
{
	int r;

	plat_lock(lock);
	r = g == gen && !stopping;
	plat_unlock(lock);
	return r;
}

/* --- the pattern --------------------------------------------------------- */

static int
lower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

/* the case matters when the pattern has a capital (smart case) */
static int
any_case(const char *pat)
{
	for(; *pat; pat++)
		if(*pat >= 'A' && *pat <= 'Z')
			return 0;
	return 1;
}

/* the pattern of search g, compiled for w: a text when it has no sign of
 * a regular expression; else the expression, and the text it starts with
 * (no | in it; a * ? { takes the character before) */
static void
compile(struct worker *w, const char *pat, unsigned g)
{
	const char *err;
	size_t n;

	if(w->re)
		re_free(w->re);
	w->re = 0;
	w->gen = g;
	w->bad = 0;
	w->fold = any_case(pat);
	if(strpbrk(pat, META) == 0)
		n = strlen(pat);
	else {
		w->re = re_compile(pat, w->fold, &err);
		w->bad = w->re == 0;	/* checked before: only without memory */
		n = strchr(pat, '|') ? 0 : strcspn(pat, META);
		if(n > 0 && (pat[n] == '*' || pat[n] == '?' || pat[n] == '{'))
			n--;
	}
	if(n >= sizeof w->lit)
		n = 0;
	memcpy(w->lit, pat, n);
	w->lit[n] = 0;
	w->litn = n;
}

/* --- a search of lines in memory ----------------------------------------- */

static int
same(const char *a, const char *b, size_t n, int fold)
{
	size_t i;

	if(!fold)
		return memcmp(a, b, n) == 0;
	for(i = 0; i < n; i++)
		if(lower((unsigned char)a[i]) != lower((unsigned char)b[i]))
			return 0;
	return 1;
}

/* the first s (n bytes) in [p, e): its first byte looked for with memchr
 * (both cases of a letter, when any case) */
static const char *
find_text(const char *p, const char *e, const char *s, size_t n, int fold)
{
	int a = (unsigned char)s[0], b = -1;
	const char *q, *q2;

	if(fold && lower(a) >= 'a' && lower(a) <= 'z') {
		a = lower(a);
		b = a - 32;
	}
	while((size_t)(e - p) >= n) {
		q = memchr(p, a, (size_t)(e - p));
		if(b >= 0 && (q2 = memchr(p, b, (size_t)((q ? q : e) - p))) != 0)
			q = q2;
		if(q == 0 || (size_t)(e - q) < n)
			return 0;
		if(same(q, s, n, fold))
			return q;
		p = q + 1;
	}
	return 0;
}

/* the end of the line of t at p (its newline, or n) */
static pos_t
line_end(const char *t, pos_t n, pos_t p)
{
	const char *nl = memchr(t + p, '\n', (size_t)(n - p));

	return nl ? (pos_t)(nl - t) : n;
}

/* the first match in t[from, n): 1, 0 if none, -1 if search g is no more
 * wanted.  The work of a regular expression is counted: between two
 * looks at g, and on a line (too much: the line is passed over) */
static int
next_match(struct worker *w, const char *t, pos_t n, pos_t from, pos_t *at, unsigned g)
{
	const char *q;
	pos_t end, le = POS_NONE;
	long work = 0, line = 0;
	int r;

	if(w->bad)
		return 0;
	for(;;) {
		if(w->re == 0 || w->litn > 0) {	/* where the text is */
			if((q = find_text(t + from, t + n, w->lit, w->litn, w->fold)) == 0)
				return 0;
			*at = (pos_t)(q - t);
			if(w->re == 0 || re_match_mem(w->re, t, n, *at, &end))
				return 1;
			r = 0;
			work += re_work(w->re);
		} else {
			if((r = re_find_mem(w->re, t, n, from, n, WORK, at, &end)) >= 0)
				return r;
			work += WORK;
		}
		/* *at: tried, not a match (or the work given ran out there) */
		if(le == POS_NONE || *at > le) {	/* another line */
			le = line_end(t, n, *at);
			line = 0;
		}
		line += work;
		if(work >= WORK) {
			if(!current(g))
				return -1;
			work = 0;
		}
		if(line > LINE_WORK) {	/* the line passed over */
			from = le + 1;
			le = POS_NONE;
			if(from >= n)
				return 0;
		} else
			from = r < 0 ? *at : *at + 1;
	}
}

static pos_t
newlines(const char *p, pos_t n)
{
	const char *e = p + n;
	pos_t k = 0;

	while(p < e && (p = memchr(p, '\n', (size_t)(e - p))) != 0) {
		k++;
		p++;
	}
	return k;
}

/* the characters of the n bytes at p: those not a continuation byte */
static pos_t
characters(const char *p, pos_t n)
{
	pos_t i, k = 0;

	for(i = 0; i < n; i++)
		k += ((unsigned char)p[i] & 0xC0) != 0x80;
	return k;
}

static void
add_hit(struct worker *w, pos_t line, pos_t col)
{
	struct hit *nh;

	if(w->nhits >= FILE_HITS_MAX)
		return;
	if(w->nhits == w->hcap) {
		w->hcap = w->hcap ? w->hcap * 2 : 64;
		if((nh = realloc(w->hits, (size_t)w->hcap * sizeof *nh)) == 0) {
			w->hcap = w->nhits;
			return;
		}
		w->hits = nh;
	}
	w->hits[w->nhits].line = line;
	w->hits[w->nhits++].col = col;
}

/* the lines of t[0, n) that match (a line each), from line *line on: *line
 * is then that of t + n; 0 if search g is no more wanted */
static int
search_lines(struct worker *w, const char *t, pos_t n, pos_t *line, unsigned g)
{
	pos_t from = 0, at, counted = 0, ls;
	const char *nl;
	int r;

	while(from < n && (r = next_match(w, t, n, from, &at, g)) != 0) {
		if(r < 0)
			return 0;
		*line += newlines(t + counted, at - counted);
		counted = at;
		for(ls = at; ls > 0 && t[ls - 1] != '\n'; ls--)
			;
		add_hit(w, *line, characters(t + ls, at - ls));
		nl = memchr(t + at, '\n', (size_t)(n - at));
		from = nl ? (pos_t)(nl - t) + 1 : n;
	}
	*line += newlines(t + counted, n - counted);
	return 1;
}

/* --- a file -------------------------------------------------------------- */

static int
room(struct worker *w, size_t need)
{
	char *nt;
	size_t cap = w->cap ? w->cap : BLOCK;

	if(need <= w->cap)
		return 1;
	while(cap < need)
		cap *= 2;
	if((nt = realloc(w->text, cap)) == 0)
		return 0;
	w->text = nt;
	w->cap = cap;
	return 1;
}

/* the whole lines in the text searched (and taken out of it); all of it
 * at the end, or when a line is too long: 0 if search g is no more wanted */
static int
search_text(struct worker *w, pos_t *line, int last, unsigned g)
{
	const char *nl = 0;
	size_t k;

	if(!last) {
		for(k = w->len; k > 0 && w->text[k - 1] != '\n'; k--)
			;
		if(k > 0)
			nl = w->text + k - 1;
	}
	k = last || (nl == 0 && w->len > LONG_LINE) ? w->len : nl ? (size_t)(nl - w->text) + 1 : 0;
	if(k == 0)
		return 1;
	if(!search_lines(w, w->text, (pos_t)k, line, g))
		return 0;
	memmove(w->text, w->text + k, w->len - k);
	w->len -= k;
	return 1;
}

enum { F_DONE, F_DROPPED, F_NOT_UTF8 };

/* the file at path searched for the pattern of search g, read in enc
 * (ENC_AUTO: from its first bytes) */
static int
search_file(struct worker *w, const char *path, unsigned g, int enc)
{
	struct dec d;
	struct u8check u;
	pos_t line = 0, size;
	int h, n, skip, first = 1, check = 0, r = F_DONE;

	if((h = file_open(path, FILE_SCAN)) < 0)
		return F_DROPPED;
	size = file_size(h);
	memset(&d, 0, sizeof d);
	memset(&u, 0, sizeof u);
	w->len = 0;
	w->nhits = 0;
	for(;;) {
		/* the first read small: a binary file is known by it */
		n = file_read(h, w->raw, first ? BINARY_SAMPLE : BLOCK);
		if(n <= 0)
			break;
		skip = 0;
		if(first) {
			first = 0;
			if(memchr(w->raw, 0, (size_t)n) &&
			   !ENC_UTF16(enc_sniff_bom(w->raw, n, &skip)) &&
			   !ENC_UTF16(enc_sniff_utf16(w->raw, n))) {
				r = F_DROPPED;	/* binary */
				break;
			}
			d.enc = enc_sniff_bom(w->raw, n, &skip);
			if(enc != ENC_AUTO)
				d.enc = enc;
			else if(d.enc == ENC_AUTO && (d.enc = enc_sniff_utf16(w->raw, n)) == ENC_AUTO) {
				d.enc = ENC_UTF8;
				check = 1;	/* UTF-8 as long as it is */
				if(size != POS_NONE && size >= LARGE) {	/* by its start */
					check = 0;
					if(!u8_check(&u, w->raw, n, 0))
						d.enc = ENC_CP1252;
				}
			}
			if(d.enc == ENC_UTF8_BOM)
				d.enc = ENC_UTF8;
		}
		/* a zero byte in any block (not UTF-16): binary, as grep has it */
		if(!ENC_UTF16(d.enc) && memchr(w->raw, 0, (size_t)n)) {
			r = F_DROPPED;
			break;
		}
		if(check && !u8_check(&u, w->raw + skip, n - skip, 0)) {
			r = F_NOT_UTF8;
			break;
		}
		if(!room(w, w->len + (size_t)DEC_ROOM(n))) {
			r = F_DROPPED;
			break;
		}
		if(d.enc == ENC_UTF8) {
			memcpy(w->text + w->len, w->raw + skip, (size_t)(n - skip));
			w->len += (size_t)(n - skip);
		} else
			w->len += (size_t)dec_run(&d, w->raw + skip, n - skip, w->text + w->len, 0);
		if(!search_text(w, &line, 0, g) || !current(g)) {
			r = F_DROPPED;
			break;
		}
	}
	if(r == F_DONE && check && !u8_check(&u, "", 0, 1))
		r = F_NOT_UTF8;	/* a character cut at the end */
	if(r == F_DONE) {	/* the end: what is left */
		if(d.enc != ENC_UTF8 && room(w, w->len + (size_t)DEC_ROOM(0)))
			w->len += (size_t)dec_run(&d, "", 0, w->text + w->len, 1);
		if(!search_text(w, &line, 1, g))
			r = F_DROPPED;
	}
	file_close(h);
	return r;
}

/* --- the threads --------------------------------------------------------- */

static void
work(void *arg)
{
	struct worker *w = arg;
	struct batch *b;
	struct job j;
	char pat[EDL_MAX];
	unsigned g;
	int r;

	for(;;) {
		plat_lock(lock);
		while(!stopping && qn == 0)
			plat_wait(lock, -1);
		if(stopping) {
			plat_unlock(lock);
			break;
		}
		j = queue[qhead];
		qhead = (qhead + 1) % QUEUE_MAX;
		qn--;
		g = gen;
		if(w->gen != g)
			strcpy(pat, pattern);
		active++;
		plat_wake(lock);	/* room in the queue */
		plat_unlock(lock);

		if(w->gen != g)
			compile(w, pat, g);
		r = search_file(w, j.path, g, ENC_AUTO);
		if(r == F_NOT_UTF8)	/* as the editor opens it */
			r = search_file(w, j.path, g, ENC_CP1252);
		b = 0;
		if(r == F_DONE && w->nhits > 0 && (b = malloc(sizeof *b)) != 0) {
			b->next = 0;
			b->file = j.file;
			b->n = w->nhits;
			if((b->h = malloc((size_t)w->nhits * sizeof *b->h)) == 0) {
				free(b);
				b = 0;
			} else
				memcpy(b->h, w->hits, (size_t)w->nhits * sizeof *b->h);
		}
		free(j.path);

		plat_lock(lock);
		active--;
		if(b && g == gen) {
			*done_end = b;
			done_end = &b->next;
			b = 0;
		}
		plat_wake(lock);
		plat_unlock(lock);
		if(b) {		/* of a search that is no more */
			free(b->h);
			free(b);
		}
	}
	if(w->re)
		re_free(w->re);
	free(w->raw);
	free(w->text);
	free(w->hits);
	free(w);
}

/* the lock held: the files given and the results dropped */
static void
drop_all(void)
{
	struct batch *b;

	for(; qn > 0; qn--) {
		free(queue[qhead].path);
		qhead = (qhead + 1) % QUEUE_MAX;
	}
	while((b = done) != 0) {
		done = b->next;
		free(b->h);
		free(b);
	}
	done_end = &done;
}

static int
start_threads(void)
{
	struct worker *w;
	int i, n = plat_cpus();

	if(n > THREADS_MAX)
		n = THREADS_MAX;
	if(lock == 0 && (lock = plat_lock_new()) == 0)
		return 0;
	for(i = nthreads; i < n; i++) {
		if((w = calloc(1, sizeof *w)) == 0 || (w->raw = malloc(BLOCK)) == 0) {
			free(w);
			break;
		}
		w->gen = (unsigned)-1;
		if((threads[nthreads] = plat_thread_start(work, w)) == 0) {
			free(w->raw);
			free(w);
			break;
		}
		nthreads++;
	}
	return nthreads > 0;
}

/* --- the finder's side --------------------------------------------------- */

int
scan_start(const char *pat, const char **err)
{
	struct re *r;

	scan_cancel();		/* the one before, whatever comes of this one */
	*err = 0;
	if(pat[0] == 0)
		return 0;
	if(strpbrk(pat, META)) {	/* said here; each thread compiles its own */
		if((r = re_compile(pat, any_case(pat), err)) == 0)
			return 0;
		re_free(r);
	}
	if(strlen(pat) >= sizeof pattern || !start_threads()) {
		*err = "The search cannot be made";
		return 0;
	}
	plat_lock(lock);	/* the pattern and its number at once */
	gen++;
	strcpy(pattern, pat);
	plat_unlock(lock);
	return 1;
}

void
scan_cancel(void)
{
	if(lock == 0)
		return;
	plat_lock(lock);
	gen++;
	drop_all();
	plat_wake(lock);
	plat_unlock(lock);
}

int
scan_feed(int file, const char *path)
{
	size_t n = strlen(path) + 1;
	char *p;

	if(lock == 0 || (p = malloc(n)) == 0)
		return 0;
	memcpy(p, path, n);
	plat_lock(lock);
	if(qn == QUEUE_MAX) {
		plat_unlock(lock);
		free(p);
		return 0;
	}
	queue[(qhead + qn) % QUEUE_MAX].file = file;
	queue[(qhead + qn++) % QUEUE_MAX].path = p;
	plat_wake(lock);
	plat_unlock(lock);
	return 1;
}

int
scan_take(void (*hit)(int file, pos_t line, pos_t col), int ms)
{
	struct batch *b, *list;
	int i, any = 0;

	if(lock == 0)
		return 0;
	plat_lock(lock);
	if(done == 0 && ms > 0 && (qn > 0 || active > 0))
		plat_wait(lock, ms);
	list = done;
	done = 0;
	done_end = &done;
	plat_unlock(lock);
	while((b = list) != 0) {
		list = b->next;
		for(i = 0; i < b->n; i++)
			hit(b->file, b->h[i].line, b->h[i].col);
		any = 1;
		free(b->h);
		free(b);
	}
	return any;
}

int
scan_busy(void)
{
	int r;

	if(lock == 0)
		return 0;
	plat_lock(lock);
	r = qn > 0 || active > 0 || done != 0;
	plat_unlock(lock);
	return r;
}

void
scan_stop(void)
{
	int i;

	if(lock == 0)
		return;
	plat_lock(lock);
	stopping = 1;
	gen++;
	drop_all();
	plat_wake(lock);
	plat_unlock(lock);
	for(i = 0; i < nthreads; i++)
		plat_thread_join(threads[i]);
	nthreads = 0;
	stopping = 0;
	plat_lock_free(lock);
	lock = 0;
}
