/* The menu bar: File, Edit, View, Help; Alt and the
 * letter of a menu, or F10, opens it.  The menus themselves are menus.c. */
#include <string.h>
#include "app.h"

static const struct {
	const char *title;
	char accel, doc;
} menus[] = {
	{ "File", 'F', 0 },
	{ "Edit", 'E', 1 },
	{ "View", 'V', 1 },
	{ "Help", 'H', 0 },
};

int menu_x[NMENUS];	/* where each title is */

int
menu_shown(int m)
{
	if(m >= NMENUS)
		return docs != 0;	/* the context menu */
	return !menus[m].doc || docs != 0;
}

void
open_menu(int m)
{
	st.menu = m;
	st.mitem = 0;
	st.mtop = 0;
	st.mdrag = 0;
	ui_steal(ID_MENU);
	ui_again = 1;
}

void
close_menu(void)
{
	st.menu = -1;
	if(ui_focus == ID_MENU)
		ui_focus = ID_NONE;
	ui_again = 1;
}

/* Alt and the letter of a menu, F10: it opens */
int
menu_shortcut(void)
{
	kcode k = ui_key;
	int m;

	if(k == K_F10) {
		ui_key = 0;
		if(st.menu >= 0)
			close_menu();
		else
			open_menu(0);
		return 1;
	}
	if(!(k & K_ALT) || (k & (K_CTRL | K_SHIFT)))
		return 0;
	for(m = 0; m < NMENUS; m++)
		if(menu_shown(m) && K_CODE(k) == menus[m].accel) {
			ui_key = 0;
			open_menu(m);
			return 1;
		}
	return 0;
}

void
draw_menubar(void)
{
	int m, x = 0, a, i;

	if(st.menu >= 0 && (ui_focus != ID_MENU || !menu_shown(st.menu)))
		st.menu = -1;
	for(m = 0; m < NMENUS; m++) {
		if(!menu_shown(m))
			continue;
		menu_x[m] = x;
		a = st.menu == m ? S_FOCUS : S_BAR;
		scr_put(x, 0, ' ', a);
		x = ui_accel(x + 1, 0, menus[m].title, menus[m].accel, a, scr_cols);
		scr_put(x++, 0, ' ', a);
		if(ui_mouse(menu_x[m], 0, x - 1)) {
			ui_key = 0;
			if(st.menu == m)
				close_menu();
			else
				open_menu(m);
		} else if(st.menu >= 0 && st.menu < NMENUS && st.menu != m && ui_key == K_MOUSE &&
		    (term_mouse.kind == M_MOVE || term_mouse.kind == M_DRAG) && term_mouse.y == 0 &&
		    term_mouse.x >= menu_x[m] && term_mouse.x < x) {
			/* a menu open: the one under the mouse instead */
			ui_key = 0;
			i = term_mouse.kind == M_DRAG;
			open_menu(m);
			st.mdrag = (char)i;
		}
	}
	scr_fill(x, 0, scr_cols - 1, 0, ' ', S_BAR);	/* after: a cell once */
}

