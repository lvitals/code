/* The icon of Code Editor: </> on a dark square, as data (taken from
 * unnamed.png, 240 by 240): < > in the blue of the editor's bar, / in its
 * white.  tools/icon-svg.c writes it as an SVG
 * (data/xcedit.svg, for the menus of the system); the window draws it
 * into the pixels of its own icon. */
#ifndef ICON_H
#define ICON_H

#define ICON_SIZE	240		/* its units: the picture's pixels */
#define ICON_RADIUS	44		/* of the corners of the square */
#define ICON_BG_IN	0x1A1A1AUL	/* a radial gradient from the top left */
#define ICON_BG_OUT	0x030303UL
#define ICON_BRACKETS	0x234285UL	/* the bar: #3B78FF half over #0C0C0C */
#define ICON_SLASH	0xFFFFFFUL	/* its text */

/* < : its corners in order (> is it turned half a circle about the
 * middle: x, y become ICON_SIZE + 1 - x, ICON_SIZE - y) */
static const short icon_lt[][2] = {
	{ 101, 67 },	/* the top of its upper arm, cut straight */
	{ 101, 91 },
	{ 52, 120 },	/* inside, the point of the angle */
	{ 87, 150 },	/* the end of its lower arm, cut straight */
	{ 87, 172 },
	{ 36, 132 },	/* outside, a short straight side */
	{ 36, 108 },
};
#define ICON_LT_N	((int)(sizeof icon_lt / sizeof icon_lt[0]))

/* / : a parallelogram */
static const short icon_slash[][2] = {
	{ 120, 67 }, { 139, 67 }, { 121, 172 }, { 102, 172 },
};
#define ICON_SLASH_N	4

#endif
