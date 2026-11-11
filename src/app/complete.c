/* Complete Word (Ctrl+Space): the words of the document that start as
 * the one before the cursor does, the nearest first.  One is put in at
 * once; more are offered in a list at the cursor, which typing narrows:
 *
 *   Up, Down, PgUp, PgDn	choose
 *   Enter, Tab, a click	take the word chosen
 *   Esc, a click elsewhere	close the list
 *
 * Only the text around the cursor is read (SPAN bytes on each side), so a
 * huge file costs no more than a small one. */
#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "../core/mem.h"
#include "syntax.h"

#define SPAN	((pos_t)1 << 20)	/* the bytes read on each side */
#define WMAX	64		/* the longest word offered */
#define UMAX	2048		/* the words told apart */
#define HSLOTS	4096		/* their hash: twice as many slots */
#define SHOWN	100		/* the words offered: the nearest */
#define ROWS	8		/* the rows of the list */

struct word {
	int off, len;		/* in pool */
	pos_t dist;		/* from the cursor: of the nearest one */
};

static char pool[UMAX * WMAX];
static int npool;
static struct word words[UMAX];
static short slot[HSLOTS];	/* 1 + an index in words; 0: none */
static int nwords;

static struct {
	char on;
	struct buf *buf;
	pos_t start;		/* where the word typed starts */
	char prefix[WMAX];
	int plen;
	int sel, top;
	int x, y, w, h;		/* the list, as drawn */
} cp;

static char dash;	/* the language has - in its names (COBOL, Lisp) */

static int
wordch(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
	    c == '_' || c >= 0x80 || (dash && c == '-');
}

static int
lower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

static int
same(const char *a, const char *b, int n, int nocase)
{
	int i;

	if(!nocase)
		return memcmp(a, b, (size_t)n) == 0;
	for(i = 0; i < n; i++)
		if(lower((unsigned char)a[i]) != lower((unsigned char)b[i]))
			return 0;
	return 1;
}

static unsigned
hash(const char *s, int n)
{
	unsigned h = 2166136261u;

	while(n-- > 0)
		h = (h ^ (unsigned char)*s++) * 16777619u;
	return h;
}

/* the word s found at a distance d: kept, once (at its nearest) */
static void
add(const char *s, int n, pos_t d)
{
	unsigned h = hash(s, n) & (HSLOTS - 1);
	struct word *w;

	for(; slot[h]; h = (h + 1) & (HSLOTS - 1)) {
		w = &words[slot[h] - 1];
		if(w->len == n && memcmp(pool + w->off, s, (size_t)n) == 0) {
			if(d < w->dist)
				w->dist = d;
			return;
		}
	}
	if(nwords == UMAX)
		return;
	w = &words[nwords];
	w->off = npool;
	w->len = n;
	w->dist = d;
	memcpy(pool + npool, s, (size_t)n);
	npool += n;
	slot[h] = (short)++nwords;
}

static int
nearer(const void *a, const void *b)
{
	const struct word *x = a, *y = b;

	return x->dist < y->dist ? -1 : x->dist > y->dist;
}

/* the words of the text around the cursor that the prefix starts, but
 * not the one typed: the nearest first (in any case if none is in its
 * own) */
static void
gather(void)
{
	pos_t len = buf_len(), lo, hi, n, i, ws, at;
	char *t;
	int nocase, wl;

	nwords = npool = 0;
	memset(slot, 0, sizeof slot);
	lo = cp.start > SPAN ? cp.start - SPAN : 0;
	hi = len - D->cur > SPAN ? D->cur + SPAN : len;
	n = hi - lo;
	if(n == 0 || (t = mem_alloc(n)) == 0)
		return;
	buf_copy(lo, n, t);
	for(nocase = 0; nocase < 2 && nwords == 0; nocase++)
		for(i = 0; i < n; ) {
			if(!wordch((unsigned char)t[i])) {
				i++;
				continue;
			}
			for(ws = i; i < n && wordch((unsigned char)t[i]); i++)
				;
			wl = (int)(i - ws);
			at = lo + ws;
			if(wl > cp.plen && wl <= WMAX && at != cp.start &&
			   same(t + ws, cp.prefix, cp.plen, nocase) &&
			   !(cp.plen == 0 || (t[ws] >= '0' && t[ws] <= '9' &&
			     !(cp.prefix[0] >= '0' && cp.prefix[0] <= '9'))))
				add(t + ws, wl, at < D->cur ? D->cur - at : at - D->cur);
		}
	mem_free(t);
	qsort(words, (size_t)nwords, sizeof words[0], nearer);
	if(nwords > SHOWN)
		nwords = SHOWN;
}

/* the word typed before the cursor: its start, and the prefix; 0 if none */
static int
typed(pos_t *start, char *prefix, int *plen)
{
	pos_t s = D->cur;

	if(has_sel())
		return 0;
	dash = D->lang < nsyns && (syns[D->lang]->flags & F_DASH) != 0;
	while(s > 0 && D->cur - s < WMAX && wordch(BCH(s - 1)))
		s--;
	if(s == D->cur || D->cur - s >= WMAX || (s > 0 && wordch(BCH(s - 1))))
		return 0;
	*start = s;
	*plen = (int)(D->cur - s);
	buf_copy(s, (pos_t)*plen, prefix);
	prefix[*plen] = 0;
	return 1;
}

static void
close_list(void)
{
	if(cp.on) {
		cp.on = 0;
		rows_stale = 1;	/* the text under it */
		ui_again = 1;
	}
}

/* the word chosen in place of the one typed */
static void
take(int i)
{
	struct word *w = &words[i];

	close_list();
	buf_begin();
	if(edit(cp.start, D->cur - cp.start, pool + w->off, (pos_t)w->len, U_OTHER)) {
		D->cur = cp.start + (pos_t)w->len;
		D->anchor = POS_NONE;
		B->merge = 0;
	}
	cursor_visible();
}

void
complete_start(void)
{
	pos_t start;
	char prefix[WMAX];
	int plen;

	if(docs == 0)
		return;
	if(!typed(&start, prefix, &plen)) {
		close_list();
		return;
	}
	cp.buf = B;
	cp.start = start;
	memcpy(cp.prefix, prefix, (size_t)plen + 1);
	cp.plen = plen;
	gather();
	cp.sel = cp.top = 0;
	if(nwords == 1)
		take(0);
	else if(nwords > 1) {
		cp.on = 1;
		ui_again = 1;
	}
}

int
complete_open(void)
{
	return cp.on;
}

/* after a key: the word typed still there (else the list closes); the
 * list narrowed (or widened) to it */
static void
follow(void)
{
	pos_t start;
	char prefix[WMAX];
	int plen;

	if(!cp.on)
		return;
	if(docs == 0 || B != cp.buf || ui_focus != ID_EDITOR || !typed(&start, prefix, &plen) ||
	   start != cp.start) {
		close_list();
		return;
	}
	if(plen == cp.plen && memcmp(prefix, cp.prefix, (size_t)plen) == 0)
		return;
	memcpy(cp.prefix, prefix, (size_t)plen + 1);
	cp.plen = plen;
	gather();
	cp.sel = cp.top = 0;
	if(nwords == 0)
		close_list();
}

void
complete_key(void)
{
	kcode k = ui_key;
	int mx, my;

	follow();
	if(!cp.on || k == 0)
		return;
	if(k == K_MOUSE) {
		mx = term_mouse.x;
		my = term_mouse.y;
		if(mx >= cp.x && mx < cp.x + cp.w && my >= cp.y && my < cp.y + cp.h) {
			if(term_mouse.kind == M_WHEELUP || term_mouse.kind == M_WHEELDOWN) {
				cp.sel += term_mouse.kind == M_WHEELUP ? -1 : 1;
				if(cp.sel < 0)
					cp.sel = 0;
				if(cp.sel >= nwords)
					cp.sel = nwords - 1;
			} else if(term_mouse.kind == M_PRESS && term_mouse.button == MB_LEFT &&
			    cp.top + my - cp.y < nwords)
				take(cp.top + my - cp.y);
			ui_key = 0;
			ui_again = 1;
		} else if(term_mouse.kind == M_PRESS)
			close_list();	/* and the click goes on to what is there */
		return;
	}
	if(ui_focus != ID_EDITOR)
		return;
	switch(k) {
	case K_UP:
		cp.sel = cp.sel ? cp.sel - 1 : nwords - 1;
		break;
	case K_DOWN:
	case K_CTRL | ' ':
		cp.sel = cp.sel + 1 < nwords ? cp.sel + 1 : 0;
		break;
	case K_PGUP:
		cp.sel = cp.sel > ROWS - 1 ? cp.sel - (ROWS - 1) : 0;
		break;
	case K_PGDN:
		cp.sel = cp.sel + ROWS - 1 < nwords ? cp.sel + ROWS - 1 : nwords - 1;
		break;
	case 13:
	case 9:
		take(cp.sel);
		break;
	case 27:
		close_list();
		break;
	default:
		return;	/* for the editor: typing narrows the list */
	}
	ui_key = 0;
	ui_again = 1;
}

static const char *
item(int i)
{
	static char s[WMAX + 1];

	memcpy(s, pool + words[i].off, (size_t)words[i].len);
	s[words[i].len] = 0;
	return s;
}

/* the list, under the word typed (over it, if there is no room) */
void
draw_complete(void)
{
	int i, w = 0, n, x, y, at;

	follow();
	if(!cp.on || ui_cx < 0 || ui_cy < 0)
		return;
	for(i = 0; i < nwords; i++)
		if((n = text_width(item(i))) > w)
			w = n;
	w += 3;		/* a space on each side, the scrollbar */
	if(w < 12)
		w = 12;
	if(w > scr_cols)
		w = scr_cols;
	cp.h = nwords < ROWS ? nwords : ROWS;
	x = ui_cx - text_width(cp.prefix) - 1;
	if(x + w > scr_cols)
		x = scr_cols - w;
	if(x < 0)
		x = 0;
	y = ui_cy + 1;
	if(y + cp.h > scr_rows - 1)
		y = ui_cy - cp.h;
	if(y < 1)
		y = 1;
	if(cp.sel < cp.top)
		cp.top = cp.sel;
	if(cp.sel >= cp.top + cp.h)
		cp.top = cp.sel - cp.h + 1;
	cp.x = x;
	cp.y = y;
	cp.w = w;
	for(i = 0; i < cp.h; i++) {
		at = cp.top + i == cp.sel ? S_FOCUS : S_FLOAT;
		scr_fill(x, y + i, x + w - 1, y + i, ' ', at);
		ui_trunc(x + 1, y + i, item(cp.top + i), at, x + w - 1);
	}
	ui_scrollbar_again();
	if(nwords > cp.h)
		ui_scrollbar(x + w - 1, y, cp.h, cp.top, nwords, cp.h, 0);
	ui_scrollbar_again();	/* the editor's: drawn again over the rows */
}
