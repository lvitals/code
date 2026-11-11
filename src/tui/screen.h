/* screen: the cells the application draws, and what the terminal shows;
 * scr_flush sends only the cells that changed (a typed key usually
 * changes one line). */
#ifndef SCREEN_H
#define SCREEN_H

#include "../platform/platform.h"

extern int scr_cols, scr_rows;

int scr_init(int cols, int rows);	/* 0 without memory */
int cwidth(rune c);			/* the columns of a character */
void scr_put(int x, int y, rune c, int at);
/* the text of s (UTF-8) from x, up to column lim (not
 * included): the column after it */
int scr_text(int x, int y, const char *s, int at, int lim);
int scr_ntext(int x, int y, const char *s, int n, int at, int lim);
int text_width(const char *s);		/* in columns */
void scr_fill(int x1, int y1, int x2, int y2, rune c, int at);
void scr_frame(int x1, int y1, int x2, int y2, int at);
/* the looks of the cells: (at & keep) | set */
void scr_attr(int x1, int y1, int x2, int y2, int keep, int set);
void scr_flush(void);
/* columns x1 to x2 of rows y1 to y2 moved k up (k < 0: down): the rows
 * left behind keep what they had, to be drawn again */
void scr_scroll(int x1, int x2, int y1, int y2, int k);
void scr_redraw(void);		/* all of it again, at the next flush */
/* cells x1 to x2 of row y drawn a little lower, in a window (see
 * term_lower): asked again at each pass; y < 0: none */
void scr_lower(int x1, int x2, int y);

#endif
