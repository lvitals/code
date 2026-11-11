/* icon-svg: the icon of Code Editor (src/platform/icon.h) as an SVG, for
 * the menus of the system (make icon, make install).
 *
 *   icon-svg > data/xcedit.svg
 *
 * The square with its corners rounded and its gradient, then </> in the
 * colours of the editor's bar (< > its blue, / its white): < and / from
 * the corners of icon.h, > the < turned half a circle about the middle. */
#include <stdio.h>
#include "../src/platform/icon.h"

static void
polygon(const short (*p)[2], int n, int turned, unsigned long colour)
{
	int i, x, y;

	printf("  <polygon fill=\"#%06lX\" points=\"", colour);
	for(i = 0; i < n; i++) {
		x = turned ? ICON_SIZE + 1 - p[i][0] : p[i][0];
		y = turned ? ICON_SIZE - p[i][1] : p[i][1];
		printf("%s%d,%d", i ? " " : "", x, y);
	}
	printf("\"/>\n");
}

int
main(void)
{
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"256\" height=\"256\" "
	    "viewBox=\"0 0 %d %d\">\n"
	    "  <title>Code Editor</title>\n"
	    "  <defs>\n"
	    "    <radialGradient id=\"bg\" gradientUnits=\"userSpaceOnUse\" cx=\"0\" cy=\"0\" r=\"%d\">\n"
	    "      <stop offset=\"0\" stop-color=\"#%06lX\"/>\n"
	    "      <stop offset=\"1\" stop-color=\"#%06lX\"/>\n"
	    "    </radialGradient>\n"
	    "  </defs>\n"
	    "  <rect width=\"%d\" height=\"%d\" rx=\"%d\" fill=\"url(#bg)\"/>\n",
	    ICON_SIZE, ICON_SIZE, ICON_SIZE, ICON_BG_IN, ICON_BG_OUT,
	    ICON_SIZE, ICON_SIZE, ICON_RADIUS);
	polygon(icon_lt, ICON_LT_N, 0, ICON_BRACKETS);	/* < */
	polygon(icon_slash, ICON_SLASH_N, 0, ICON_SLASH);	/* / */
	polygon(icon_lt, ICON_LT_N, 1, ICON_BRACKETS);	/* > */
	printf("</svg>\n");
	return ferror(stdout) ? 1 : 0;
}
