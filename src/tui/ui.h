/* ui: the widgets of an immediate mode UI.  The application draws all of
 * the screen in a pass, and each widget, as it is drawn, takes the key of
 * the pass if it has the focus (or the mouse, if the click is on it): a
 * pass both shows and does.  A pass that changes something asks for
 * another, which shows the change. */
#ifndef UI_H
#define UI_H

#include "screen.h"

#define EDL_MAX		1024

/* a line of text that can be edited */
struct edl {
	char s[EDL_MAX];
	int len, cur, off;	/* off: the first byte shown */
};

extern kcode ui_key;		/* the key of the pass, 0 once used */
extern int ui_focus;		/* the id of the focused widget */
extern int ui_cx, ui_cy;	/* where the cursor goes; -1: hidden */
extern int ui_again;		/* another pass is wanted */
extern int ui_nograb;		/* the next modal keeps no focus: one is on it */
extern const char *ui_clip;	/* what Ctrl+V pastes in an edit line */
extern size_t ui_cliplen;

void ui_pass(kcode key);
/* the end of the pass: the focus goes to deflt if no widget had it */
int ui_end(int deflt);
void ui_steal(int id);		/* the focus to id */
int ui_mouse(int x1, int y, int x2);	/* a press on these cells */
int ui_mouse_ok(void);		/* the mouse is not behind a modal */
int ui_utf8(rune c, char *s);	/* c as UTF-8: its bytes */

/* a group of widgets: Tab and Shift+Tab move the focus among them (and
 * Left and Right among buttons); grab: it takes the focus when no widget
 * of it has it */
void ui_well_begin(int grab);
int ui_well_end(void);		/* 1 if the focus is in it */
int ui_focusable(int id, int kind, int x1, int y, int x2);
enum { W_BUTTON, W_EDIT, W_LIST };

/* s with its accelerator (a letter) underlined: the column after it */
int ui_accel(int x, int y, const char *s, int accel, int at, int lim);
/* [s], with an accelerator letter (or 0): 1 when pressed */
int ui_button(int id, int x, int y, const char *s, int accel, int at);
int ui_check(int id, int x, int y, const char *s, char *v, int at);
int ui_edit(int id, int x, int y, int w, struct edl *e, int at);	/* 1: changed */
void edl_set(struct edl *e, const char *s);
/* a list of n items in w by h cells; *sel is the selected one, *top the
 * first shown: 1 when the selection moved, 2 when an item is activated */
int ui_list(int id, int x, int y, int w, int h, int n,
    const char *(*item)(int i), int *sel, int *top);
/* a scrollbar of h rows: the thumb as long as shown is of total, at pos
 * of the range; 0 if none (always: drawn even when all is shown) */
int ui_scrollbar(int x, int y, int h, long pos, long total, long shown, int always);
/* the same along a row of w columns, from x */
int ui_hscrollbar(int x, int y, int w, long pos, long total, long shown);
void ui_hscrollbar_again(void);
/* the pos a click or a drag at row of the bar asks for (or column) */
long ui_scrollbar_at(int h, long total, long shown, int row);
void ui_scrollbar_again(void);	/* the next one is drawn, as it is or not */
/* s, cut with an ellipsis at lim */
int ui_trunc(int x, int y, const char *s, int at, int lim);

/* a frame in the middle of the screen, the rest dimmed: its corner */
void ui_modal_begin(const char *title, int w, int h, int at, int *x, int *y);
/* a modal that draws its own frames: the rest dimmed, the mouse and the
 * keys its own, until ui_modal_end */
void ui_modal_enter(void);
int ui_modal_end(void);		/* 1: ESC, closed */

#endif
