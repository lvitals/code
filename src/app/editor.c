/* The text area: the margin with the line numbers, the visual rows (a
 * long line wraps at a space when Word Wrap is on), the scrollbar, the
 * selection, and the keys and the mouse. */
#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "../core/mem.h"
#include "syntax.h"

static int textw = 80, viewh = 20, margin;	/* the last layout */
static int viewx, viewy;			/* the corner of the text */
pos_t hit = POS_NONE;	/* the start of the last match found */
static pos_t drag = POS_NONE;	/* where the mouse was pressed */
/* the cursors moved together: the one moving now (-1: none), and the
   edits of all of them one undo */
static int mc_active = -1;
static char mc_batch;
static int mc_cut(int cut);
static void mc_paste(void);
static void mc_toggle(pos_t p);
static int mc_at(pos_t i);

char *clip;
pos_t cliplen;
char clip_line;

/* --- the selection ------------------------------------------------------- */

int
has_sel(void)
{
	return D->anchor != POS_NONE && D->anchor != D->cur;
}

pos_t
sel_begin(void)
{
	return has_sel() && D->anchor < D->cur ? D->anchor : D->cur;
}

pos_t
sel_end(void)
{
	return has_sel() && D->anchor > D->cur ? D->anchor : D->cur;
}

char *
sel_text(void)
{
	pos_t a = sel_begin(), n = sel_end() - a;
	char *s;

	if(n == 0 || (s = mem_alloc(n + 1)) == 0)
		return 0;
	buf_copy(a, n, s);
	s[n] = 0;
	return s;
}

/* --- the visual rows ----------------------------------------------------- */

/* the columns of the character at i, at column col */
static int
charw(pos_t i, int col)
{
	rune c = buf_char(i);

	if(c == '\t')
		return B->tabsize - col % B->tabsize;
	if(c < ' ' || c == 127)
		return 1;
	return cwidth(c);
}

/* the end of the row from rs (its text), and the start of the next */
static pos_t
row_end(pos_t rs, pos_t *next)
{
	pos_t i = rs, len = buf_len(), brk = POS_NONE;
	int col = 0, w, c;

	if(!D->wrap) {
		i = buf_lineend(rs);
		*next = i < len ? i + 1 : POS_NONE;
		return i;
	}
	while(i < len) {
		c = BCH(i);
		if(c == '\n') {
			*next = i + 1;
			return i;
		}
		w = charw(i, col);
		if(col + w > textw && col > 0) {
			*next = brk != POS_NONE ? brk : i;
			return *next;
		}
		col += w;
		i = buf_next(i);
		if(c == ' ' || c == '\t')
			brk = i;
	}
	*next = POS_NONE;
	return len;
}

/* --- a long line: checkpoints ------------------------------------------- */

/* Along the line of the cursor, every CKSTEP bytes or more: the column
 * there (without Word Wrap), or the start of a row (with it).  A column
 * or a row is then found from the checkpoint before it, not from the
 * start of the line: moving along a line of any length is fast.  An edit
 * cuts those after it; an undo all of them. */
#define CKSTEP		((pos_t)16384)
static struct {
	struct buf *b;
	pos_t ls;
	int tab, wrap, w;
	pos_t *off;
	long *col;
	int n, cap;
} ck;
unsigned edit_gen;	/* one more for each change of the text */

static void
ck_reset(void)
{
	ck.b = 0;
	ck.n = 0;
}

static void
ck_cut(pos_t off)
{
	int lo = 0, hi = ck.n, m;

	if(ck.b != B)
		return;
	if(off < ck.ls) {
		ck_reset();
		return;
	}
	while(lo < hi) {	/* the first at or after off */
		m = (lo + hi) / 2;
		if(ck.off[m] < off)
			lo = m + 1;
		else
			hi = m;
	}
	ck.n = lo;
}

/* the checkpoints of the line from ls, as the view is */
static void
ck_line(pos_t ls)
{
	if(ck.b == B && ck.ls == ls && ck.tab == B->tabsize && ck.wrap == D->wrap &&
	   (!D->wrap || ck.w == textw))
		return;
	ck.b = B;
	ck.ls = ls;
	ck.tab = B->tabsize;
	ck.wrap = D->wrap;
	ck.w = textw;
	ck.n = 0;
}

static void
ck_add(pos_t off, long col)
{
	pos_t *o;
	long *c;
	int cap;

	if(off - (ck.n ? ck.off[ck.n - 1] : ck.ls) < CKSTEP)
		return;
	if(ck.n == ck.cap) {
		cap = ck.cap ? ck.cap * 2 : 64;
		if((o = realloc(ck.off, cap * sizeof *o)) == 0)
			return;
		ck.off = o;
		if((c = realloc(ck.col, cap * sizeof *c)) == 0)
			return;
		ck.col = c;
		ck.cap = cap;
	}
	ck.off[ck.n] = off;
	ck.col[ck.n++] = col;
}

/* the last checkpoint at or before off (by == 0) or column x (by == 1) */
static void
ck_find(int by, pos_t v, pos_t *off, long *col)
{
	int lo = 0, hi = ck.n, m;

	while(lo < hi) {
		m = (lo + hi) / 2;
		if(by ? (pos_t)ck.col[m] <= v : ck.off[m] <= v)
			lo = m + 1;
		else
			hi = m;
	}
	*off = lo ? ck.off[lo - 1] : ck.ls;
	*col = lo ? ck.col[lo - 1] : 0;
}

static pos_t
row_start(pos_t off)
{
	pos_t rs = buf_linestart(off), nx;
	long c;

	if(!D->wrap)
		return rs;
	ck_line(rs);
	ck_find(0, off, &rs, &c);
	for(;;) {
		row_end(rs, &nx);
		if(nx == POS_NONE || off < nx)
			return rs;
		rs = nx;
		ck_add(rs, 0);
	}
}

static pos_t
row_prev(pos_t rs)
{
	return rs == 0 ? POS_NONE : row_start(rs - 1);
}

static pos_t
row_next(pos_t rs)
{
	pos_t nx;

	row_end(rs, &nx);
	return nx;
}

static int
col_of(pos_t rs, pos_t off)
{
	pos_t i = rs, n;
	long col = 0;

	if(off > buf_len())	/* past the end: buf_next would not go on */
		off = buf_len();
	if(!D->wrap && rs == buf_linestart(rs)) {	/* from a checkpoint */
		ck_line(rs);
		ck_find(0, off, &i, &col);
	}
	while(i < off) {
		if((n = buf_plain(i, off)) > 0) {	/* a column a byte */
			i += n;
			col += (long)n;
		} else {
			col += charw(i, (int)col);
			i = buf_next(i);
		}
		if(!D->wrap)
			ck_add(i, col);
	}
	return (int)col;
}

/* the first character of the line from ls that ends past column x: its
 * column at *col (without Word Wrap) */
static pos_t
seek_col(pos_t ls, pos_t re, long x, long *col)
{
	pos_t i, n;
	int w;

	ck_line(ls);
	ck_find(1, (pos_t)x, &i, col);
	while(i < re) {
		if((n = buf_plain(i, re)) > 0) {
			if(*col + (long)n > x)
				n = (pos_t)(x - *col);
			i += n;
			*col += (long)n;
			if(*col >= x && x >= 0)
				break;
		} else {
			w = charw(i, (int)*col);
			if(*col + w > x)
				break;
			*col += w;
			i = buf_next(i);
		}
		ck_add(i, *col);
	}
	return i;
}

/* the offset at column x of the row from rs */
static pos_t
at_col(pos_t rs, int x)
{
	pos_t re, nx, i;
	int col = 0, w;
	long c;

	if(x < 0)	/* left of the text (a click on the line numbers): its start */
		x = 0;
	re = row_end(rs, &nx);
	if(!D->wrap)
		return seek_col(rs, re, x, &c);
	for(i = rs; i < re; i = buf_next(i)) {
		w = charw(i, col);
		if(col + w > x)
			return i;
		col += w;
	}
	if(nx == re && re > rs)	/* a wrapped row: its end is the next */
		return buf_prev(re);
	return re;
}

#define row_col(rs, off)	col_of(rs, off)

void
cursor_visible(void)
{
	pos_t rs = row_start(D->cur), r, p, d;
	int n, col;

	if(rs < D->top)
		D->top = rs;
	else if(!D->wrap) {	/* by the newlines between: no line numbers */
		d = buf_count(D->top, rs, (pos_t)viewh + 4);
		if(d >= (pos_t)viewh) {
			if(d - viewh < 4)
				for(p = d - viewh + 1; p > 0; p--)
					D->top = buf_lineend(D->top) + 1;
			else
				for(D->top = rs, n = 1; n < viewh && D->top > 0; n++)
					D->top = buf_linestart(D->top - 1);
		}
		(void)r;
	} else {
		r = D->top;
		for(n = 0; n < viewh && r != POS_NONE; n++) {
			if(r == rs)
				goto seen;
			r = row_next(r);
		}
		D->top = rs;
		for(n = 1; n < viewh; n++) {
			if((p = row_prev(D->top)) == POS_NONE)
				break;
			D->top = p;
		}
	}
seen:
	if(D->wrap) {
		D->left = 0;
		return;
	}
	col = row_col(rs, D->cur);
	if(col < D->left)
		D->left = col;
	if(col >= D->left + textw)
		D->left = col - textw + 1;
}

/* --- the edits ----------------------------------------------------------- */

/* the next command starts (not while each of the cursors does it) */
static void
begin(void)
{
	if(!mc_batch)
		buf_begin();
}

static void
adjust(pos_t *x, pos_t off, pos_t ndel, pos_t n)
{
	if(*x == POS_NONE || *x < off)
		return;
	if(*x < off + ndel)
		*x = off;
	else
		*x = *x - ndel + n;
}

int
edit(pos_t off, pos_t ndel, const char *s, pos_t n, int kind)
{
	int k;

	if(mc_batch)
		kind = U_OTHER;	/* all the cursors' edits: one undo */
	if(edit_low == POS_NONE) {	/* what the next drawing need not do */
		edit_low = off;
		edit_high = off + n;
		edit_delta = n - ndel;
	} else
		rows_stale = 1;		/* two edits: all of it */
	if(!buf_replace(off, ndel, s, n, D->cur, kind)) {
		error("Out of memory", 0);
		return 0;
	}
	ck_cut(off);
	edit_gen++;
	/* the lines it added: too many, all of it drawn again */
	if(B->dlines == DL_MANY || B->dlines > 10000 || B->dlines < -10000) {
		edit_lines = 0;
		rows_stale = 1;
	} else
		edit_lines = (int)B->dlines;
	D->pref = -1;
	if(hit != POS_NONE && hit >= off)
		hit = POS_NONE;
	adjust(&D->mark, off, ndel, n);
	jump_adjust(off, ndel, n);
	if(D->nmc && mc_active < 0)
		mc_clear();	/* an edit of one cursor: the others go */
	else
		for(k = 0; k < D->nmc; k++)
			if(k != mc_active) {
				adjust(&D->mc[k].cur, off, ndel, n);
				adjust(&D->mc[k].anchor, off, ndel, n);
			}
	return 1;
}

/* s in place of the selection (or at the cursor); typed: a key */
void
write_text(const char *s, pos_t n, int typed)
{
	pos_t a = D->cur, del = 0, e;

	if(has_sel()) {
		a = sel_begin();
		del = sel_end() - a;
	} else if(typed && D->overtype && n && s[0] != '\n' &&
	    D->cur < buf_lineend(D->cur)) {
		e = buf_next(D->cur);
		del = e - D->cur;
	}
	if(!typed || del)
		begin();
	if(edit(a, del, s, n, typed && !del ? U_TYPE : U_OTHER)) {
		D->cur = a + n;
		D->anchor = POS_NONE;
	}
}

static void
delete_range(pos_t a, pos_t e)
{
	if(e <= a)
		return;
	begin();
	if(edit(a, e - a, "", 0, U_OTHER)) {
		D->cur = a;
		D->anchor = POS_NONE;
	}
}

/* the indentation of the line from ls: its bytes and its columns, up to
 * max columns */
static int
indent_of(pos_t ls, int max, int *cols)
{
	pos_t i = ls, len = buf_len();
	int col = 0, n;

	for(; i < len; i++) {
		if(BCH(i) == ' ')
			n = col + 1;
		else if(BCH(i) == '\t')
			n = col + B->tabsize - col % B->tabsize;
		else
			break;
		if(n > max)
			break;
		col = n;
	}
	*cols = col;
	return (int)(i - ls);
}

/* the indentation of a new line, or a Tab: cols columns of it */
static int
make_indent(int from, int cols, char *s, int max)
{
	int n = 0, c = from;

	if(B->tabs)
		while(n < max && c + (B->tabsize - c % B->tabsize) <= from + cols) {
			s[n++] = '\t';
			c += B->tabsize - c % B->tabsize;
		}
	while(n < max && c < from + cols) {
		s[n++] = ' ';
		c++;
	}
	return n;
}

static void
newline(void)
{
	char s[1 + 80];
	int cols, n;
	pos_t ls = buf_linestart(has_sel() ? sel_begin() : D->cur);
	pos_t lim = (has_sel() ? sel_begin() : D->cur) - ls;

	if(D->autoindent) {
		n = indent_of(ls, 10000, &cols);
		if((pos_t)n > lim)
			indent_of(ls, col_of(ls, ls + lim), &cols);
	} else
		cols = 0;
	s[0] = '\n';
	n = make_indent(0, cols, s + 1, 80);
	write_text(s, n + 1, 0);
}

static void
indent(int dir)
{
	pos_t s1 = sel_begin(), s2 = sel_end(), l, nlines, ls, last;
	int ic, cc, pc, pcols, n;
	char s[16];

	if(dir > 0 && buf_linestart(s1) == buf_linestart(s2)) {
		ls = buf_linestart(s1);
		cc = col_of(ls, s1);
		n = B->tabs ? (s[0] = '\t', 1) : make_indent(cc, B->tabsize - cc % B->tabsize, s, 16);
		write_text(s, n, 0);
		return;
	}
	ls = buf_linestart(s1);
	last = buf_linestart(s2);
	if(last > ls && s2 == last)
		last = buf_linestart(s2 - 1);
	nlines = buf_count(ls, last, POS_NONE);	/* the lines after the first */
	begin();
	for(l = 0; l <= nlines; l++, ls = buf_lineend(ls) + 1) {
		ic = indent_of(ls, 10000, &cc);
		if(dir < 0) {
			if(cc == 0)
				continue;
			pc = indent_of(ls, (cc - 1) / B->tabsize * B->tabsize, &pcols);
			if(!edit(ls + pc, ic - pc, "", 0, U_OTHER))
				return;
			adjust(&D->cur, ls + pc, ic - pc, 0);
			adjust(&D->anchor, ls + pc, ic - pc, 0);
		} else {
			if(BCH(ls) == '\n' || ls == buf_len())
				continue;	/* an empty line stays */
			n = make_indent(cc, B->tabsize - cc % B->tabsize, s, 16);
			if(!edit(ls + ic, 0, s, n, U_OTHER))
				return;
			if(D->cur >= ls + ic && !(D->cur == ls + ic && D->cur == s2 && l == nlines && nlines > 0))
				D->cur += n;
			if(D->anchor != POS_NONE && D->anchor >= ls + ic)
				D->anchor += n;
		}
	}
}

static void
backspace(int word)
{
	pos_t ls, i;
	int cols, pc, pcols;

	if(has_sel()) {
		delete_range(sel_begin(), sel_end());
		return;
	}
	if(D->cur == 0)
		return;
	ls = buf_linestart(D->cur);
	for(i = ls; i < D->cur && (BCH(i) == ' ' || BCH(i) == '\t'); i++)
		;
	/* in the indentation: a whole level (or, with the option off, a
	   character, as a space typed is taken) */
	if(i == D->cur && D->cur > ls && D->bsindent) {
		cols = col_of(ls, D->cur);
		pc = word ? 0 : indent_of(ls, (cols - 1) / B->tabsize * B->tabsize, &pcols);
		delete_range(ls + pc, D->cur);
		return;
	}
	delete_range(word ? buf_word(D->cur, -1) : buf_prev(D->cur), D->cur);
}

static void
move_lines(int dir)
{
	pos_t b1 = buf_linestart(sel_begin()), t = sel_end(), e2;
	pos_t a, m, e, n1, n2, shift;
	char *s;
	int nl;

	if(has_sel() && buf_linestart(t) > b1 && t == buf_linestart(t))
		t--;	/* the selection ends at a line start: not that line */
	e2 = buf_lineend(t);	/* the end of the last line moved */
	if(dir < 0 ? b1 == 0 : e2 >= buf_len())
		return;
	a = dir < 0 ? buf_linestart(b1 - 1) : b1;
	m = dir < 0 ? b1 : e2 + 1;
	e = dir < 0 ? e2 : buf_lineend(e2 + 1);
	nl = e < buf_len();	/* the last line moves: it has no newline */
	e += nl;
	n1 = m - a;		/* the first part, and the second */
	n2 = e - m;
	if((s = mem_alloc(e - a + 1)) == 0) {
		error("Out of memory", 0);
		return;
	}
	buf_copy(m, n2, s);
	if(nl)
		buf_copy(a, n1, s + n2);
	else {
		s[n2] = '\n';
		buf_copy(a, n1 - 1, s + n2 + 1);
	}
	begin();
	if(edit(a, e - a, s, e - a, U_OTHER)) {
		shift = dir < 0 ? 0 : n2 + !nl;
		if(dir < 0) {
			D->cur -= n1;
			if(D->anchor != POS_NONE)
				D->anchor -= n1;
		} else {
			D->cur += shift;
			if(D->anchor != POS_NONE)
				D->anchor += shift;
		}
	}
	mem_free(s);
}


static void
set_clip(const char *s, pos_t n, int line)
{
	mem_free(clip);
	clip = mem_alloc(n + 1);
	cliplen = clip ? n : 0;
	if(clip) {
		memcpy(clip, s, n);
		clip[n] = 0;
		term_clipboard(clip, n);
	}
	clip_line = (char)line;
	ui_clip = clip;
	ui_cliplen = cliplen;
}

void
cmd_cut(int cut)
{
	pos_t a, e;
	char *s;
	int line = !has_sel();

	if(D->nmc && mc_active < 0 && mc_cut(cut))
		return;		/* the selections of all the cursors */
	if(line) {
		a = buf_linestart(D->cur);
		e = buf_lineend(D->cur);
		e += e < buf_len();
	} else {
		a = sel_begin();
		e = sel_end();
	}
	if((s = mem_alloc(e - a + 2)) == 0) {
		error("Out of memory", 0);
		return;
	}
	buf_copy(a, e - a, s);
	if(line && (e == a || s[e - a - 1] != '\n'))
		s[e - a] = '\n';	/* the last line: as one with an end */
	set_clip(s, e - a + (line && (e == a || s[e - a - 1] != '\n')), line);
	mem_free(s);
	if(cut)
		delete_range(a, e);
}

void
cmd_paste(void)
{
	pos_t col;

	if(!clip || !cliplen)
		return;
	if(D->nmc && mc_active < 0) {
		mc_paste();	/* at each cursor */
		return;
	}
	if(clip_line && !has_sel()) {
		col = D->cur - buf_linestart(D->cur);
		D->cur -= col;
		write_text(clip, cliplen, 0);
		D->cur += col;
	} else
		write_text(clip, cliplen, 0);
}

void
cmd_undo(int redo)
{
	pos_t p = buf_undo(redo);

	mc_clear();
	rows_stale = 1;
	ck_reset();
	edit_gen++;
	if(p == POS_NONE)
		return;
	D->cur = p > buf_len() ? buf_len() : p;
	D->anchor = POS_NONE;
	D->pref = -1;
	hit = POS_NONE;
}

void
cmd_selectall(void)
{
	mc_clear();
	D->anchor = 0;
	D->cur = buf_len();
}

/* Delete to the line head/tail, and the cursor mark. */
void
cmd_delete_linepart(int tail)
{
	pos_t a, e;

	if(has_sel()) {
		delete_range(sel_begin(), sel_end());
		return;
	}
	a = tail ? D->cur : buf_linestart(D->cur);
	e = tail ? buf_lineend(D->cur) : D->cur;
	delete_range(a, e);
}

void
cmd_mark(int jump)
{
	if(jump) {
		if(D->mark == POS_NONE)
			return;
		jump_push();
		D->cur = D->mark > buf_len() ? buf_len() : D->mark;
		D->anchor = POS_NONE;
		D->pref = -1;
		cursor_visible();
	} else
		D->mark = D->cur;
}

void
cmd_goto(pos_t line, pos_t col)
{
	pos_t ls = buf_lineoff(line), i;
	pos_t n;

	jump_push();
	for(i = ls, n = 0; n < col && i < buf_len() && BCH(i) != '\n'; n++)
		i = buf_next(i);
	D->cur = i;
	D->anchor = POS_NONE;
	D->pref = -1;
}

/* the terminal pasted: CR LF and CR are LF */
static void
paste_raw(void)
{
	size_t n, i, o = 0;
	const char *p = term_paste(&n);
	char *s;

	if(!p || !n || (s = mem_alloc(n)) == 0)
		return;
	for(i = 0; i < n; i++)
		if(p[i] == '\r') {
			s[o++] = '\n';
			if(i + 1 < n && p[i + 1] == '\n')
				i++;
		} else
			s[o++] = p[i];
	write_text(s, o, 0);
	mem_free(s);
}

/* --- the keys ------------------------------------------------------------ */

static void
vmove_rows(int rows, int shift)
{
	pos_t rs, r;
	int n;

	if(has_sel() && !shift) {
		D->cur = rows < 0 ? sel_begin() : sel_end();
		D->pref = -1;
	}
	rs = row_start(D->cur);
	if(D->pref < 0)
		D->pref = col_of(rs, D->cur);
	for(n = 0; n != rows; n += rows < 0 ? -1 : 1) {
		r = rows < 0 ? row_prev(rs) : row_next(rs);
		if(r == POS_NONE) {
			D->cur = rows < 0 ? 0 : buf_len();
			D->pref = rows < 0 ? 0 : -1;
			return;
		}
		rs = r;
	}
	D->cur = at_col(rs, D->pref);
}

static void
home(void)
{
	pos_t ls = buf_linestart(D->cur), rs = row_start(D->cur), ie;
	int cols;

	ie = ls + indent_of(ls, 10000, &cols);
	if(rs != ls && D->cur != rs)
		D->cur = rs;
	else if(D->cur > ie || D->cur == ls)
		D->cur = ie;
	else
		D->cur = ls;
}

static void
end(void)
{
	pos_t rs = row_start(D->cur), nx, re = row_end(rs, &nx);

	if(nx == re && re > rs)
		re = buf_prev(re);	/* a wrapped row */
	D->cur = D->cur == re ? buf_lineend(D->cur) : re;
}

/* key k at the cursor: 0 if it is not for the editor */
static int
key_one(kcode k)
{
	kcode c = K_CODE(k), m = k & K_MODS;
	int shift = (m & K_SHIFT) != 0, move = 1, scroll = 1;
	char u[4];

	if(m & K_SHIFT && (c == K_LEFT || c == K_RIGHT || c == K_UP || c == K_DOWN ||
	    c == K_HOME || c == K_END || c == K_PGUP || c == K_PGDN)) {
		if(D->anchor == POS_NONE || !has_sel())
			D->anchor = D->cur;
	}
	switch(k & ~K_SHIFT) {
	case K_LEFT:
		if(has_sel() && !shift)
			D->cur = sel_begin();
		else
			D->cur = buf_prev(D->cur);
		break;
	case K_RIGHT:
		if(has_sel() && !shift)
			D->cur = sel_end();
		else
			D->cur = buf_next(D->cur);
		break;
	case K_CTRL | K_LEFT:
		D->cur = buf_word(D->cur, -1);
		break;
	case K_CTRL | K_RIGHT:
		D->cur = buf_word(D->cur, 1);
		break;
	case K_UP:
		vmove_rows(-1, shift);
		break;
	case K_DOWN:
		vmove_rows(1, shift);
		break;
	case K_PGUP:
		vmove_rows(-(viewh - 1), shift);
		break;
	case K_PGDN:
		vmove_rows(viewh - 1, shift);
		break;
	case K_HOME:
		home();
		break;
	case K_END:
		end();
		break;
	case K_CTRL | K_HOME:
		if(!shift)
			jump_push();
		D->cur = 0;
		break;
	case K_CTRL | K_END:
		if(!shift)
			jump_push();
		D->cur = buf_len();
		break;
	case K_CTRL | K_UP:
	case K_CTRL | K_DOWN:
		if(c == K_UP) {
			if(row_prev(D->top) != POS_NONE)
				D->top = row_prev(D->top);
		} else if(row_next(D->top) != POS_NONE)
			D->top = row_next(D->top);
		scroll = 0;
		break;
	default:
		move = 0;
	}
	if(move) {
		if(!shift && scroll)
			D->anchor = POS_NONE;
		if(c != K_UP && c != K_DOWN && c != K_PGUP && c != K_PGDN)
			D->pref = -1;
		B->merge = 0;
		if(scroll)
			cursor_visible();
		return 1;
	}
	switch(k) {
	case 8:
	case K_CTRL | 'H':
	case K_CTRL | 8:
		backspace(k != 8);
		break;
	case K_DEL:
	case K_CTRL | K_DEL:
		if(has_sel())
			delete_range(sel_begin(), sel_end());
		else
			delete_range(D->cur, k == K_DEL ? buf_next(D->cur) : buf_word(D->cur, 1));
		break;
	case 13:
		newline();
		break;
	case 9:
		indent(1);
		break;
	case K_SHIFT | 9:
		indent(-1);
		break;
	case 27:
		if(D->nmc)
			mc_clear();	/* one cursor again */
		else
			D->anchor = POS_NONE;
		scroll = 0;
		break;
	case K_INS:
		D->overtype = !D->overtype;
		break;
	case K_SHIFT | K_INS:
	case K_CTRL | 'V':
		cmd_paste();
		break;
	case K_CTRL | K_INS:
	case K_CTRL | 'C':
		cmd_cut(0);
		break;
	case K_SHIFT | K_DEL:
	case K_CTRL | 'X':
		cmd_cut(1);
		break;
	case K_CTRL | 'A':
		cmd_selectall();
		break;
	case K_CTRL | 'L':
		D->anchor = buf_linestart(has_sel() ? sel_begin() : D->cur);
		D->cur = buf_lineend(D->cur);
		D->cur += D->cur < buf_len();
		break;
	case K_CTRL | 'Z':
		cmd_undo(0);
		break;
	case K_CTRL | 'Y':
	case K_CTRL | K_SHIFT | 'Z':
		cmd_undo(1);
		break;
	case K_ALT | 'Z':
		app_action(A_WRAP);
		break;
	case K_ALT | K_UP:
		move_lines(-1);
		break;
	case K_ALT | K_DOWN:
		move_lines(1);
		break;
	case K_PASTE:
		paste_raw();
		break;
	case K_CTRL | ' ':
		complete_start();
		break;
	case K_CTRL | ']':
		cmd_gomatch();
		break;
	case K_CTRL | 'D':
		mc_next();
		break;
	case K_SHIFT | K_ALT | K_UP:
	case K_SHIFT | K_ALT | K_DOWN:
		mc_vertical(c == K_UP ? -1 : 1);
		break;
	case K_SHIFT | K_ALT | K_RIGHT:
	case K_SHIFT | K_ALT | K_LEFT:
		cmd_expand(c == K_RIGHT);
		break;
	default:
		if(!(m & (K_CTRL | K_ALT)) && c >= ' ' && c < K_UP && c != 127) {
			write_text(u, ui_utf8(c, u), 1);
			break;
		}
		return 0;	/* not for the editor */
	}
	if(scroll)
		cursor_visible();
	return 1;
}


/* --- the cursors moved together ----------------------------------------- */

static pos_t
mlo(const struct mcur *m)
{
	return m->anchor != POS_NONE && m->anchor < m->cur ? m->anchor : m->cur;
}

static pos_t
mhi(const struct mcur *m)
{
	return m->anchor != POS_NONE && m->anchor > m->cur ? m->anchor : m->cur;
}

static int
mc_order(const void *a, const void *b)
{
	pos_t x = mlo(a), y = mlo(b);

	return x < y ? -1 : x > y;
}

static int
mc_room(int n)
{
	struct mcur *p;
	int cap;

	if(n <= D->mccap)
		return 1;
	cap = D->mccap ? D->mccap * 2 : 16;
	while(cap < n)
		cap *= 2;
	if((p = realloc(D->mc, (size_t)cap * sizeof *p)) == 0) {
		error("Out of memory", 0);
		return 0;
	}
	D->mc = p;
	D->mccap = cap;
	return 1;
}

void
mc_clear(void)
{
	if(docs == 0 || D->nmc == 0)
		return;
	D->nmc = 0;
	rows_stale = 1;
	ui_again = 1;
}

/* the document's cursor among the others, marked: all in order */
static int
mc_gather(void)
{
	struct mcur *m;

	if(!mc_room(D->nmc + 1))
		return 0;
	m = &D->mc[D->nmc++];
	m->cur = D->cur;
	m->anchor = D->anchor;
	m->pref = D->pref;
	m->prim = 1;
	qsort(D->mc, (size_t)D->nmc, sizeof *D->mc, mc_order);
	return 1;
}

/* the cursors that meet made one; the document's taken out of them */
static void
mc_scatter(void)
{
	struct mcur *m = D->mc, *o;
	int i, n = 0;

	qsort(m, (size_t)D->nmc, sizeof *m, mc_order);
	for(i = 0; i < D->nmc; i++) {
		if(m[i].anchor == m[i].cur)
			m[i].anchor = POS_NONE;
		o = n ? &m[n - 1] : 0;
		if(o && (mlo(&m[i]) < mhi(o) || mlo(&m[i]) == mlo(o) ||
		    (mlo(&m[i]) == mhi(o) && m[i].anchor == POS_NONE && o->anchor == POS_NONE))) {
			if(mhi(&m[i]) > mhi(o)) {	/* one, as wide as both */
				if(o->anchor == POS_NONE || o->cur > o->anchor)
					o->cur = mhi(&m[i]);
				else
					o->anchor = mhi(&m[i]);
			}
			o->prim |= m[i].prim;
			continue;
		}
		m[n++] = m[i];
	}
	for(i = 0; i < n && !m[i].prim; i++)
		;
	if(i == n)
		i = n - 1;
	D->cur = m[i].cur;
	D->anchor = m[i].anchor;
	D->pref = m[i].pref;
	memmove(m + i, m + i + 1, (size_t)(n - i - 1) * sizeof *m);
	D->nmc = n - 1;
	for(i = 0; i < D->nmc; i++)
		D->mc[i].prim = 0;
}

/* op at each cursor, in the order of the text (i: its place in it) */
static void
mc_each(void (*op)(int i, const void *arg), const void *arg, int edits)
{
	int i, prim = 0;

	if(!mc_gather())
		return;
	if(edits)
		buf_begin();
	mc_batch = (char)edits;
	for(i = 0; i < D->nmc; i++) {
		mc_active = i;
		D->cur = D->mc[i].cur;
		D->anchor = D->mc[i].anchor;
		D->pref = D->mc[i].pref;
		op(i, arg);
		D->mc[i].cur = D->cur;
		D->mc[i].anchor = D->anchor;
		D->mc[i].pref = D->pref;
		prim |= D->mc[i].prim;
	}
	mc_active = -1;
	mc_batch = 0;
	(void)prim;
	mc_scatter();
	B->merge = 0;
	rows_stale = 1;
	cursor_visible();
}

static void
op_key(int i, const void *arg)
{
	(void)i;
	key_one(*(const kcode *)arg);
}

/* a key for all the cursors: 1 if it was one */
static int
mc_key(kcode k)
{
	kcode c = K_CODE(k), m = k & K_MODS, base = k & ~K_SHIFT;
	int moves = base == K_LEFT || base == K_RIGHT || base == K_UP || base == K_DOWN ||
	    base == K_HOME || base == K_END || base == (K_CTRL | K_LEFT) ||
	    base == (K_CTRL | K_RIGHT);
	int edits = k == 8 || k == (K_CTRL | 8) || k == (K_CTRL | 'H') || k == K_DEL ||
	    k == (K_CTRL | K_DEL) || k == 13 || k == 9 || k == (K_SHIFT | 9) || k == K_PASTE ||
	    (!(m & (K_CTRL | K_ALT)) && c >= ' ' && c < K_UP && c != 127);

	if(moves || edits) {
		mc_each(op_key, &k, edits);
		return 1;
	}
	/* for the editor, the cursors kept: copy, cut, paste (each does them
	   all), more cursors, Esc, the view; any other: one cursor again */
	if(k == (K_CTRL | 'C') || k == (K_CTRL | K_INS) || k == (K_CTRL | 'X') ||
	   k == (K_SHIFT | K_DEL) || k == (K_CTRL | 'V') || k == (K_SHIFT | K_INS) ||
	   k == (K_CTRL | 'D') || k == (K_SHIFT | K_ALT | K_UP) || k == (K_SHIFT | K_ALT | K_DOWN) ||
	   k == 27 || k == K_INS || k == (K_CTRL | K_UP) || k == (K_CTRL | K_DOWN) ||
	   k == (K_ALT | 'Z') || k == K_MOUSE || k == K_NONE)
		return 0;
	mc_clear();
	return 0;
}

void
editor_key(void)
{
	kcode k = ui_key;

	if(k == 0 || ui_focus != ID_EDITOR)
		return;
	ui_key = 0;	/* the last pass shows what it did: no other is asked */
	if(D->nmc && mc_key(k))
		return;
	if(!key_one(k))
		ui_key = k;	/* not for the editor */
}

/* one cursor more, where the document's is; that one to [a, b) */
static void
mc_add(pos_t anchor, pos_t cur)
{
	struct mcur *m;

	if(!mc_room(D->nmc + 1))
		return;
	m = &D->mc[D->nmc++];
	m->cur = D->cur;
	m->anchor = D->anchor;
	m->pref = D->pref;
	m->prim = 0;
	D->anchor = anchor;
	D->cur = cur;
	D->pref = -1;
	if(!mc_gather())
		return;
	mc_scatter();
	rows_stale = 1;
	ui_again = 1;
	cursor_visible();
}

void
mc_next(void)
{
	pos_t a, at, n;
	char *s;
	int i;

	if(docs == 0)
		return;
	if(!has_sel()) {	/* the word at the cursor, selected first */
		a = buf_word(buf_next(D->cur), -1);
		if(a > D->cur)
			a = D->cur;
		if(buf_word(a, 1) > a && isword(BCH(a))) {
			D->anchor = a;
			D->cur = buf_word(a, 1);
			while(D->cur > a && !isword(BCH(D->cur - 1)))
				D->cur--;
			cursor_visible();
		}
		return;
	}
	if((s = sel_text()) == 0)
		return;
	n = sel_end() - sel_begin();
	if(strlen(s) == (size_t)n && buf_find(sel_end(), s, FIND_CASE, &at)) {
		for(i = 0; i < D->nmc && !(mlo(&D->mc[i]) == at && mhi(&D->mc[i]) == at + n); i++)
			;
		if(i == D->nmc && at != sel_begin())	/* not one already */
			mc_add(at, at + n);
	}
	mem_free(s);
}

void
mc_vertical(int dir)
{
	pos_t rs, r;
	int col;

	if(docs == 0)
		return;
	rs = row_start(D->cur);
	col = D->pref >= 0 ? D->pref : col_of(rs, D->cur);
	r = dir < 0 ? row_prev(rs) : row_next(rs);
	if(r == POS_NONE)
		return;
	mc_add(POS_NONE, at_col(r, col));
	D->pref = col;
}

/* Alt+Click: a cursor at p, or none there if there was one */
static void
mc_toggle(pos_t p)
{
	int i;

	for(i = 0; i < D->nmc; i++)
		if(D->mc[i].cur == p) {
			memmove(D->mc + i, D->mc + i + 1, (size_t)(D->nmc - i - 1) * sizeof *D->mc);
			D->nmc--;
			rows_stale = 1;
			return;
		}
	if(p == D->cur && D->nmc) {	/* the document's: the last other takes it */
		D->nmc--;
		D->cur = D->mc[D->nmc].cur;
		D->anchor = D->mc[D->nmc].anchor;
		D->pref = -1;
		rows_stale = 1;
		return;
	}
	mc_add(POS_NONE, p);
}

/* the cursor at i (2), or i in the selection of another (1), or none */
static int
mc_at(pos_t i)
{
	int lo = 0, hi = D->nmc - 1, mid;
	const struct mcur *m;

	while(lo <= hi) {	/* the last that starts at i or before */
		mid = (lo + hi) / 2;
		if(mlo(&D->mc[mid]) <= i)
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	if(hi < 0)
		return 0;
	m = &D->mc[hi];
	if(m->cur == i)
		return 2;
	return i < mhi(m) ? 1 : 0;
}

/* copy (cut): the selections of all the cursors, a line each */
static void
op_text(int i, const void *arg)
{
	char **out = (char **)((void **)arg)[0];
	pos_t *n = (pos_t *)((void **)arg)[1];

	if(i > 0)
		(*out)[(*n)++] = '\n';
	if(has_sel()) {
		buf_copy(sel_begin(), sel_end() - sel_begin(), *out + *n);
		*n += sel_end() - sel_begin();
	}
}

static void
op_delete(int i, const void *arg)
{
	(void)i;
	(void)arg;
	if(has_sel())
		delete_range(sel_begin(), sel_end());
}

static int
mc_cut(int cut)
{
	pos_t total = 0, n = 0;
	char *s;
	void *args[2];
	int i, any = has_sel();

	for(i = 0; i < D->nmc; i++) {
		total += mhi(&D->mc[i]) - mlo(&D->mc[i]) + 1;
		any |= D->mc[i].anchor != POS_NONE && D->mc[i].anchor != D->mc[i].cur;
	}
	if(!any)
		return 0;	/* no selection: as with one cursor */
	total += sel_end() - sel_begin() + 1;
	if((s = mem_alloc(total)) == 0) {
		error("Out of memory", 0);
		return 1;
	}
	args[0] = &s;
	args[1] = &n;
	mc_each(op_text, args, 0);
	set_clip(s, n, 0);
	mem_free(s);
	if(cut)
		mc_each(op_delete, 0, 1);
	return 1;
}

/* paste: a line of the clipboard to each cursor, when it has as many;
   else all of it to each */
static void
op_paste(int i, const void *arg)
{
	const char *p = clip, *e = clip + cliplen, *q;
	int lines = *(const int *)arg, k;

	if(lines) {
		for(k = 0; k < i && p < e; k++) {
			while(p < e && *p != '\n')
				p++;
			p += p < e;
		}
		for(q = p; q < e && *q != '\n'; q++)
			;
		write_text(p, (pos_t)(q - p), 0);
	} else
		write_text(clip, cliplen, 0);
}

static void
mc_paste(void)
{
	pos_t i, parts = 1;
	int lines;

	for(i = 0; i < cliplen; i++)
		if(clip[i] == '\n' && i + 1 < cliplen)
			parts++;
	lines = parts == (pos_t)D->nmc + 1;
	mc_each(op_paste, &lines, 1);
}

/* what the scrollbar shows: where the top is, of how much, and how much
 * is on the screen; by the lines, or the bytes while they are not counted */
static void
bar_measure(pos_t *pos, pos_t *total, pos_t *shown)
{
	pos_t e;
	int k;

	if(bar_lines()) {
		*pos = buf_lineno(D->top);
		*total = buf_lines();
		*shown = (pos_t)viewh;
		return;
	}
	*pos = D->top;
	*total = buf_len();
	for(e = D->top, k = 0; k < viewh && e != POS_NONE && e < buf_len(); k++)
		e = row_next(e);	/* the bytes of the rows shown */
	*shown = (e == POS_NONE || e > buf_len() ? buf_len() : e) - D->top;
	if(*shown == 0)
		*shown = 1;
}

/* the top as the scrollbar asks at row of it (its middle there) */
static void
thumb_to(int row)
{
	pos_t pos, total, shown, p;
	int k;

	bar_measure(&pos, &total, &shown);
	p = (pos_t)ui_scrollbar_at(viewh, (long)total, (long)shown, row);
	if(bar_lines())
		D->top = buf_lineoff(p);
	else {
		D->top = buf_linestart(p > buf_len() ? buf_len() : p);
		if(p + shown >= buf_len())	/* the end: the last screen of rows */
			for(D->top = buf_linestart(buf_len()), k = 1; k < viewh && D->top > 0; k++)
				D->top = row_start(D->top - 1);
	}
}

static char sbdrag;	/* the thumb of the scrollbar is being dragged */
static char hsbdrag;	/* the one of the horizontal scrollbar */
static char hbar;	/* without Word Wrap: the horizontal scrollbar shown */
static long hbar_total = 1;	/* the columns it measures */
static long hb_left = -1, hb_total, hb_margin;	/* as drawn last */

/* the columns of the text from rs to re (a line too long: its bytes) */
static long
row_width(pos_t rs, pos_t re)
{
	pos_t i = rs, n;
	long col = 0;

	if(re - rs > ((pos_t)1 << 20))
		return (long)(re - rs);
	while(i < re) {
		if((n = buf_plain(i, re)) > 0) {
			i += n;
			col += (long)n;
		} else {
			col += charw(i, (int)col);
			i = buf_next(i);
		}
	}
	return col;
}

/* the widest of the n rows from the top (with the column after its end) */
static long
widest_row(int n)
{
	pos_t rs = D->top, re, len = buf_len();
	long w, max = 0;
	int k;

	for(k = 0; k < n && rs <= len; k++) {
		re = buf_lineend(rs);
		if((w = row_width(rs, re) + 1) > max)
			max = w;
		if(re >= len)
			break;
		rs = re + 1;
	}
	return max;
}

/* what the horizontal bar measures: the widest row shown, or as far as
 * the view is scrolled */
static long
hbar_measure(void)
{
	long max = widest_row(viewh), at = (long)D->left + textw;

	return max > at ? max : at;
}

/* a horizontal bar, in h rows: without Word Wrap, when a row is wider
 * than the window or the view is scrolled sideways */
static int
hbar_needed(int h)
{
	return !D->wrap && h > 3 && (D->left > 0 || widest_row(h) > (long)textw);
}

/* the first column shown, kept within what the bar measures */
static void
set_left(long left)
{
	long most = hbar_total - textw;

	if(left > most)
		left = most;
	D->left = left > 0 ? (int)left : 0;
}

/* the mouse on the text: a click puts the cursor, a drag selects (and
 * scrolls past the edges), a double click takes a word and a triple one
 * the line, the middle button pastes, the right one opens the context
 * menu, the wheel scrolls */
/* a and b are lines apart: what a click there is, a jump */
#define FAR_LINES	((pos_t)10)
static int
far(pos_t a, pos_t b)
{
	return buf_count(a < b ? a : b, a < b ? b : a, FAR_LINES) >= FAR_LINES;
}

static void
editor_mouse(void)
{
	struct mouse *ms = &term_mouse;
	pos_t rs = D->top, r, p;
	int y, n, my = ms->y;

	if(ui_key != K_MOUSE || !ui_mouse_ok() || ms->x < 0 || ms->x >= scr_cols ||
	   ms->kind == M_MOVE)
		return;
	if(ms->kind == M_RELEASE) {
		drag = POS_NONE;
		sbdrag = hsbdrag = 0;
		return;
	}
	/* the horizontal scrollbar (the row under the text): an arrow
	   scrolls 4 columns, the track and a drag put the thumb there */
	if(hbar && ((ms->kind == M_PRESS && ms->button == MB_LEFT && my == viewy + viewh &&
	    ms->x >= margin && ms->x < scr_cols - 1) || (ms->kind == M_DRAG && hsbdrag))) {
		n = ms->x - margin;
		y = scr_cols - 1 - margin;	/* its width */
		if(ms->kind == M_PRESS && n == 0)
			set_left((long)D->left - 4);
		else if(ms->kind == M_PRESS && n == y - 1)
			set_left((long)D->left + 4);
		else if(y >= 3) {
			hsbdrag = 1;
			set_left(ui_scrollbar_at(y, hbar_total, textw, n < 1 ? 1 : n > y - 2 ? y - 2 : n));
		}
		ui_key = 0;
		ui_again = 1;
		return;
	}
	/* the wheel sideways, or Shift and the wheel: the columns */
	if(!D->wrap && my >= viewy && my <= viewy + viewh &&
	   (ms->kind == M_WHEELLEFT || ms->kind == M_WHEELRIGHT ||
	    ((ms->kind == M_WHEELUP || ms->kind == M_WHEELDOWN) && (ms->mods & K_SHIFT)))) {
		set_left((long)D->left + (ms->kind == M_WHEELLEFT || ms->kind == M_WHEELUP ? -6 : 6));
		ui_key = 0;
		ui_again = 1;
		return;
	}
	/* the scrollbar (the last column): an arrow scrolls a row; a click
	   on the track puts the thumb there, and a drag carries it (off the
	   bar too, until the button is let go) */
	if((ms->kind == M_PRESS && ms->button == MB_LEFT && ms->x >= scr_cols - 1 &&
	    my >= viewy && my < viewy + viewh) || (ms->kind == M_DRAG && sbdrag)) {
		n = my - viewy;
		if(ms->kind == M_PRESS && (n == 0 || n == viewh - 1) && viewh >= 3) {
			r = n == 0 ? row_prev(D->top) : row_next(D->top);
			if(r != POS_NONE)
				D->top = r;
		} else if(viewh >= 3) {
			sbdrag = 1;
			thumb_to(n);
		}
		ui_key = 0;
		ui_again = 1;
		return;
	}
	if(ms->kind == M_DRAG && drag != POS_NONE && (my < viewy || my >= viewy + viewh)) {
		/* past an edge: the text scrolls under the selection */
		r = my < viewy ? row_prev(D->top) : row_next(D->top);
		if(r != POS_NONE)
			D->top = r;
		my = my < viewy ? viewy : viewy + viewh - 1;
	} else if(my < viewy || my >= viewy + viewh)
		return;
	if(ms->kind == M_WHEELUP || ms->kind == M_WHEELDOWN) {
		for(n = 0; n < 3; n++) {
			r = ms->kind == M_WHEELUP ? row_prev(D->top) : row_next(D->top);
			if(r == POS_NONE)
				break;
			D->top = r;
		}
		ui_key = 0;
		ui_again = 1;
		return;
	}
	if(ms->kind == M_DRAG && drag == POS_NONE)
		return;
	for(y = viewy; y < my; y++) {
		r = row_next(rs);
		if(r == POS_NONE)
			break;
		rs = r;
	}
	if(y < my && ms->kind == M_PRESS)
		p = buf_len();
	else
		p = at_col(rs, ms->x - viewx + D->left);
	ui_steal(ID_EDITOR);
	if(ms->kind == M_PRESS && ms->button != MB_LEFT) {
		/* outside the selection: the cursor goes there first */
		if(!has_sel() || p < sel_begin() || p >= sel_end()) {
			D->cur = p;
			D->anchor = POS_NONE;
		}
		drag = POS_NONE;
		if(ms->button == MB_MIDDLE)
			cmd_paste();
		else {
			st.mx = ms->x;
			st.my = ms->y;
			open_menu(CTX_MENU);
		}
	} else if(ms->kind == M_PRESS) {
		if(ms->clicks == 3) {	/* the line, with its newline */
			D->anchor = buf_linestart(p);
			D->cur = buf_lineend(p);
			D->cur += D->cur < buf_len();
			drag = POS_NONE;
		} else if(ms->clicks == 2) {
			D->anchor = buf_word(buf_next(p), -1);
			if(D->anchor > p)
				D->anchor = p;
			D->cur = buf_word(D->anchor, 1);
			drag = POS_NONE;
		} else if(ms->mods & K_ALT) {
			mc_toggle(p);	/* a cursor more (or less) */
			drag = POS_NONE;
			ui_key = 0;
			ui_again = 1;
			return;
		} else {
			mc_clear();
			if(ms->mods & K_SHIFT) {
				if(!has_sel())
					D->anchor = D->cur;
			} else {
				D->anchor = p;
				if(far(D->cur, p))
					jump_push();	/* a jump, to come back from */
			}
			D->cur = p;
			drag = D->anchor;
		}
	} else
		D->cur = p;
	D->pref = -1;
	B->merge = 0;
	cursor_visible();
	ui_key = 0;
	ui_again = 1;
}

/* --- the drawing --------------------------------------------------------- */

/* the scrollbar by lines: when they are counted, or few */
int
bar_lines(void)
{
	return buf_len() <= SMALL_TEXT || buf_lines_known();
}

static int
digits(pos_t n)
{
	char t[12];

	return udec(t, n);
}

char drawing = 1;	/* this pass shows the text: the last one */
char rows_stale = 1;	/* something was over the text: all of it again */
pos_t edit_low = POS_NONE;	/* the edit since the text was shown: */
pos_t edit_high, edit_delta;	/* from where the text only moved, by */

/* What each row of the text area showed: a row the same, before the
 * first byte edited, is not drawn again (a key
 * usually changes one) */
#define SIGMAX		256
struct rowsig {
	pos_t rs, re, nx, line, s1, s2;
	char mv;	/* moved where the margin had no number: drawn */
	int hs;		/* the state of the syntax at its line */
	char cl;	/* drawn as the line of the cursor */
};
static struct rowsig sig[SIGMAX];
static int sig_y, sig_h, sig_left, sig_nd, sig_tab, sig_focus, sig_blank;
static pos_t sig_top, sig_s1, sig_s2, bar_pos, bar_total, bar_shown;
static char sig_mode = -1, sig_known, sig_dim, sig_curline, sig_hl;
static int sig_lang;
static pos_t sig_curls, sig_ma = POS_NONE, sig_mb = POS_NONE;
static int sig_ruler, sig_nmc;
static long sig_cur;
static const struct rowsig sig_none = {	/* matches no row: drawn */
	POS_NONE, POS_NONE, POS_NONE, POS_NONE - 1, POS_NONE, POS_NONE, 0, -2, 0
};

/* --- the highlighting of the rows shown ---------------------------------- */

#define LEX_BACK	((pos_t)65536)	/* lexed before the top, for its state */
#define LEX_MAX		((pos_t)65536)	/* the most of a line lexed at once */
static char hl;			/* this pass highlights the syntax */
static char mcur;		/* the margin is on the line of the cursor */
static pos_t curls;		/* the start of the line of the cursor */
static struct {			/* the state at the start of each line shown */
	pos_t ls[SIGMAX + 2];
	int st[SIGMAX + 2], n;
	struct buf *b;
	pos_t top;
	unsigned gen;
	int lang, h;
} hls;
static struct {			/* the classes of the bytes of one line */
	char *text;
	unsigned char *cls;
	size_t cap;
	pos_t a, b, ls;
	struct buf *buf;
	unsigned gen;
	int lang, st;
} lx;

/* the bytes [a, b) of a line lexed from state st (their classes in lx
 * if keep): the state after them */
static int
lex(pos_t a, pos_t b, int st, int keep)
{
	size_t n = (size_t)(b - a);
	char *t;
	unsigned char *c;

	if(n > lx.cap) {
		if((t = realloc(lx.text, n)) == 0)
			return st;
		lx.text = t;
		if((c = realloc(lx.cls, n)) == 0)
			return st;
		lx.cls = c;
		lx.cap = n;
	}
	buf_copy(a, (pos_t)n, lx.text);
	st = syn_line(D->lang, lx.text, (int)n, st, keep ? lx.cls : 0);
	if(keep) {
		lx.a = a;
		lx.b = b;
	}
	return st;
}

/* the state at the start of the line from ls, lexing on from there */
static int
lex_line_state(pos_t ls, int st)
{
	pos_t e = buf_lineend(ls);

	return lex(ls, e - ls > LEX_MAX ? ls + LEX_MAX : e, st, 0);
}

/* the states of the lines shown, from a window of text before the top:
 * what is before it is not read (a comment open since then is not seen) */
static void
hl_prepare(int h)
{
	pos_t ls, a, len = buf_len();
	int st = 0, k;

	hl = D->syntax && syn_has(D->lang);
	curls = buf_linestart(D->cur);
	if(!hl || (hls.b == B && hls.top == D->top && hls.gen == edit_gen &&
	   hls.lang == D->lang && hls.h == h))
		return;
	hls.b = B;
	hls.top = D->top;
	hls.gen = edit_gen;
	hls.lang = D->lang;
	hls.h = h;
	lx.buf = 0;
	ls = buf_linestart(D->top);
	a = ls > LEX_BACK ? buf_linestart(ls - LEX_BACK) : 0;
	if(ls - a > 2 * LEX_BACK)
		a = ls;
	while(a < ls) {
		st = lex_line_state(a, st);
		a = buf_lineend(a) + 1;
	}
	for(k = 0; k <= h && k < SIGMAX + 1; k++) {
		hls.ls[k] = ls;
		hls.st[k] = st;
		if(ls >= len)
			break;
		st = lex_line_state(ls, st);
		ls = buf_lineend(ls) + 1;
	}
	hls.n = k + 1;
}

/* the state at the start of the line of row rs */
static int
hl_state(pos_t rs)
{
	pos_t ls;
	int k;

	if(!hl || rs == POS_NONE)
		return -1;
	ls = buf_linestart(rs);
	for(k = 0; k < hls.n; k++)
		if(hls.ls[k] == ls)
			return hls.st[k];
	return 0;
}

/* the classes of the line from ls (state st) around byte i: in lx */
static void
hl_line(pos_t ls, int st, pos_t i)
{
	pos_t e, a;

	if(lx.buf == B && lx.gen == edit_gen && lx.lang == D->lang && lx.ls == ls &&
	   lx.st == st && i >= lx.a && (i < lx.b || lx.b == lx.a))
		return;
	e = buf_lineend(ls);
	a = ls;
	if(e - ls > LEX_MAX) {	/* a long line: a window of it */
		a = i - ls > 4096 ? i - 4096 : ls;
		if(e - a > LEX_MAX)
			e = a + LEX_MAX;
	}
	lex(a, e, st, 1);
	lx.buf = B;
	lx.gen = edit_gen;
	lx.lang = D->lang;
	lx.ls = ls;
	lx.st = st;
}

/* --- indent guides -------------------------------------------------------- */

/* the columns of the blanks the line from ls starts with; *blank: it has
 * nothing else */
static int
indent_cols(pos_t ls, int *blank)
{
	pos_t i, len = buf_len();
	int col = 0, c;

	for(i = ls; i < len && col < 4096; i++) {
		c = BCH(i);
		if(c == ' ')
			col++;
		else if(c == '\t')
			col += B->tabsize - col % B->tabsize;
		else
			break;
	}
	*blank = i >= len || BCH(i) == '\n' || BCH(i) == '\r';
	return col;
}

/* how deep the guides of the line from ls go: its indentation, or (a
 * blank line in a block) that of the lines around it, the lesser */
static int
guide_cols(pos_t ls)
{
	pos_t p, len = buf_len();
	int blank, ind = indent_cols(ls, &blank), k, prev = 0, next = 0;

	if(!blank)
		return ind;
	for(p = ls, k = 0; p > 0 && k < 64; k++) {
		p = buf_linestart(p - 1);
		ind = indent_cols(p, &blank);
		if(!blank) {
			prev = ind;
			break;
		}
	}
	for(p = ls, k = 0; k < 64; k++) {
		p = buf_lineend(p);
		if(p >= len)
			break;
		ind = indent_cols(++p, &blank);
		if(!blank) {
			next = ind;
			break;
		}
	}
	return prev < next ? prev : next;
}

/* --- the block at the cursor: where it starts and ends ------------------- */

/* A bracket, or what opens or closes a block in the language (its blocks
 * in langs.c: if ... fi, function ... end, #if ... #endif), at the cursor:
 * it and its match are shown.  The text around the cursor is lexed, so
 * that a keyword is one (not in a string or a comment) and a bracket
 * matches one of its own kind of text (code with code, a string's with
 * its string's). */
#define MATCH_SPAN	((pos_t)65536)	/* looked at on each side */
static struct {
	pos_t a, alen, b, blen;		/* the two, or a POS_NONE */
	struct buf *buf;
	pos_t cur;
	unsigned gen;
	int lang;
	pos_t len;
} bm = { POS_NONE, 0, POS_NONE, 0, 0, POS_NONE, 0, -1, 0 };

struct mdef {
	char kind;			/* k, o, p */
	const char *open, *close;
};
struct mtok {
	pos_t at;
	int len, def;
	char open, cls;
};

static int
mword(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
	    c == '_' || c >= 0x80;
}

static int
msame(const char *p, const char *s, int n, int nocase)
{
	int i;

	for(i = 0; i < n; i++)
		if(nocase ? ((p[i] | 32) != (s[i] | 32) || !p[i]) : p[i] != s[i])
			return 0;
	return 1;
}

static void
block_match(void)
{
	static char *text;
	static unsigned char *cls;
	static struct mtok *tk;
	static size_t cap, tkcap;
	struct mdef defs[80];
	const struct syn *sy = syns[D->lang < nsyns ? D->lang : 0];
	const char *const *b;
	pos_t lo, hi, len = buf_len(), n, i, ls;
	int nd = 0, ntk = 0, k, j, d, st, nocase = (sy->flags & F_NOCASE) != 0, me = -1, wl, lex;
	struct mtok *t, *u;
	char *nt;
	unsigned char *nc;
	struct mtok *ntkp;

	if(bm.buf == B && bm.cur == D->cur && bm.gen == edit_gen && bm.lang == D->lang &&
	   bm.len == len)
		return;
	bm.buf = B;
	bm.len = len;
	bm.cur = D->cur;
	bm.gen = edit_gen;
	bm.lang = D->lang;
	bm.a = bm.b = POS_NONE;
	/* what opens and closes: the language's, and the brackets */
	for(b = sy->blocks; b && b[0] && nd < 70; b += 3, nd++) {
		defs[nd].kind = b[0][0];
		defs[nd].open = b[1];
		defs[nd].close = b[2];
	}
	for(k = 0; k < 3; k++) {
		static const char *const br[3][2] = { { "(", ")" }, { "[", "]" }, { "{", "}" } };

		for(j = 0; j < nd && !(defs[j].kind == 'o' && strcmp(defs[j].open, br[k][0]) == 0); j++)
			;
		if(j == nd) {
			defs[nd].kind = 'o';
			defs[nd].open = br[k][0];
			defs[nd++].close = br[k][1];
		}
	}
	/* the text around the cursor, lexed from a line start (state 0) */
	lo = buf_linestart(D->cur > MATCH_SPAN ? D->cur - MATCH_SPAN : 0);
	hi = buf_lineend(len - D->cur > MATCH_SPAN ? D->cur + MATCH_SPAN : len);
	if(hi < len)
		hi++;
	n = hi - lo;
	if(n == 0)
		return;
	if(n > cap) {
		if((nt = realloc(text, n)) == 0)
			return;
		text = nt;
		if((nc = realloc(cls, n)) == 0)
			return;
		cls = nc;
		cap = n;
	}
	buf_copy(lo, n, text);
	lex = syn_has(D->lang);
	for(i = 0, st = 0; i < n; i = ls) {
		for(ls = i; ls < n && text[ls] != '\n'; ls++)
			;
		if(lex)
			st = syn_line(D->lang, text + i, (int)(ls - i), st, cls + i);
		else
			memset(cls + i, HL_TEXT, ls - i);
		if(ls < n)
			cls[ls++] = HL_TEXT;
	}
	/* the tokens of the blocks, in order */
	for(i = 0; i < n; i++) {
		int at_word = mword((unsigned char)text[i]) && (i == 0 || !mword((unsigned char)text[i - 1]));

		for(wl = 0; at_word && i + wl < n && mword((unsigned char)text[i + wl]); wl++)
			;
		for(k = 0; k < nd; k++) {
			for(j = 0; j < 2; j++) {
				const char *w = j ? defs[k].close : defs[k].open;
				int wn = (int)strlen(w);

				if(i + wn > n || !msame(text + i, w, wn, nocase))
					continue;
				if(defs[k].kind == 'k' && (!at_word || wl != wn ||
				    (lex && cls[i] != HL_KEYWORD)))
					continue;
				if(defs[k].kind == 'p' && (i + wn < n && mword((unsigned char)text[i + wn])))
					continue;	/* #if, not #ifdef */
				if(defs[k].kind == 'p' && lex && cls[i] != HL_PREPROC)
					continue;
				if(defs[k].kind == 'o' && lex && (cls[i] == HL_KEYWORD || cls[i] == HL_TYPE))
					continue;
				break;
			}
			if(j < 2)
				break;
		}
		if(k == nd)
			continue;
		if((size_t)ntk == tkcap) {
			tkcap = tkcap ? tkcap * 2 : 1024;
			if((ntkp = realloc(tk, tkcap * sizeof *tk)) == 0)
				return;
			tk = ntkp;
		}
		t = &tk[ntk];
		t->at = lo + i;
		t->len = (int)strlen(j ? defs[k].close : defs[k].open);
		t->def = k;
		t->open = j == 0;
		t->cls = (char)(cls[i] == HL_STRING || cls[i] == HL_COMMENT ? cls[i] : HL_TEXT);
		if(me < 0 && D->cur >= t->at && D->cur < t->at + t->len)
			me = ntk;	/* the one at the cursor */
		ntk++;
		i += t->len - 1;
	}
	if(me < 0)	/* else the one just before it */
		for(k = 0; k < ntk; k++)
			if(tk[k].at + tk[k].len == D->cur)
				me = k;
	if(me < 0)
		return;
	/* its match: the one that closes (opens) it, the others of its kind
	   between counted */
	t = &tk[me];
	{
		const char *cl = defs[t->def].close, *op = defs[t->def].open;
		char kind = defs[t->def].kind;

		d = 1;
		for(k = t->open ? me + 1 : me - 1; k >= 0 && k < ntk; k += t->open ? 1 : -1) {
			u = &tk[k];
			if(defs[u->def].kind != kind || (kind == 'o' && u->cls != t->cls))
				continue;
			if(t->open ? (!u->open && strcmp(defs[u->def].close, cl) == 0) :
			    (u->open && strcmp(defs[u->def].close, cl) == 0))
				d--;
			else if(t->open ? (u->open && strcmp(defs[u->def].close, cl) == 0) :
			    (!u->open && strcmp(defs[u->def].close, cl) == 0))
				d++;
			if(d == 0) {
				bm.a = t->at;
				bm.alen = (pos_t)t->len;
				bm.b = u->at;
				bm.blen = (pos_t)u->len;
				return;
			}
		}
		(void)op;
	}
}

static int
in_match(pos_t i)
{
	return (bm.a != POS_NONE && i >= bm.a && i < bm.a + bm.alen) ||
	    (bm.b != POS_NONE && i >= bm.b && i < bm.b + bm.blen);
}

/* Go to Matching Bracket (Ctrl+]): to the other end of the bracket or the
 * block at the cursor, as it is shown */
void
cmd_gomatch(void)
{
	if(docs == 0)
		return;
	block_match();
	if(bm.a == POS_NONE || bm.b == POS_NONE)
		return;
	jump_push();
	D->cur = bm.b;
	D->anchor = POS_NONE;
	D->pref = -1;
	cursor_visible();
}

/* Expand Selection (Shift+Alt+Right) takes the least of what holds the
 * selection: the word, the inside of the quotes, the quotes, the inside
 * of the brackets, the brackets, the line, the lines, all of the text.
 * Shrink Selection (Shift+Alt+Left) goes back the way it came. */
#define EXPANDS	64
static struct {
	struct buf *buf;
	unsigned gen;
	int n;
	pos_t anchor[EXPANDS], cur[EXPANDS];	/* the selections before */
	pos_t a, b;			/* the one made last */
} ex;

/* [a, b) holds [s1, s2) and more: the least so far, in *ba, *bb */
static void
expand_to(pos_t a, pos_t b, pos_t s1, pos_t s2, pos_t *ba, pos_t *bb)
{
	if(a <= s1 && b >= s2 && b - a > s2 - s1 && b - a < *bb - *ba) {
		*ba = a;
		*bb = b;
	}
}

static int
opener(int c)
{
	return c == '(' ? ')' : c == '[' ? ']' : c == '{' ? '}' : 0;
}

/* the bracket after o (which opens) that closes it, POS_NONE if none */
static pos_t
closer(pos_t o, pos_t hi)
{
	int c = BCH(o), e = opener(c), d = 0, ch;
	pos_t i;

	for(i = o + 1; i < hi; i++) {
		ch = BCH(i);
		if(ch == c)
			d++;
		else if(ch == e && d-- == 0)
			return i;
	}
	return POS_NONE;
}

static void
expand(pos_t *ra, pos_t *rb)
{
	const struct syn *sy = syns[D->lang < nsyns ? D->lang : 0];
	const char *quotes = sy->quotes && *sy->quotes ? sy->quotes : "\"'`";
	pos_t s1 = sel_begin(), s2 = sel_end(), len = buf_len(), ba = 0, bb = len;
	pos_t ls, le, i, q, a, b, o, c, lo, hi;
	int k, d[3], ch, cols;

	/* the word */
	for(a = s1; a > 0 && mword(BCH(a - 1)); a--)
		;
	for(b = s2; b < len && mword(BCH(b)); b++)
		;
	for(i = a; i < b && mword(BCH(i)); i++)
		;
	if(i == b && b > a)
		expand_to(a, b, s1, s2, &ba, &bb);
	/* the quotes of the line, paired from its start */
	ls = buf_linestart(s1);
	le = buf_lineend(s1);
	for(i = ls; i < le; i++) {
		ch = BCH(i);
		if(!strchr(quotes, ch) || ch == 0)
			continue;
		for(q = i + 1; q < le && BCH(q) != ch; q++)
			if(BCH(q) == '\\' && q + 1 < le)
				q++;
		if(q == le)
			break;
		expand_to(i + 1, q, s1, s2, &ba, &bb);
		expand_to(i, q + 1, s1, s2, &ba, &bb);
		i = q;
	}
	/* the brackets about it: an opener not closed before s1, and what
	   closes it after s2 */
	lo = s1 > MATCH_SPAN ? s1 - MATCH_SPAN : 0;
	hi = len - s2 > MATCH_SPAN ? s2 + MATCH_SPAN : len;
	d[0] = d[1] = d[2] = 0;
	for(o = s1; o > lo; ) {
		o--;
		ch = BCH(o);
		k = ch == '(' || ch == ')' ? 0 : ch == '[' || ch == ']' ? 1 :
		    ch == '{' || ch == '}' ? 2 : -1;
		if(k < 0)
			continue;
		if(!opener(ch)) {
			d[k]++;
			continue;
		}
		if(d[k] > 0) {
			d[k]--;
			continue;
		}
		if((c = closer(o, hi)) == POS_NONE)
			continue;
		if(c >= s2) {	/* the nearest about it: those further are more */
			expand_to(o + 1, c, s1, s2, &ba, &bb);
			expand_to(o, c + 1, s1, s2, &ba, &bb);
			break;
		}
	}
	/* the line, without its indentation; then the whole lines */
	ls = buf_linestart(s1);
	le = buf_lineend(s2 > s1 && s2 == buf_linestart(s2) ? s2 - 1 : s2);
	if(buf_linestart(s1) == buf_linestart(s2))
		expand_to(ls + indent_of(ls, 10000, &cols), le, s1, s2, &ba, &bb);
	expand_to(ls, le + (le < len), s1, s2, &ba, &bb);
	*ra = ba;
	*rb = bb;
}

void
cmd_expand(int grow)
{
	pos_t a, b;

	if(docs == 0)
		return;
	if(ex.buf != B || ex.gen != edit_gen || !has_sel() || sel_begin() != ex.a ||
	   sel_end() != ex.b)
		ex.n = 0;	/* a selection of another making: from it */
	if(!grow) {
		if(ex.n == 0) {
			D->anchor = POS_NONE;
			return;
		}
		ex.n--;
		D->anchor = ex.anchor[ex.n];
		D->cur = ex.cur[ex.n];
		ex.a = sel_begin();
		ex.b = sel_end();
	} else {
		expand(&a, &b);
		if(has_sel() && a == sel_begin() && b == sel_end())
			return;	/* all of it */
		if(ex.n == EXPANDS) {
			memmove(ex.anchor, ex.anchor + 1, (EXPANDS - 1) * sizeof ex.anchor[0]);
			memmove(ex.cur, ex.cur + 1, (EXPANDS - 1) * sizeof ex.cur[0]);
			ex.n--;
		}
		ex.anchor[ex.n] = D->anchor;
		ex.cur[ex.n++] = D->cur;
		D->anchor = a;
		D->cur = b;
		ex.a = a;
		ex.b = b;
	}
	ex.buf = B;
	ex.gen = edit_gen;
	D->pref = -1;
	cursor_visible();
}

int
hl_style(int i_cls, int cl)
{
	if(i_cls <= HL_TEXT || i_cls >= HL_COUNT)
		return cl ? S_CURLINE : S_TEXT;
	return (cl ? S_CL_KEYWORD : S_KEYWORD) + i_cls - 1;
}
int edit_lines;		/* the lines the edit added (or took, < 0) */

/* How the margin numbers the rows, this pass.  The lines of a row are
 * known from the top: in absolute numbers when the text before the top
 * is counted (or counting it takes little), else from the top (line 0).
 * Relative numbers (the distance to the line of the cursor, which shows
 * its own number) need only the newlines between the top and the cursor.
 * Absolute numbers not counted yet show as relative ones, dimmed, while
 * the editor counts. */
enum { NUM_ABS, NUM_REL, NUM_NONE };
#define NUM_BUDGET	((pos_t)4 << 20)	/* bytes counted to draw */
static struct {
	char mode, known, dim;
	pos_t top;	/* the line of the top, when known */
	long cur;	/* the line of the cursor: from the top, or absolute */
} nm;

static void
numbering(void)
{
	pos_t ls = buf_linestart(D->cur), n;
	int ok;

	nm.known = (char)buf_lineno_try(D->top, NUM_BUDGET, &nm.top);
	if(!nm.known)
		nm.top = 0;
	if(ls >= D->top) {
		ok = buf_count_try(D->top, ls, NUM_BUDGET, &n);
		nm.cur = (long)(nm.top + n);
	} else {
		ok = buf_count_try(ls, D->top, NUM_BUDGET, &n);
		nm.cur = (long)nm.top - (long)n;
	}
	nm.dim = 0;
	if(!D->linenums)
		nm.mode = NUM_NONE;
	else if(!D->relnums && nm.known)
		nm.mode = NUM_ABS;
	else {
		nm.mode = ok ? NUM_REL : NUM_NONE;
		nm.dim = !D->relnums;
	}
}

/* the number of line n (as nm counts) at the margin of row y */
static void
margin_num(int y, pos_t n, int nd)
{
	char t[24];
	int k;
	long d;

	if(nd == 0)
		return;
	if(nm.mode == NUM_NONE) {
		scr_fill(0, y, nd - 1, y, ' ', mcur ? S_CURLINE : S_MARGIN);
		return;
	}
	if(nm.mode == NUM_ABS)
		k = udec(t, n + 1);
	else if((d = (long)n - nm.cur) == 0)
		k = udec(t, nm.known ? n + 1 : 0);
	else
		k = udec(t, (pos_t)(d < 0 ? -d : d));
	if(k > nd)
		k = nd;
	scr_fill(0, y, nd - k - 1, y, ' ', mcur ? S_CURLINE : S_MARGIN);
	scr_ntext(nd - k, y, t, k, nm.dim ? S_DOTS : mcur ? S_CURNUM : S_MARGIN, nd);
}

/* the digits of the margin: the largest number it can show (the line of
 * the top and the rows under it); 0 without line numbers */
static int
margin_digits(void)
{
	pos_t m = 0, d;

	if(!D->linenums)
		return 0;
	numbering();
	if(nm.known)
		m = nm.top + (pos_t)viewh;
	if(nm.mode == NUM_REL) {
		d = (pos_t)(nm.cur - (long)nm.top < 0 ? (long)nm.top - nm.cur : nm.cur - (long)nm.top);
		if(d + (pos_t)viewh > m)
			m = d + (pos_t)viewh;
	}
	return digits(m > 99 ? m : 99);
}

static void
layout(int nd)
{
	margin = nd ? nd + 3 : 0;
	textw = scr_cols - margin - 1;
	if(textw < 2)
		textw = 2;
	viewx = margin;
}

void
draw_editor(int y, int h)
{
	pos_t rs, re, nx, i, s1, s2, line, len, a1, a2;
	int hs, blank0, gind;
	char cl;
	struct rowsig *g;
	char stale;
	int k, from, to;
	int row, col, x, w, nd, at, focus, sel, h0, rcol, mk;
	rune c;


	focus = ui_focus == ID_EDITOR;
	if(docs == 0) {
		scr_fill(0, y, scr_cols - 1, y + h - 1, ' ', S_TEXT);
		return;
	}
	ui_focusable(ID_EDITOR, W_EDIT, -1, -1, -1);
	h0 = h;
	viewh = h > 0 ? h : 1;
	viewy = y;
	layout(margin_digits());
	/* without Word Wrap, and a row wider than the window (or the view
	   scrolled sideways): the last row is the horizontal scrollbar */
	hbar = hbar_needed(h0);
	if(hbar)
		viewh = --h;
	editor_mouse();
	editor_key();
	len = buf_len();
	if(D->cur > len)
		D->cur = len;
	if(D->anchor != POS_NONE && D->anchor > len)
		D->anchor = POS_NONE;
	if(D->top > len)
		D->top = len;
	D->top = row_start(D->top);
	/* the margin as wide as the numbers shown: the top may have moved */
	if((nd = margin_digits()) != margin - 3 && (nd || margin)) {
		layout(nd);
		if(!D->wrap && (col = row_col(row_start(D->cur), D->cur)) >= D->left + textw)
			D->left = col - textw + 1;
	}
	/* the keys moved the view: the bar may come or go */
	if(hbar_needed(h0) != hbar) {
		hbar = !hbar;
		h = hbar ? h0 - 1 : h0;
		viewh = h > 0 ? h : 1;
	}
	s1 = has_sel() ? sel_begin() : POS_NONE;
	s2 = has_sel() ? sel_end() : POS_NONE;
	cur_line = cur_lstart = POS_NONE;	/* the status bar asks */
	if(!drawing)
		return;
	/* the syntax of the lines shown; the line of the cursor highlighted:
	   when the cursor leaves it, two rows change */
	hl_prepare(h);
	if(D->guides && edit_low != POS_NONE)	/* the guides of blank lines */
		rows_stale = 1;
	if(D->matchbr)
		block_match();
	else
		bm.a = bm.b = POS_NONE, bm.buf = 0;
	if(bm.a != sig_ma || bm.b != sig_mb) {	/* the match moved: drawn again */
		rows_stale = 1;
		sig_ma = bm.a;
		sig_mb = bm.b;
	}
	if((D->curline || sig_curline) && (curls != sig_curls || D->curline != sig_curline))
		rows_stale = 1;
	sig_curls = curls;
	sig_curline = D->curline;
	if((char)(hl && 1) != sig_hl || D->lang != sig_lang)
		rows_stale = 1;
	sig_hl = hl;
	sig_lang = D->lang;
	if(D->nmc || sig_nmc)	/* the other cursors: drawn again, all */
		rows_stale = 1;
	sig_nmc = D->nmc;
	rcol = settings_ruler();	/* the column marked: as the line of the cursor */
	if(rcol != sig_ruler)
		rows_stale = 1;
	sig_ruler = rcol;
	rcol = rcol > 0 ? rcol : -1;
	/* the numbers changed under the rows: relative ones when the cursor
	   leaves its line, ones counted from the top when the top moves */
	if(nd && (nm.mode != sig_mode || nm.known != sig_known || nm.dim != sig_dim ||
	   (nm.mode == NUM_REL && nm.cur != sig_cur) || (!nm.known && D->top != sig_top)))
		rows_stale = 1;
	sig_mode = nm.mode;
	sig_known = nm.known;
	sig_dim = nm.dim;
	sig_cur = nm.cur;
	if(rows_stale || y != sig_y || h != sig_h || D->left != sig_left || nd != sig_nd ||
	   B->tabsize != sig_tab || focus != sig_focus) {
		rows_stale = 1;
		sig_y = y;
		sig_h = h;
		sig_left = D->left;
		sig_nd = nd;
		sig_tab = B->tabsize;
		sig_focus = focus;
	}
	if(rows_stale)
		sig_blank = SIGMAX;
	stale = rows_stale;
	from = 0;
	to = h;
	/* only the cursor moved: the rows are as they were, the cursor is
	   looked for in them */
	if(!rows_stale && edit_low == POS_NONE && D->top == sig_top && s1 == sig_s1 &&
	   s2 == sig_s2) {
		goto found;
	}
	/* the text scrolled k rows: what is shown moves, the new rows are
	   drawn */
	if(!rows_stale && edit_low == POS_NONE && s1 == sig_s1 && s2 == sig_s2 &&
	   h <= SIGMAX && sig[0].rs != POS_NONE) {
		k = 0;
		for(row = 1; row < h && sig[row].rs != POS_NONE; row++)
			if(sig[row].rs == D->top) {
				k = row;	/* down */
				break;
			}
		if(k == 0 && D->top < sig[0].rs)
			for(row = 1, rs = D->top; row < h; row++) {
				if((rs = row_next(rs)) == POS_NONE)
					break;
				if(rs == sig[0].rs) {
					k = -row;	/* up */
					break;
				}
			}
		if(k) {
			from = k > 0 ? h - k : 0;	/* the rows to draw */
			to = k > 0 ? h : -k;
			scr_scroll(0, scr_cols - 2, y, y + h - 1, k);
			/* the scrollbar moved with the rows: redraw it so a moved
			   thumb cannot leave stale cells behind. */
			ui_scrollbar_again();
			if(k > 0)
				for(row = 0; row < h; row++)
					sig[row] = row + k < h ? sig[row + k] : sig_none;
			else
				for(row = h - 1; row >= 0; row--)
					sig[row] = row + k >= 0 ? sig[row + k] : sig_none;
			sig_blank = SIGMAX;
		}
	}
	/* a line went in or out: the text of the rows under it moves on the
	   screen as it is; the margin stays (row n shows line top + n) */
	if(!rows_stale && edit_low != POS_NONE && edit_lines && !D->wrap && D->top == sig_top &&
	   s1 == sig_s1 && s2 == sig_s2 && h <= SIGMAX) {
		for(k = 0; k < h && sig[k].rs != POS_NONE; k++)
			if(sig[k].rs <= edit_low && (sig[k].nx == POS_NONE || edit_low < sig[k].nx))
				break;
		if(k < h - 1 && sig[k].rs != POS_NONE) {
			scr_scroll(margin, scr_cols - 2, y + k + 1, y + h - 1, -edit_lines);
			ui_scrollbar_again();
			/* each row from the one edit_lines up (or down), its
			   number on with the lines; none: drawn.  A row without
			   text before (past its end) gets its number */
			for(row = edit_lines > 0 ? h - 1 : k + 1; row > k && row < h;
			    row += edit_lines > 0 ? -1 : 1) {
				g = sig + row - edit_lines;
				if(row - edit_lines <= k || row - edit_lines >= h || g->rs == POS_NONE)
					memcpy(sig + row, &sig_none, sizeof sig_none);
				else {
					i = sig[row].rs == POS_NONE;
					memcpy(sig + row, g, sizeof sig_none);
					sig[row].line += edit_lines;
					sig[row].mv = (char)i;
				}
			}
			sig_blank = SIGMAX;
		}
	}
	/* the rows that end before the edit: as they were, not looked at */
	if(!rows_stale && from == 0 && edit_low != POS_NONE && D->top == sig_top &&
	   s1 == sig_s1 && s2 == sig_s2)
		for(g = sig; from < h - 1 && from < SIGMAX - 1 && g->rs != POS_NONE &&
		    g->nx != POS_NONE && g->nx <= edit_low && !g->mv; g++)
			from++;
	sig_top = D->top;
	sig_s1 = s1;
	sig_s2 = s2;
	rs = D->top;
	line = nm.known ? nm.top : 0;	/* see numbering */
	if(from > 0) {	/* scrolled up: on from the last row that moved */
		g = sig + from - 1;
		rs = g->nx;
		line = g->line + (g->nx != POS_NONE && g->nx == g->re + 1);
	}
	/* the rows past the text that are blank on the screen: as the last
	   frame left them (one drawn blank in this one says nothing of the
	   rows under it: a scroll moved them) */
	blank0 = sig_blank;
	for(row = from, g = sig + from; row < to; row++, g++) {
		/* the row as it would be: shown already? */
		re = nx = POS_NONE;
		if(rs != POS_NONE && row < SIGMAX && !rows_stale && g->rs != POS_NONE &&
		   rs == g->rs && (edit_low == POS_NONE || (g->nx != POS_NONE && g->nx <= edit_low))) {
			re = g->re;	/* where it ends: as it was */
			nx = g->nx;
		} else if(rs != POS_NONE && row < SIGMAX && !rows_stale && g->rs != POS_NONE &&
		   edit_low != POS_NONE && rs >= edit_high && g->rs + edit_delta == rs) {
			re = g->re + edit_delta;	/* moved by the edit */
			nx = g->nx == POS_NONE ? POS_NONE : g->nx + edit_delta;
		} else if(rs != POS_NONE)
			re = row_end(rs, &nx);
		else if(!rows_stale && row >= blank0 && blank0 < SIGMAX)
			break;	/* the rows past the text: as they were */
		else if(sig_blank > row)
			sig_blank = row;
		if(s1 < s2 && rs != POS_NONE && s1 <= re && s2 > rs) {
			a1 = s1 > rs ? s1 : rs;
			a2 = s2 < re + 1 ? s2 : re + 1;
		} else
			a1 = a2 = POS_NONE;
		hs = hl_state(rs);
		cl = rs != POS_NONE && D->curline && buf_linestart(rs) == curls;
		if(row < SIGMAX) {
			/* the same row: before the edit, or past it and moved */
			if(!rows_stale && g->line == line && g->s1 == a1 && g->s2 == a2 &&
			   g->hs == hs && g->cl == cl &&
			   ((g->rs == rs && g->re == re && (rs == POS_NONE || edit_low == POS_NONE ||
			     (nx != POS_NONE && nx <= edit_low))) ||
			    (edit_low != POS_NONE && rs != POS_NONE && rs >= edit_high &&
			     g->rs + edit_delta == rs && g->re + edit_delta == re))) {
				g->rs = rs;
				g->re = re;
				g->nx = nx;
				if(g->mv) {	/* moved: its number */
					g->mv = 0;
					mcur = cl;
					margin_num(y + row, line, nd);
				}
				if(focus && rs != POS_NONE && D->cur >= rs && (nx == POS_NONE || D->cur < nx)) {
					ui_cx = margin + row_col(rs, D->cur) - D->left;
					ui_cy = y + row;
					cur_line = line;
					cur_lstart = D->wrap ? POS_NONE : rs;
					if(ui_cx >= margin + textw || ui_cx < margin)
						ui_cx = -1;
				}
				goto next;
			}
			g->rs = rs;
			g->re = re;
			g->nx = nx;
			g->line = line;
			g->s1 = a1;
			g->s2 = a2;
			g->mv = 0;
			g->hs = hs;
			g->cl = cl;
		}
		if(rs == POS_NONE) {	/* each cell once: a cell put twice is sent */
			if(nd) {
				scr_fill(0, y + row, nd, y + row, ' ', S_MARGIN);
				scr_put(nd + 1, y + row, G_V, S_MARGIN);
			}
			scr_fill(nd ? nd + 2 : 0, y + row, scr_cols - 2, y + row, ' ', S_TEXT);
			continue;
		}
		mcur = cl;
		if(nd) {
			if(rs == 0 || BCH(rs - 1) == '\n')
				margin_num(y + row, line, nd);
			else {
				scr_fill(0, y + row, nd - 2, y + row, ' ', cl ? S_CURLINE : S_MARGIN);
				scr_put(nd - 1, y + row, G_WRAPDOT, S_DOTS);
			}
			scr_put(nd, y + row, ' ', S_MARGIN);
			scr_put(nd + 1, y + row, G_V, S_MARGIN);
			scr_put(nd + 2, y + row, ' ', S_MARGIN);
		}
		/* the indent guides: at each level of the line's indentation
		   (its first row), on its blanks and past its end */
		gind = D->guides && (rs == 0 || BCH(rs - 1) == '\n') ? guide_cols(rs) : 0;
		col = 0;
		i = rs;
		if(!D->wrap && D->left > 0) {	/* from the first column shown */
			long lc;

			i = seek_col(rs, re, D->left, &lc);
			col = (int)lc;
		}
		x = margin - D->left + col;
		if(hl)
			hl_line(buf_linestart(rs), hs, i);
		for(; i < re && x < margin + textw; i = buf_next(i)) {
			c = buf_char(i);
			w = charw(i, col);
			sel = i >= s1 && i < s2;
			mk = D->nmc ? mc_at(i) : 0;
			sel |= mk == 1;
			at = sel ? (focus ? S_SELECTION : S_SELBLUR) :
			    in_match(i) ? S_SELBLUR :	/* the block's start and end */
			    hl_style(hl && i >= lx.a && i < lx.b ? lx.cls[i - lx.a] : HL_TEXT,
			    cl || (col <= rcol && col + w > rcol));
			if(c == '\t' || x < margin) {
				/* a tab shown: its first cell, those after, its last */
				int tw = w;

				for(k = 0; w > 0; w--, x++, col++, k++)
					if(x >= margin && x < margin + textw)
						scr_put(x, y + row, c == '\t' && D->showtabs ?
						    (k == tw - 1 ? G_TABEND : k == 0 ? G_TABVIEW : G_TABLINE) :
						    c == '\t' && col < gind && col % B->tabsize == 0 ? G_GUIDE : ' ',
						    c == '\t' && !D->showtabs && col < gind && col % B->tabsize == 0 &&
						    !sel ? hl_style(HL_COMMENT, cl || col == rcol) :
						    !sel && col == rcol ? S_CURLINE : at);
				continue;
			}
			if(c == ' ' && col < gind && col % B->tabsize == 0 && x >= margin &&
			   x < margin + textw) {
				scr_put(x, y + row, G_GUIDE, sel ? at : hl_style(HL_COMMENT, cl || col == rcol));
				x++;
				col++;
				continue;
			}
			if(c < ' ' || c == 127)
				c = c == 127 ? 0x2421 : 0x2400 + c;
			if(mk == 2)
				at = (at & ~0x1F) | S_TEXT | F_REV;	/* another cursor */
			if(x + w <= margin + textw)
				scr_put(x, y + row, c, at);
			x += w;
			col += w;
		}
		if(x < margin)
			x = margin;
		/* another cursor at the end of the line */
		mk = D->nmc && (nx == POS_NONE || nx == re + 1) && mc_at(re) == 2 ? F_REV : 0;
		if(x < margin + textw) {
			/* (an empty line in a block: its guide, not its mark) */
			if(D->showeol && nx == re + 1 && !(re == rs && gind > 0 && col == 0))
				scr_put(x++, y + row, G_EOLVIEW, S_DOTS | mk);
			else if(D->showeof && nx == POS_NONE && re == len)
				scr_put(x++, y + row, G_EOFVIEW, S_DOTS | mk);
			else if(mk)
				scr_put(x++, y + row, ' ', S_TEXT | F_REV);
		}
		/* past the end of the text: the selection goes on to the edge
		   when it takes the newline */
		at = re < s2 && re >= s1 && nx != re ? (focus ? S_SELECTION : S_SELBLUR) :
		    cl ? S_CURLINE : S_TEXT;
		if(x < margin + textw)
			scr_fill(x, y + row, margin + textw - 1, y + row, ' ', at);
		/* the column marked, past the end of the line */
		k = margin - D->left + rcol;
		if(rcol >= 0 && k >= x && k < margin + textw && at == S_TEXT)
			scr_put(k, y + row, ' ', S_CURLINE);
		/* a blank line in a block: its guides past its end */
		for(k = (x - margin + D->left + B->tabsize - 1) / B->tabsize * B->tabsize; k < gind;
		    k += B->tabsize)
			if(margin - D->left + k >= x && margin - D->left + k < margin + textw)
				scr_put(margin - D->left + k, y + row, G_GUIDE, hl_style(HL_COMMENT, cl || k == rcol));
		if(focus && D->cur >= rs && (nx == POS_NONE || D->cur < nx)) {
			ui_cx = margin + col_of(rs, D->cur) - D->left;
			ui_cy = y + row;
			cur_line = line;
			cur_lstart = D->wrap ? POS_NONE : rs;
			if(ui_cx >= margin + textw || ui_cx < margin)
				ui_cx = -1;
		}
next:
		if(rs == POS_NONE)
			continue;
		if(nx != POS_NONE && nx == re + 1) {
			line++;
		}
		/* past the edit, with the same lines after it: the rows that
		   follow only moved, and are as they were on the screen */
		if(!hl && !rows_stale && edit_low != POS_NONE && nx != POS_NONE && nx >= edit_high &&
		   row + 1 < to &&
		   row + 1 < SIGMAX && g[1].rs != POS_NONE && g[1].rs + edit_delta == nx &&
		   g[1].line == line) {
			/* (moved down or up by the lines of the edit: their
			   numbers) */
			for(k = row + 1, g++; k < to && k < SIGMAX && g->rs != POS_NONE; k++, g++) {
				g->rs += edit_delta;
				g->re += edit_delta;
				if(g->nx != POS_NONE)
					g->nx += edit_delta;
				if(g->mv) {
					g->mv = 0;
					margin_num(y + k, g->line, nd);
				}
			}
			if(k >= to || k >= SIGMAX)
				break;
			/* the rows past them (the end of the text came up): on
			   with them as with the others */
			row = k - 1;
			g = sig + row;
			nx = g->nx;
			line = g->line + (nx != POS_NONE && nx == g->re + 1);
		}
		rs = nx;
	}
	rows_stale = 0;
	edit_low = POS_NONE;
found:	/* the cursor in a row not drawn: looked for */
	if(focus && ui_cy < 0)
		for(row = 0, g = sig; row < h && row < SIGMAX && g->rs != POS_NONE; row++, g++)
			if(D->cur >= g->rs && (g->nx == POS_NONE || D->cur < g->nx)) {
				ui_cx = margin + row_col(g->rs, D->cur) - D->left;
				ui_cy = y + row;
				cur_line = g->line;
				cur_lstart = D->wrap ? POS_NONE : g->rs;
				if(ui_cx >= margin + textw || ui_cx < margin)
					ui_cx = -1;
				break;
			}
	/* the scrollbar, always, when it moved: by the lines (the rows shown
	   of them), or by the bytes while they are not counted (the bytes
	   shown of them) */
	bar_measure(&line, &a1, &a2);
	if(stale || line != bar_pos || a1 != bar_total || a2 != bar_shown) {
		bar_pos = line;
		bar_total = a1;
		bar_shown = a2;
		if(stale)
			ui_scrollbar_again();
		ui_scrollbar(scr_cols - 1, y, h, (long)line, (long)a1, (long)a2, 1);
	}
	/* the horizontal one: as wide as the widest row shown, at the
	   first column shown */
	if(hbar) {
		hbar_total = hbar_measure();
		if(stale || D->left != hb_left || hbar_total != hb_total || margin != hb_margin) {
			hb_left = D->left;
			hb_total = hbar_total;
			hb_margin = margin;
			if(stale)
				ui_hscrollbar_again();
			if(margin > 0)
				scr_fill(0, y + h, margin - 1, y + h, ' ', S_MARGIN);
			ui_hscrollbar(margin, y + h, scr_cols - 1 - margin, (long)D->left,
			    (long)hbar_total, (long)textw);
			scr_put(scr_cols - 1, y + h, ' ', S_TEXT);	/* the corner */
		}
	}
}
