/* The menus of the bar, each item with its accelerator and its
 * shortcut: drawn while one is open. */
#include <string.h>
#include "app.h"

struct item {
	const char *text;
	char accel, doc, action;	/* doc: only with a document */
	const char *keys;	/* its shortcut, as shown */
};

#define C(k)	KEY_CTRL "+" k

/* a line between groups of items of one kind */
#define SEP	{ "-", 0, 0, A_NONE, "" }
#define ISSEP(it)	((it)->text[0] == '-' && (it)->text[1] == 0)

/* the documents: made and opened, kept, closed; the settings; the end */
static const struct item file_items[] = {
	{ "New File", 'N', 0, A_NEW, C("N") },
	{ "Open File" DOTS, 'O', 0, A_OPEN, C("O") },
	{ "Open Project Folder" DOTS, 'F', 0, A_OPENFOLDER, KEY_ALT "+O" },
	{ "Open File in Project" DOTS, 'I', 0, A_FINDFILE, KEY_ALT "+P" },
	SEP,
	{ "Save", 'S', 1, A_SAVE, C("S") },
	{ "Save As" DOTS, 'A', 1, A_SAVEAS, "" },
	{ "Encoding" DOTS, 'E', 1, A_ENCODING, "" },
	SEP,
	{ "Close File", 'C', 1, A_CLOSE, C("W") },
	SEP,
	{ "Preferences", 'P', 0, A_PREFS, "" },
	SEP,
	{ "Exit", 'X', 0, A_EXIT, C("Q") },
	{ 0, 0, 0, 0, 0 }
};

/* the history; the clipboard and the selection; find; delete; the mark;
 * how typing indents */
static const struct item edit_items[] = {
	{ "Undo", 'U', 1, A_UNDO, C("Z") },
	{ "Redo", 'R', 1, A_REDO, C("Y") },
	SEP,
	{ "Cut", 'T', 1, A_CUT, C("X") },
	{ "Copy", 'C', 1, A_COPY, C("C") },
	{ "Paste", 'P', 1, A_PASTE, C("V") },
	{ "Select All", 'A', 1, A_SELALL, C("A") },
	{ "Expand Selection", 'X', 1, A_EXPAND, "Shift+" KEY_ALT "+Right" },
	{ "Shrink Selection", 'H', 1, A_SHRINK, "Shift+" KEY_ALT "+Left" },
	SEP,
	{ "Add Next Occurrence", 'N', 1, A_MCNEXT, C("D") },
	{ "Add Cursor Above", 'V', 1, A_MCUP, "Shift+" KEY_ALT "+Up" },
	{ "Add Cursor Below", 'B', 1, A_MCDOWN, "Shift+" KEY_ALT "+Down" },
	SEP,
	{ "Complete Word", 'W', 1, A_COMPLETE, C("Space") },
	{ "Filter Through Command" DOTS, 'O', 1, A_FILTER, KEY_ALT "+|" },
	SEP,
	{ "Find", 'F', 1, A_FIND, C("F") },
	{ "Replace", 'L', 1, A_REPLACE, C("R") },
	{ "Search in Project" DOTS, 'G', 0, A_FINDTEXT, KEY_ALT "+/" },
	SEP,
	{ "Delete to Line Start", 'S', 1, A_DELHEAD, C("Shift+H") },
	{ "Delete to Line End", 'E', 1, A_DELTAIL, C("Shift+G") },
	SEP,
	{ "Set Mark", 'M', 1, A_MARK, KEY_ALT "+M" },
	{ "Jump to Mark", 'J', 1, A_JUMPMARK, KEY_ALT "+J" },
	SEP,
	{ "Auto Indent", 'I', 1, A_AUTOINDENT, "" },
	{ "Backspace Removes a Whole Indent", 'K', 1, A_BSINDENT, "" },
	{ 0, 0, 0, 0, 0 }
};

/* where to go; how the text is laid out; the marks of what is not seen;
 * the numbers; the colours; the size */
static const struct item view_items[] = {
	{ "Go to File" DOTS, 'F', 1, A_GOTOFILE, C("P") },
	{ "Go to Line:Column" DOTS, 'G', 1, A_GOTO, C("G") },
	{ "Go to Matching Bracket", 'M', 1, A_GOMATCH, C("]") },
	{ "Go Back", 'K', 1, A_BACK, KEY_ALT "+Left" },
	{ "Go Forward", 'A', 1, A_FORWARD, KEY_ALT "+Right" },
	{ "Focus Statusbar", 'S', 1, A_STATUS, "" },
	SEP,
	{ "Word Wrap", 'W', 1, A_WRAP, KEY_ALT "+Z" },
	SEP,
	{ "Show Tabs", 'T', 1, A_SHOWTABS, "" },
	{ "Show Indent Guides", 'U', 1, A_GUIDES, "" },
	{ "Show Line Ends", 'E', 1, A_SHOWEOL, "" },
	{ "Show End of File", 'D', 1, A_SHOWEOF, "" },
	SEP,
	{ "Line Numbers", 'N', 1, A_LINENUMS, KEY_ALT "+N" },
	{ "Relative Line Numbers", 'R', 1, A_RELNUMS, KEY_ALT "+R" },
	SEP,
	{ "Syntax Highlighting", 'Y', 1, A_SYNTAX, "" },
	{ "Highlight Current Line", 'C', 1, A_CURLINE, "" },
	{ "Highlight Matching Brackets", 'B', 1, A_MATCHBR, "" },
	SEP,
	{ "Zoom In", 'I', 0, A_ZOOMIN, C("+") },
	{ "Zoom Out", 'O', 0, A_ZOOMOUT, C("-") },
	{ "Reset Zoom", 'Z', 0, A_ZOOMRESET, C("0") },
	{ 0, 0, 0, 0, 0 }
};

static const struct item help_items[] = {
	{ "About", 'A', 0, A_ABOUT, "" },
	{ 0, 0, 0, 0, 0 }
};

/* the context menu, at the mouse */
static const struct item ctx_items[] = {
	{ "Cut", 'T', 1, A_CUT, C("X") },
	{ "Copy", 'C', 1, A_COPY, C("C") },
	{ "Paste", 'P', 1, A_PASTE, C("V") },
	{ "Select All", 'A', 1, A_SELALL, C("A") },
	SEP,
	{ "Find" DOTS, 'F', 1, A_FIND, C("F") },
	{ 0, 0, 0, 0, 0 }
};

static const struct item *const items_of[NMENUS + 1] = {
	file_items, edit_items, view_items, help_items, ctx_items
};

/* an item that is on or off: on */
int
menu_checked(int a)
{
	struct doc *d = docs;

	if(d == 0)
		return 0;
	switch(a) {
	case A_WRAP: return d->wrap;
	case A_AUTOINDENT: return d->autoindent;
	case A_SHOWTABS: return d->showtabs;
	case A_SHOWEOL: return d->showeol;
	case A_SHOWEOF: return d->showeof;
	case A_LINENUMS: return d->linenums;
	case A_RELNUMS: return d->relnums;
	case A_CURLINE: return d->curline;
	case A_SYNTAX: return d->syntax;
	case A_MATCHBR: return d->matchbr;
	case A_BSINDENT: return d->bsindent;
	case A_GUIDES: return d->guides;
	}
	return 0;
}

/* the item of the open menu under the mouse, -1 if none (v rows of
 * items shown, from top) */
static int
hovered(int x, int y, int x2, int v, int top)
{
	struct mouse *ms = &term_mouse;

	if(ui_key != K_MOUSE || ms->x < x || ms->x > x2 || ms->y <= y || ms->y > y + v)
		return -1;
	return top + ms->y - y - 1;
}

static int
item_ok(const struct item *it)
{
	if(it->action >= A_ZOOMIN && it->action <= A_ZOOMRESET)
		return term_font_size() > 0;	/* a window: a terminal has its own */
	return !it->doc || docs != 0;
}

/* the items of menu m, all of them in order (one that does not apply now
 * is shown greyed, not hidden: the menu keeps its shape) */
static int
shown_items(int m, const struct item **out, int max)
{
	const struct item *it;
	int n = 0;

	for(it = items_of[m]; it->text && n < max; it++)
		out[n++] = it;
	return n;
}

static int
nitems(int m)
{
	const struct item *v[64];

	return shown_items(m, v, 64);
}

static const struct item *
nth(int m, int i)
{
	const struct item *v[64];
	int n = shown_items(m, v, 64);

	return i >= 0 && i < n ? v[i] : 0;
}

/* an item that can be chosen: not a line, and one that applies now */
static int
usable(const struct item *it)
{
	return it && !ISSEP(it) && item_ok(it);
}

/* the next item from i (dir 1 or -1, around) that can be chosen */
static int
next_item(int m, int i, int dir)
{
	int n = nitems(m), k;

	for(k = 0; k < n; k++) {
		i = (i + dir + n) % n;
		if(usable(nth(m, i)))
			return i;
	}
	return 0;
}

/* the widths of menu m: of its texts (*w), of those that have a shortcut
 * (*wk: the shortcuts are in a column after them), of the shortcuts */
static void
widths(int m, int *w, int *wk, int *ws)
{
	const struct item *it;
	int l, k;

	for(it = items_of[m]; it->text; it++) {
		if(ISSEP(it))
			continue;
		l = text_width(it->text);
		if(l > *w)
			*w = l;
		if((k = (int)strlen(it->keys)) == 0)
			continue;
		if(l > *wk)
			*wk = l;
		if(k > *ws)
			*ws = k;
	}
}

/* the item focused, in the rows shown */
static void
keep_seen(int v, int n)
{
	if(st.mitem < st.mtop)
		st.mtop = st.mitem;
	if(st.mitem >= st.mtop + v)
		st.mtop = st.mitem - v + 1;
	if(st.mtop > n - v)
		st.mtop = n - v;
	if(st.mtop < 0)
		st.mtop = 0;
}

#define MENU_MAX	60	/* the widest a menu is (and no wider than the screen) */

/* the open menu, over the rest: as wide as its items need, up to
 * MENU_MAX; one taller than the screen shows the rows that fit, with a
 * scrollbar on its right side: the wheel, its arrows, its track and the
 * keys move them. */
void
draw_menus(void)
{
	static char bardrag;	/* the thumb of its scrollbar is dragged */
	int m = st.menu, n, i, w = 0, wk = 0, ws = 0, iw, kx, x, y, a, v, top, x2, row, bar, room;
	const struct item *it;
	struct mouse *ms = &term_mouse;
	kcode k;

	if(m < 0)
		return;
	if(ui_focus == ID_MENU)
		ui_steal(ID_MENU);	/* it is seen */
	n = nitems(m);
	if(st.mitem >= n || !usable(nth(m, st.mitem)))
		st.mitem = next_item(m, st.mitem >= n ? n - 1 : st.mitem, 1);
	/* inside the frame: the check, the text, two blanks, and the
	   shortcuts in a column after the longest text that has one (a
	   longer text without one takes the room of the column) */
	widths(m, &w, &wk, &ws);
	iw = 2 + w + 2;
	if(ws && 2 + wk + 2 + ws + 2 > iw)
		iw = 2 + wk + 2 + ws + 2;
	i = (scr_cols < MENU_MAX ? scr_cols : MENU_MAX) - 2;
	if(iw > i)		/* too wide: cut at the right, the shortcuts first */
		iw = i > 1 ? i : 1;
	x = m == CTX_MENU ? st.mx : menu_x[m];
	if(x + iw + 2 > scr_cols)
		x = scr_cols - iw - 2;
	if(x < 0)
		x = 0;
	x2 = x + iw + 1;
	kx = x + 1 + 2 + wk + 2;	/* where the shortcuts start */
	/* the rows: as many items as fit under the bar (with the frame) */
	room = scr_rows - 1 - 2;
	if(room < 1)
		room = 1;
	v = n < room ? n : room;
	y = 1;
	if(m == CTX_MENU) {	/* under the click, or over it near the bottom */
		y = st.my;
		if(y + v + 2 > scr_rows)
			y = st.my - v - 1;
		if(y + v + 2 > scr_rows)
			y = scr_rows - v - 2;
		if(y < 1)
			y = 1;
	}
	bar = v < n;		/* not all fit: scrolled */
	if(!bar)
		st.mtop = 0;
	keep_seen(v, n);
	/* its scrollbar (the right side of the frame): the wheel over the
	   menu, a click on an arrow or the track, a drag of the thumb */
	if(ui_key == K_MOUSE && ms->kind == M_RELEASE && bardrag) {
		bardrag = 0;
		ui_key = 0;
	}
	if(bar && ui_key == K_MOUSE && ms->x >= x && ms->x <= x2 && ms->y > y && ms->y <= y + v &&
	   (ms->kind == M_WHEELUP || ms->kind == M_WHEELDOWN)) {
		st.mtop += ms->kind == M_WHEELUP ? -1 : 1;
		ui_key = 0;
		ui_again = 1;
	} else if(bar && ui_key == K_MOUSE && ((ms->kind == M_PRESS && ms->x == x2 && ms->y > y &&
	    ms->y <= y + v) || (ms->kind == M_DRAG && bardrag))) {
		row = ms->y - y - 1;
		if(ms->kind == M_PRESS && (row == 0 || v < 3))
			st.mtop--;
		else if(ms->kind == M_PRESS && row == v - 1)
			st.mtop++;
		else {
			bardrag = 1;
			st.mtop = (int)ui_scrollbar_at(v, n, v, row < 1 ? 1 : row > v - 2 ? v - 2 : row);
		}
		ui_key = 0;
		ui_again = 1;
	}
	if(st.mtop > n - v)
		st.mtop = n - v;
	if(st.mtop < 0)
		st.mtop = 0;
	top = st.mtop;
	if(bar && (st.mitem < top || st.mitem >= top + v)) {
		/* scrolled by the mouse: the focus on an item in sight */
		st.mitem = next_item(m, st.mitem < top ? top - 1 : top + v, st.mitem < top ? 1 : -1);
	}
	/* the mouse: moved or dragged over an item, it is focused; pressed
	   on it, or released on it after a drag, it is done */
	if((i = hovered(x, y, bar ? x2 - 1 : x2, v, top)) >= 0) {
		struct mouse *ms = &term_mouse;

		if(!usable(nth(m, i)))	/* a line, a greyed item: nothing */
			ui_key = 0;
		else if(ms->kind == M_MOVE || ms->kind == M_DRAG) {
			if(ms->kind == M_DRAG)
				st.mdrag = 1;
			if(st.mitem != i)
				ui_again = 1;
			st.mitem = i;
			ui_key = 0;
		} else if((ms->kind == M_PRESS && (ms->button == MB_LEFT || m == CTX_MENU)) ||
		    (ms->kind == M_RELEASE && st.mdrag)) {
			it = nth(m, i);
			ui_key = 0;
			close_menu();
			app_action(it->action);
			return;
		} else
			ui_key = 0;
	} else if(ui_key == K_MOUSE && term_mouse.kind == M_DRAG)
		st.mdrag = 1;
	k = ui_key;
	if(ui_focus == ID_MENU && k) {
		ui_key = 0;
		ui_again = 1;
		if(k == K_UP)
			st.mitem = next_item(m, st.mitem, -1);
		else if(k == K_DOWN)
			st.mitem = next_item(m, st.mitem, 1);
		else if(k == K_HOME || k == K_PGUP) {	/* the first (a page up) */
			i = k == K_HOME || st.mitem - (v - 1) < 0 ? 0 : st.mitem - (v - 1);
			st.mitem = next_item(m, i - 1, 1);
		} else if(k == K_END || k == K_PGDN) {	/* the last (a page down) */
			i = k == K_END || st.mitem + v - 1 > n - 1 ? n - 1 : st.mitem + v - 1;
			st.mitem = next_item(m, i + 1, -1);
		}
		else if((k == K_LEFT || k == K_RIGHT) && m == CTX_MENU) {
			close_menu();
			return;
		} else if(k == K_LEFT || k == K_RIGHT) {
			do
				m = (m + (k == K_LEFT ? NMENUS - 1 : 1)) % NMENUS;
			while(!menu_shown(m));
			open_menu(m);
			return;
		} else if(k == 27) {
			close_menu();
			return;
		} else if((k == 13 || k == ' ') && usable(nth(m, st.mitem))) {
			it = nth(m, st.mitem);
			close_menu();
			app_action(it->action);
			return;
		} else {
			for(i = 0; i < n; i++)
				if(usable(nth(m, i)) && (K_CODE(k) & ~0x20) == nth(m, i)->accel &&
				    !(k & (K_CTRL | K_ALT))) {
					close_menu();
					app_action(nth(m, i)->action);
					return;
				}
			if(k == K_MOUSE)
				ui_key = k;
			else if(k & (K_CTRL | K_ALT)) {
				close_menu();	/* a shortcut: the menu closes and does it */
				ui_key = k;
			}
		}
	}
	keep_seen(v, n);	/* the keys moved the focus */
	top = st.mtop;
	scr_fill(x, y, x2, y + v + 1, ' ', S_FLOAT);
	scr_frame(x, y, x2, y + v + 1, S_FLOAT);
	for(row = 0; row < v; row++) {
		i = top + row;
		it = nth(m, i);
		if(ISSEP(it)) {	/* a line across, between groups */
			scr_put(x, y + 1 + row, G_SEPL, S_FLOAT);
			scr_fill(x + 1, y + 1 + row, x2 - 1, y + 1 + row, G_H, S_FLOAT);
			scr_put(x2, y + 1 + row, G_SEPR, S_FLOAT);
			continue;
		}
		a = i == st.mitem ? S_FOCUS : S_FLOAT;
		if(!item_ok(it))	/* it does not apply now: greyed */
			a = S_FLOAT | F_DIM;
		scr_fill(x + 1, y + 1 + row, x2 - 1, y + 1 + row, ' ', a);
		if(menu_checked(it->action))
			scr_put(x + 1, y + 1 + row, G_CHECKED, a);
		ui_accel(x + 3, y + 1 + row, it->text, it->accel, a,
		    it->keys[0] && kx - 1 < x2 ? kx - 1 : x2);
		if(it->keys[0] && kx < x2)
			scr_text(kx, y + 1 + row, it->keys, a, x2);
	}
	if(bar) {	/* on the right side of the frame */
		ui_scrollbar_again();
		if(v >= 3)
			ui_scrollbar(x2, y + 1, v, top, n, v, 1);
		else {
			if(top > 0)
				scr_put(x2, y + 1, G_SCROLLUP, S_TRACK);
			if(top + v < n)
				scr_put(x2, y + v, G_SCROLLDOWN, S_TRACK);
		}
		ui_scrollbar_again();	/* the editor's: drawn whole next time */
	}
	if(ui_key == K_MOUSE && term_mouse.kind == M_PRESS) {
		ui_key = 0;	/* a click elsewhere closes it */
		close_menu();
	} else if(ui_key == K_MOUSE && term_mouse.kind == M_RELEASE && st.mdrag) {
		st.mdrag = 0;	/* a drag let go elsewhere: it stays open */
		ui_key = 0;
	}
}
