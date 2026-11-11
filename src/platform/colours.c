/* The colours of the styles (colours.h). */
#include "colours.h"

/* the 16 colours, the foreground and the background (Campbell, until the
 * terminal says) */
unsigned long pal[18] = {
	0x0C0C0C, 0xC50F1F, 0x13A10E, 0xC19C00, 0x0037DA, 0x881798, 0x3A96DD, 0xCCCCCC,
	0x767676, 0xE74856, 0x16C60C, 0xF9F1A5, 0x3B78FF, 0xB4009E, 0x61D6D6, 0xF2F2F2,
	0xCCCCCC, 0x0C0C0C
};
unsigned long sfg[S_LAST], sbg[S_LAST];

unsigned long
colours_blend(unsigned long a, unsigned long b, int num, int den)
{
	unsigned long r = 0;
	int sh, x, y;

	for(sh = 0; sh < 24; sh += 8) {
		x = (int)(a >> sh & 0xFF);
		y = (int)(b >> sh & 0xFF);
		r |= (unsigned long)((x * num + y * (den - num)) / den) << sh;
	}
	return r;
}

static unsigned long
contrast(unsigned long c)
{
	long l = (long)(c >> 16 & 0xFF) * 299 + (long)(c >> 8 & 0xFF) * 587 + (long)(c & 0xFF) * 114;

	return l > 128000 ? pal[BLACK] : pal[BRIGHTWHITE];
}

void
colours_styles(void)
{
	unsigned long fg = pal[FG], bg = pal[BG];
	int i;

	for(i = 0; i < S_LAST; i++) {
		sfg[i] = fg;
		sbg[i] = bg;
	}
	sfg[S_DOTS] = colours_blend(fg, bg, 1, 2);
	sbg[S_SELECTION] = colours_blend(pal[BRIGHTBLUE], fg, 1, 2);
	sfg[S_SELECTION] = contrast(sbg[S_SELECTION]);
	sbg[S_SELBLUR] = colours_blend(sbg[S_SELECTION], bg, 1, 2);
	sfg[S_SELBLUR] = contrast(sbg[S_SELBLUR]);
	sbg[S_BAR] = colours_blend(pal[BRIGHTBLUE], bg, 1, 2);
	sfg[S_BAR] = contrast(sbg[S_BAR]);
	sbg[S_FLOAT] = colours_blend(fg, bg, 1, 3);
	sfg[S_FLOAT] = contrast(sbg[S_FLOAT]);
	sbg[S_FOCUS] = pal[GREEN];
	sfg[S_FOCUS] = contrast(pal[GREEN]);
	sbg[S_PANEL] = pal[WHITE];
	sfg[S_PANEL] = pal[BLACK];
	sbg[S_FAIL] = sbg[S_ALERT] = pal[RED];
	sfg[S_FAIL] = sfg[S_ALERT] = pal[BRIGHTWHITE];
	sbg[S_LIST] = colours_blend(pal[BLACK], sbg[S_FLOAT], 1, 4);
	sfg[S_LIST] = sfg[S_FLOAT];
	sbg[S_TRACK] = sbg[S_THUMB] = colours_blend(fg, bg, 1, 8);
	sfg[S_THUMB] = colours_blend(fg, bg, 2, 3);
	/* the line of the cursor: a little lighter (or darker) than the text */
	sbg[S_CURLINE] = sbg[S_CURNUM] = colours_blend(fg, bg, 1, 8);
	sfg[S_CURNUM] = pal[BRIGHTWHITE] == bg ? pal[BLACK] : fg;
	/* the syntax, in the colours of the terminal */
	sfg[S_KEYWORD] = pal[BRIGHTBLUE];
	sfg[S_TYPE] = pal[CYAN];
	sfg[S_STRING] = pal[GREEN];
	sfg[S_COMMENT] = colours_blend(fg, bg, 1, 2);
	sfg[S_NUMBER] = pal[YELLOW];
	sfg[S_PREPROC] = pal[MAGENTA];
	sfg[S_DELETED] = pal[RED];
	/* the horizontal bar: its track and thumb a band across the middle
	   of the cells, on the background of the text, level with ◀ ▶ */
	sfg[S_HTRACK] = sbg[S_TRACK];
	sbg[S_HTRACK] = bg;
	sfg[S_HTHUMB] = sfg[S_THUMB];
	sbg[S_HTHUMB] = bg;
	for(i = 0; i <= S_DELETED - S_KEYWORD; i++) {
		sfg[S_CL_KEYWORD + i] = sfg[S_KEYWORD + i];
		sbg[S_CL_KEYWORD + i] = sbg[S_CURLINE];
	}
}

/* rgb:RRRR/GGGG/BBBB, the answer to a colour asked */
