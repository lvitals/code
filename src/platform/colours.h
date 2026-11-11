/* The colours of the styles: the 16 of a palette, the foreground and the
 * background, and what each style (S_*) is made of them.  The terminal
 * asks its own palette; the window has the default one. */
#ifndef COLOURS_H
#define COLOURS_H

#include "platform.h"

enum { BLACK = 0, RED = 1, GREEN = 2, YELLOW = 3, BLUE = 4, MAGENTA = 5, CYAN = 6, WHITE = 7,
	BRIGHTBLUE = 12, BRIGHTMAGENTA = 13, BRIGHTCYAN = 14, BRIGHTWHITE = 15, FG = 16, BG = 17 };

extern unsigned long pal[18];			/* 0xRRGGBB */
extern unsigned long sfg[S_LAST], sbg[S_LAST];	/* of each style */

void colours_styles(void);			/* sfg, sbg from pal */
/* a over b, a taking num/den of it */
unsigned long colours_blend(unsigned long a, unsigned long b, int num, int den);

#endif
