#include <string.h>
#include "screen.h"
#include "../core/mem.h"

int scr_cols, scr_rows;

/* The cells change only when what is put differs, and each row keeps the
 * columns that changed since the last flush: the cells are both what is
 * drawn and what is shown, with no second copy to compare. */
static cell_t *ch;
static short *lo, *hi;		/* the columns changed in each row */
static size_t *rowoff;		/* where each row starts */
static attr_t *at;
#define ROW(y)		(ch + rowoff[y])

int
scr_init(int cols, int rows)
{
	size_t n = (size_t)cols * rows;

	int y;

	mem_free(ch);
	mem_free(lo);
	mem_free(rowoff);
	mem_free(at);
	scr_cols = cols;
	scr_rows = rows;
	lo = mem_alloc(2 * rows * sizeof(short));
	rowoff = mem_alloc(rows * sizeof(size_t));
	ch = mem_alloc(n * sizeof(cell_t));
	at = mem_alloc(n * sizeof(attr_t));
	if(!ch || !at || !lo || !rowoff)
		return 0;
	for(n = 0; n < (size_t)cols * rows; n++) {
		ch[n] = ' ';
		at[n] = S_TEXT;
	}
	hi = lo + rows;
	for(y = 0, n = 0; y < rows; y++, n += cols)
		rowoff[y] = n;
	scr_redraw();
	return 1;
}

void
scr_redraw(void)
{
	int y;

	for(y = 0; y < scr_rows; y++) {
		lo[y] = 0;
		hi[y] = (short)(scr_cols - 1);
	}
}

/* the columns x1 to x2 of row y changed */
static void
changed(int x1, int x2, int y)
{
	if(x1 < lo[y])
		lo[y] = (short)x1;
	if(x2 > hi[y])
		hi[y] = (short)x2;
}

void
scr_scroll(int x1, int x2, int y1, int y2, int k)
{
	int y, n = x2 - x1 + 1, from, to, step;

	if(k == 0 || y2 - y1 + 1 <= (k < 0 ? -k : k))
		return;
	/* row by row, columns x1 to x2 (the scrollbar past them stays) */
	from = k > 0 ? y1 : y2;
	to = k > 0 ? y2 - k + 1 : y1 - k - 1;
	step = k > 0 ? 1 : -1;
	for(y = from; y != to; y += step) {
		memmove(ROW(y) + x1, ROW(y + k) + x1, (size_t)n * sizeof(cell_t));
		memmove(at + rowoff[y] + x1, at + rowoff[y + k] + x1, (size_t)n * sizeof(attr_t));
	}
	for(y = y1; y <= y2; y++) {	/* changed(x1, x2, y), not called */
		if(lo[y] > x1)
			lo[y] = (short)x1;
		if(hi[y] < x2)
			hi[y] = (short)x2;
	}
}

static void
set(size_t i, int x, int y, cell_t c, int a)
{
	if(ch[i] == c && at[i] == a)
		return;
	ch[i] = c;
	at[i] = (attr_t)a;
	changed(x, x, y);
}

int
cwidth(rune c)
{
	if(c < 0x300)
		return 1;
	if((c >= 0x300 && c < 0x370) || (c >= 0x200B && c <= 0x200F) ||
	   (c >= 0xFE00 && c <= 0xFE0F) || (c >= 0x1AB0 && c < 0x1B00) ||
	   (c >= 0x20D0 && c < 0x2100))
		return 0;	/* the combining marks */
	if((c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0xA4CF && c != 0x303F) ||
	   (c >= 0xAC00 && c <= 0xD7A3) || (c >= 0xF900 && c <= 0xFAFF) ||
	   (c >= 0xFE30 && c <= 0xFE4F) || (c >= 0xFF00 && c <= 0xFF60) ||
	   (c >= 0xFFE0 && c <= 0xFFE6) || (c >= 0x1F300 && c <= 0x1F64F) ||
	   (c >= 0x1F900 && c <= 0x1F9FF) || (c >= 0x20000 && c <= 0x3FFFD))
		return 2;
	return 1;
}

void
scr_put(int x, int y, rune c, int a)
{
	rune v = c;

	if((unsigned)x >= (unsigned)scr_cols || (unsigned)y >= (unsigned)scr_rows)
		return;
	if(v < ' ' && v >= G_LAST)
		v = G_CONTROL;
	size_t i = rowoff[y] + x;

	if(ch[i] == CELL_WIDE && x > 0 && v != CELL_WIDE)
		set(i - 1, x - 1, y, ' ', at[i - 1]);	/* half of a wide one: it goes */
	if(x + 1 < scr_cols && ch[i + 1] == CELL_WIDE)
		set(i + 1, x + 1, y, ' ', at[i + 1]);
	if(cwidth(v) == 2) {
		if(x + 1 >= scr_cols)
			v = ' ';
		else {
			if(x + 2 < scr_cols && ch[i + 2] == CELL_WIDE)
				set(i + 2, x + 2, y, ' ', at[i + 2]);
			set(i + 1, x + 1, y, CELL_WIDE, a);
		}
	}
	set(i, x, y, (cell_t)v, a);
}

/* the next character of s, and its bytes */
static rune
decode(const char *s, int n, int *len)
{
	const unsigned char *p = (const unsigned char *)s;
	int k, i;
	rune c;

	if(p[0] < 0x80) {
		*len = 1;
		return p[0];
	}
	if(p[0] >= 0xF0 && p[0] < 0xF5) {
		k = 3;
		c = p[0] & 7;
	} else if(p[0] >= 0xE0 && p[0] < 0xF0) {
		k = 2;
		c = p[0] & 15;
	} else if(p[0] >= 0xC2 && p[0] < 0xE0) {
		k = 1;
		c = p[0] & 31;
	} else {
		*len = 1;
		return 0xFFFD;
	}
	for(i = 1; i <= k; i++) {
		if(i >= n || (p[i] & 0xC0) != 0x80) {
			*len = 1;
			return 0xFFFD;
		}
		c = c << 6 | (p[i] & 0x3F);
	}
	*len = k + 1;
	return c;
}

int
scr_ntext(int x, int y, const char *s, int n, int a, int lim)
{
	int i = 0, len, w, cx = x;
	rune c;

	while(i < n) {
		c = decode(s + i, n - i, &len);
		w = cwidth(c);
		if(cx + w > lim)
			break;
		if(w)
			scr_put(cx, y, c, a);
		cx += w;
		i += len;
	}
	return cx;
}

int
scr_text(int x, int y, const char *s, int a, int lim)
{
	return scr_ntext(x, y, s, (int)strlen(s), a, lim);
}

int
text_width(const char *s)
{
	int n = (int)strlen(s), i = 0, len, w = 0;

	while(i < n) {
		w += cwidth(decode(s + i, n - i, &len));
		i += len;
	}
	return w;
}

void
scr_fill(int x1, int y1, int x2, int y2, rune c, int a)
{
	int x, y, any, xa = x1 < 0 ? 0 : x1, xb = x2 >= scr_cols ? scr_cols - 1 : x2;
	cell_t v = c < ' ' && c >= G_LAST ? G_CONTROL : (cell_t)c, *q;
	attr_t *r;

	if(cwidth(c) != 1) {
		for(y = y1; y <= y2; y++)
			for(x = x1; x <= x2; x++)
				scr_put(x, y, c, a);
		return;
	}
	for(y = y1 < 0 ? 0 : y1; y <= y2 && y < scr_rows; y++) {
		q = ch + rowoff[y] + xa;
		r = at + rowoff[y] + xa;
		for(x = xa, any = 0; x <= xb; x++, q++, r++)
			if(*q != v || *r != a) {
				if(*q == CELL_WIDE && x > 0 && x == xa)
					q[-1] = ' ', changed(x - 1, x - 1, y);
				*q = v;
				*r = (attr_t)a;
				any = 1;
			}
		if(xb + 1 < scr_cols && *q == CELL_WIDE)
			*q = ' ', any = 1, xb++;
		if(any)
			changed(xa, xb, y);
		xb = x2 >= scr_cols ? scr_cols - 1 : x2;
	}
}

void
scr_frame(int x1, int y1, int x2, int y2, int a)
{
	int x, y;

	for(x = x1 + 1; x < x2; x++) {
		scr_put(x, y1, G_H, a);
		scr_put(x, y2, G_H, a);
	}
	for(y = y1 + 1; y < y2; y++) {
		scr_put(x1, y, G_V, a);
		scr_put(x2, y, G_V, a);
	}
	scr_put(x1, y1, G_TL, a);
	scr_put(x2, y1, G_TR, a);
	scr_put(x1, y2, G_BL, a);
	scr_put(x2, y2, G_BR, a);
}

void
scr_attr(int x1, int y1, int x2, int y2, int keep, int add)
{
	int x, y;
	size_t i;

	for(y = y1 < 0 ? 0 : y1; y <= y2 && y < scr_rows; y++)
		for(x = x1 < 0 ? 0 : x1; x <= x2 && x < scr_cols; x++) {
			i = rowoff[y] + x;
			set(i, x, y, ch[i], (at[i] & keep) | add);
		}
}

static int lowx1, lowx2 = -1, lowy = -1;

void
scr_lower(int x1, int x2, int y)
{
	lowx1 = x1;
	lowx2 = x2;
	lowy = y;
}

void
scr_flush(void)
{
	int y, x;
	size_t i;

	for(y = 0; y < scr_rows; y++) {
		if(lo[y] > hi[y])
			continue;
		x = lo[y];
		i = rowoff[y];
		if(ch[i + x] == CELL_WIDE && x > 0)
			x--;	/* from the character itself */
		term_write(x, y, ch + i + x, at + i + x, hi[y] - x + 1);
		lo[y] = (short)scr_cols;
		hi[y] = -1;
	}
	term_lower(lowx1, lowx2, lowy);
	term_flush();
}
