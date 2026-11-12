/* The window of xcedit (Code Editor): GLFW for the window, the keys and
 * the mouse, and
 * OpenGL to draw the cells.  The text is drawn with stb_truetype from a
 * font in the program (FantasqueSansMono, data/fonts), or one the
 * settings name (a file, or a family found in the font folders); the frames, the
 * scrollbars and the other glyphs of the interface are drawn as shapes, so
 * they meet at the edges of the cells whatever the font.  The editor sees
 * the same grid of cells and the same keys as in a terminal: the files and
 * the folders are those of posix/files.c.  Started from a terminal, it
 * gives the terminal back once its window is open (term_stay: not). */
#define _XOPEN_SOURCE 700	/* fork, setsid: the window apart from the terminal */
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <GLFW/glfw3.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "stb_truetype.h"
#include "../platform.h"
#include "../colours.h"
#include "../icon.h"

#define FONT_PX		16	/* the height of the text, at a scale of 1 */
#define START_COLS	100
#define START_ROWS	32

extern const unsigned char font_regular[];
extern const unsigned long font_regular_size;

struct mouse term_mouse;
const char plat_name[] = "xcedit";

static GLFWwindow *win;
static int cols, rows;			/* the grid */
static cell_t *gch;			/* its characters */
static attr_t *gat;		/* their looks */
static int curx = -1, cury, curon;	/* the cursor */
static int cellw, cellh;		/* a cell, in pixels of the framebuffer */
static float ascent;
static float scale = 1;			/* of the screen (HiDPI) */
static int lowx1, lowx2 = -1, lowy = -1;	/* term_lower */
#define LOWER	0.33f	/* how much lower: a third of a cell (a half
				   looked too low) */
static char dirty = 1;			/* the grid changed since drawn */
static int lastfw;			/* a frame drawn: not the first */

/* --- the events, as the keys of a terminal -------------------------------- */

#define QMAX	1024
static struct ev {
	kcode k;
	struct mouse m;
} q[QMAX];
static int qhead, qtail;
static char skipchar;		/* a key with Ctrl or Alt: not its character */
static int mx = -1, my = -1, mbutton = -1;	/* the mouse: its cell, its button */
static double wheelx, wheely;	/* a touchpad: the parts of a notch */
static char *pastebuf, *lastclip;
static size_t pastelen;

/* XCEDIT_TRACE=file: each event, as the editor gets it (to see what a
 * mouse or a keyboard gives, where it cannot be watched) */
static void
trace(kcode k)
{
	static const char *path;
	static int asked;
	static const char *const kinds[] = { "press", "drag", "release", "wheelup",
		"wheeldown", "move", "wheelleft", "wheelright" };
	FILE *f;

	if(!asked) {
		asked = 1;
		path = getenv("XCEDIT_TRACE");
	}
	if(path == 0 || (f = fopen(path, "a")) == 0)
		return;
	if(k == K_MOUSE)
		fprintf(f, "%.3f mouse %s cell %d,%d button %d clicks %d\n", glfwGetTime(),
		    term_mouse.kind >= 0 && term_mouse.kind < 8 ? kinds[term_mouse.kind] : "?",
		    term_mouse.x, term_mouse.y, term_mouse.button, term_mouse.clicks);
	else
		fprintf(f, "%.3f key %#lx\n", glfwGetTime(), (unsigned long)k);
	fclose(f);
}

static void
push(kcode k)
{
	int n = (qtail + 1) % QMAX;

	trace(k);

	if(n == qhead)
		return;		/* full: dropped */
	q[qtail].k = k;
	q[qtail].m = term_mouse;
	qtail = n;
	glfwPostEmptyEvent();
}

/* a new size, once however many come: the editor lays out once */
static char resize_queued;

static void
push_resize(void)
{
	if(resize_queued)
		return;
	resize_queued = 1;
	push(K_RESIZE);
}

static kcode
mods(int m)
{
	return (m & GLFW_MOD_SHIFT ? K_SHIFT : 0) | (m & GLFW_MOD_CONTROL ? K_CTRL : 0) |
	    (m & GLFW_MOD_ALT ? K_ALT : 0);
}

/* the text of the clipboard, as a paste: 0 if it is what the editor copied
 * (then Ctrl+V pastes its own, with its lines) */
static int
paste_clipboard(void)
{
	const char *s = glfwGetClipboardString(win);
	size_t n;

	if(s == 0 || *s == 0 || (lastclip && strcmp(s, lastclip) == 0))
		return 0;
	n = strlen(s);
	free(pastebuf);
	if((pastebuf = malloc(n + 1)) == 0)
		return 0;
	memcpy(pastebuf, s, n + 1);
	pastelen = n;
	push(K_PASTE);
	return 1;
}

/* Ctrl or Alt (md) and a letter, a digit or a space (k): Ctrl+V is the
 * clipboard's text when another program put it there */
static void
mod_key(kcode k, kcode md)
{
	if(k == 'V' && md == K_CTRL && paste_clipboard())
		return;
	push(k | md);
}

static int shift_down;	/* the Shift keys held: a bit each (their own
			   presses and releases tell it, not the mouse) */

int
term_shift(void)
{
	return shift_down != 0;
}

/* the window left: a Shift let go elsewhere is not seen, so none held */
static void
focus_cb(GLFWwindow *w, int focused)
{
	(void)w;
	if(!focused && shift_down) {
		shift_down = 0;
		push(K_NONE);
	}
}

static void
key_cb(GLFWwindow *w, int key, int scancode, int action, int m)
{
	kcode k = 0, md = mods(m);
	const char *name;
	int was = shift_down;

	(void)w;
	/* Shift alone, pressed or let go: drawn again (a button may say
	   another thing while it is held) */
	if(key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
		int bit = key == GLFW_KEY_LEFT_SHIFT ? 1 : 2;

		shift_down = action == GLFW_RELEASE ? shift_down & ~bit : shift_down | bit;
		if(shift_down != was)
			push(K_NONE);
		return;
	}
	if(action == GLFW_RELEASE)
		return;
	skipchar = 0;
	switch(key) {
	case GLFW_KEY_ESCAPE: k = 27; break;
	case GLFW_KEY_ENTER: case GLFW_KEY_KP_ENTER: k = 13; break;
	case GLFW_KEY_TAB: k = 9; break;
	case GLFW_KEY_BACKSPACE: k = 8; break;
	case GLFW_KEY_INSERT: k = K_INS; break;
	case GLFW_KEY_DELETE: k = K_DEL; break;
	case GLFW_KEY_UP: k = K_UP; break;
	case GLFW_KEY_DOWN: k = K_DOWN; break;
	case GLFW_KEY_LEFT: k = K_LEFT; break;
	case GLFW_KEY_RIGHT: k = K_RIGHT; break;
	case GLFW_KEY_HOME: k = K_HOME; break;
	case GLFW_KEY_END: k = K_END; break;
	case GLFW_KEY_PAGE_UP: k = K_PGUP; break;
	case GLFW_KEY_PAGE_DOWN: k = K_PGDN; break;
	case GLFW_KEY_F1: k = K_F1; break;
	case GLFW_KEY_F2: k = K_F2; break;
	case GLFW_KEY_F3: k = K_F3; break;
	case GLFW_KEY_F4: k = K_F4; break;
	case GLFW_KEY_F5: k = K_F5; break;
	case GLFW_KEY_F10: k = K_F10; break;
	}
	if(k) {
		if(k == K_INS && md == K_SHIFT && paste_clipboard())
			return;
		push(k | md);
		skipchar = (md & (K_CTRL | K_ALT)) != 0;
		return;
	}
	if(!(md & (K_CTRL | K_ALT)))
		return;		/* a character: from char_cb */
	/* Ctrl and the keypad's + - 0: the zoom, as the keys of the row */
	if(key == GLFW_KEY_KP_ADD || key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_KP_0) {
		push(K_CTRL | (key == GLFW_KEY_KP_ADD ? '=' : key == GLFW_KEY_KP_SUBTRACT ? '-' : '0'));
		skipchar = 1;
		return;
	}
	/* Ctrl or Alt and a letter, a digit or a space: as the terminal has
	   them (the letter as on the keyboard's layout).  A key with no name
	   (one the US keyboard has not, as the / of ABNT2): its character
	   comes to char_cb */
	if(key == GLFW_KEY_SPACE)
		k = ' ';
	else if((name = glfwGetKeyName(key, scancode)) != 0 && name[0] && !name[1])
		k = (unsigned char)name[0];
	else
		return;
	if(k >= 'a' && k <= 'z')
		k -= 32;
	skipchar = 1;
	mod_key(k, md);
}

/* a character, with the modifiers held: typed (none, or Shift), or with
 * Ctrl or Alt when key_cb did not take the key; the character is the
 * layout's (Shift and AltGr in it), so Shift is not said again */
static void
char_cb(GLFWwindow *w, unsigned int c, int m)
{
	kcode md = mods(m) & (K_CTRL | K_ALT);

	(void)w;
	if(skipchar)
		return;
	if(md && c >= 'a' && c <= 'z')
		c -= 32;
	push((kcode)c | md);
}

/* the cell of a position of the window */
static void
cell_at(double x, double y, int *cx, int *cy)
{
	int ww, wh, fw, fh;

	glfwGetWindowSize(win, &ww, &wh);
	glfwGetFramebufferSize(win, &fw, &fh);
	if(ww > 0)
		x = x * fw / ww;
	if(wh > 0)
		y = y * fh / wh;
	*cx = (int)(x / cellw);
	*cy = (int)(y / cellh);
	/* the cells drawn lower (term_lower): where they are seen */
	if(lowy >= 0 && *cx >= lowx1 && *cx <= lowx2) {
		if(y >= (lowy + LOWER) * cellh && y < (lowy + 1 + LOWER) * cellh)
			*cy = lowy;
		else if(*cy == lowy)
			*cy = lowy - 1;
	}
	if(*cx >= cols)
		*cx = cols - 1;
	if(*cy >= rows)
		*cy = rows - 1;
}

static void
mouse_event(int kind, int button, int m)
{
	term_mouse.x = mx;
	term_mouse.y = my;
	term_mouse.kind = kind;
	term_mouse.button = button;
	term_mouse.mods = mods(m);
	if(shift_down)		/* a move comes without its modifiers */
		term_mouse.mods |= K_SHIFT;
	push(K_MOUSE);
	term_mouse.clicks = 1;
}

static int
button_of(int b)
{
	return b == GLFW_MOUSE_BUTTON_LEFT ? MB_LEFT : b == GLFW_MOUSE_BUTTON_MIDDLE ? MB_MIDDLE :
	    b == GLFW_MOUSE_BUTTON_RIGHT ? MB_RIGHT : -1;
}

/* a press or a release of a button at the cell mx, my: of the mouse, or
   of a touch (touch_*) */
static void
click(int button, int press, int m)
{
	static double last;
	static int lx = -1, ly = -1, lclicks;
	double now;

	if(press) {
		/* the clicks in a row at one place: 1, 2, 3, then 1 again */
		now = glfwGetTime();
		term_mouse.clicks = 1;
		if(button == MB_LEFT && now - last < 0.4 && lx == mx && ly == my)
			term_mouse.clicks = lclicks % 3 + 1;
		if(button == MB_LEFT) {
			lclicks = term_mouse.clicks;
			last = now;
			lx = mx;
			ly = my;
		}
		mbutton = button;
		mouse_event(M_PRESS, button, m);
	} else {
		mbutton = -1;
		mouse_event(M_RELEASE, button, m);
	}
}

static void
button_cb(GLFWwindow *w, int b, int action, int m)
{
	double x, y;
	int button = button_of(b);

	if(button < 0)
		return;
	glfwGetCursorPos(w, &x, &y);
	cell_at(x, y, &mx, &my);
	click(button, action == GLFW_PRESS, m);
}

#ifdef TOUCH_WAYLAND
static void pos_cb(GLFWwindow *w, double x, double y);

/* A touch screen: GLFW gives none of its touches (on Wayland, not even
 * as the mouse; on X11 the server makes them clicks of the mouse), so the
 * window reads them itself (touch_wayland).  A finger:
 *   a tap               a click (twice: a double click, the word ...)
 *   moved               the text scrolls with it, as the wheel does
 *   held, then lifted   the right button: the context menu
 *   held, then moved    the left button held: a selection
 * Two fingers (the second down before the first is held or selects):
 *   apart, or nearer    the zoom, as Ctrl and + or -
 *   moved together      the text scrolls with them
 * then nothing until all are lifted.  x, y: in the window's units
 * (cell_at); f: the finger, 0 or 1 */
#define TOUCH_SLOP	8.0	/* moved this far (units of the window): not a tap */
#define TOUCH_HOLD	0.45	/* held this long (seconds): not a tap */
#define TOUCH_PINCH	1.12	/* the fingers this much farther apart, or
				   nearer: a step of the zoom */

enum { T_TAP, T_SCROLL, T_SELECT, T_PINCH, T_DONE };

static struct {
	int down;		/* the fingers down: 1 << f */
	int first;		/* the finger that touched first */
	int mode;		/* T_* */
	double x0, y0, t0;	/* where and when it touched */
	double lx, ly;		/* where it (or the two, their middle) was last */
	double accx, accy;	/* moved, not yet scrolled */
	double fx[2], fy[2];	/* where each finger is */
	double dist;		/* T_PINCH: the fingers' distance at the last step */
} touch;

static double
touch_dist(void)
{
	double dx = touch.fx[0] - touch.fx[1], dy = touch.fy[0] - touch.fy[1];

	return sqrt(dx * dx + dy * dy);
}

/* the text follows the finger, now at x, y: a notch of the wheel is 3
   lines, or 6 columns (editor.c) */
static void
touch_scroll(double x, double y)
{
	int ww, wh, fw, fh;
	double ux = 1, uy = 1;	/* a cell, in units of the window */

	glfwGetWindowSize(win, &ww, &wh);
	glfwGetFramebufferSize(win, &fw, &fh);
	if(fw > 0)
		ux = (double)cellw * ww / fw;
	if(fh > 0)
		uy = (double)cellh * wh / fh;
	touch.accy += y - touch.ly;
	touch.accx += x - touch.lx;
	touch.lx = x;
	touch.ly = y;
	cell_at(x, y, &mx, &my);
	for(; touch.accy >= 3 * uy; touch.accy -= 3 * uy)
		mouse_event(M_WHEELUP, 0, 0);
	for(; touch.accy <= -3 * uy; touch.accy += 3 * uy)
		mouse_event(M_WHEELDOWN, 0, 0);
	for(; touch.accx >= 6 * ux; touch.accx -= 6 * ux)
		mouse_event(M_WHEELLEFT, 0, 0);
	for(; touch.accx <= -6 * ux; touch.accx += 6 * ux)
		mouse_event(M_WHEELRIGHT, 0, 0);
}

static void
touch_down(int f, double x, double y)
{
	if(touch.down & 1 << f)
		return;
	touch.down |= 1 << f;
	touch.fx[f] = x;
	touch.fy[f] = y;
	if(touch.down == 1 << f) {	/* the first */
		touch.first = f;
		touch.mode = T_TAP;
		touch.x0 = touch.lx = x;
		touch.y0 = touch.ly = y;
		touch.t0 = glfwGetTime();
		touch.accx = touch.accy = 0;
		cell_at(x, y, &mx, &my);
		mouse_event(M_MOVE, 3, 0);	/* the menu bar's hover, as the mouse */
		return;
	}
	/* the second: a pinch, unless the first selects (or is done) */
	if(touch.mode != T_TAP && touch.mode != T_SCROLL)
		return;
	touch.mode = T_PINCH;
	touch.dist = touch_dist();
	touch.lx = (touch.fx[0] + touch.fx[1]) / 2;
	touch.ly = (touch.fy[0] + touch.fy[1]) / 2;
	touch.accx = touch.accy = 0;
}

static void
touch_motion(int f, double x, double y)
{
	double dx, dy, d;

	if(!(touch.down & 1 << f))
		return;
	touch.fx[f] = x;
	touch.fy[f] = y;
	if(touch.mode == T_PINCH) {
		d = touch_dist();
		if(touch.dist < 1)
			touch.dist = d;	/* the two touched at one place */
		else if(d >= touch.dist * TOUCH_PINCH) {
			for(; d >= touch.dist * TOUCH_PINCH; touch.dist *= TOUCH_PINCH)
				push(K_CTRL | '=');
			touch.accx = touch.accy = 0;
		} else if(d <= touch.dist / TOUCH_PINCH) {
			for(; d <= touch.dist / TOUCH_PINCH; touch.dist /= TOUCH_PINCH)
				push(K_CTRL | '-');
			touch.accx = touch.accy = 0;
		}
		touch_scroll((touch.fx[0] + touch.fx[1]) / 2, (touch.fy[0] + touch.fy[1]) / 2);
		return;
	}
	if(f != touch.first || touch.mode == T_DONE)
		return;
	if(touch.mode == T_TAP) {
		dx = x - touch.x0;
		dy = y - touch.y0;
		if(dx * dx + dy * dy < TOUCH_SLOP * TOUCH_SLOP)
			return;
		if(glfwGetTime() - touch.t0 >= TOUCH_HOLD) {
			/* held, then moved: a selection from where it touched */
			touch.mode = T_SELECT;
			cell_at(touch.x0, touch.y0, &mx, &my);
			click(MB_LEFT, 1, 0);
		} else
			touch.mode = T_SCROLL;
	}
	if(touch.mode == T_SELECT) {
		pos_cb(win, x, y);
		touch.lx = x;
		touch.ly = y;
		return;
	}
	touch_scroll(x, y);
}

static void
touch_up(int f)
{
	if(!(touch.down & 1 << f))
		return;
	touch.down &= ~(1 << f);
	if(f != touch.first || touch.mode == T_PINCH) {
		if(touch.mode == T_PINCH)
			touch.mode = T_DONE;	/* the other: no tap, no scroll */
		return;
	}
	if(touch.mode == T_SELECT) {
		cell_at(touch.lx, touch.ly, &mx, &my);
		click(MB_LEFT, 0, 0);
	} else if(touch.mode == T_TAP) {
		cell_at(touch.x0, touch.y0, &mx, &my);
		if(glfwGetTime() - touch.t0 >= TOUCH_HOLD) {
			click(MB_RIGHT, 1, 0);
			click(MB_RIGHT, 0, 0);
		} else {
			click(MB_LEFT, 1, 0);
			click(MB_LEFT, 0, 0);
		}
	}
	touch.mode = T_DONE;
}

/* the touches taken away (by the compositor: a gesture of its own) */
static void
touch_cancel(void)
{
	if(touch.down && touch.mode == T_SELECT)
		click(MB_LEFT, 0, 0);
	touch.down = 0;
	touch.mode = T_DONE;
}

/* Wayland: the window's own wl_touch, on the display GLFW has (its events
   come in glfwPollEvents, on the default queue).  Two fingers at most;
   the others are not seen */
#include <wayland-client.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

static struct wl_touch *wtouch;
static int32_t wfinger[2] = { -1, -1 };	/* the ids of the fingers followed */

static int
wt_finger(int32_t id)
{
	return wfinger[0] == id ? 0 : wfinger[1] == id ? 1 : -1;
}

static void
wt_down(void *d, struct wl_touch *t, uint32_t serial, uint32_t time,
    struct wl_surface *s, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	int f = wt_finger(-1);

	(void)d; (void)t; (void)serial; (void)time;
	if(f < 0 || s != glfwGetWaylandWindow(win))
		return;	/* a third finger, or the title bar (libdecor's) */
	wfinger[f] = id;
	touch_down(f, wl_fixed_to_double(x), wl_fixed_to_double(y));
}

static void
wt_up(void *d, struct wl_touch *t, uint32_t serial, uint32_t time, int32_t id)
{
	int f = wt_finger(id);

	(void)d; (void)t; (void)serial; (void)time;
	if(id < 0 || f < 0)
		return;
	wfinger[f] = -1;
	touch_up(f);
}

static void
wt_motion(void *d, struct wl_touch *t, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
	int f = wt_finger(id);

	(void)d; (void)t; (void)time;
	if(id >= 0 && f >= 0)
		touch_motion(f, wl_fixed_to_double(x), wl_fixed_to_double(y));
}

static void
wt_frame(void *d, struct wl_touch *t)
{
	(void)d; (void)t;
}

static void
wt_cancel(void *d, struct wl_touch *t)
{
	(void)d; (void)t;
	wfinger[0] = wfinger[1] = -1;
	touch_cancel();
}

static const struct wl_touch_listener wt_listener = {	/* version 5: no shape */
	.down = wt_down, .up = wt_up, .motion = wt_motion, .frame = wt_frame,
	.cancel = wt_cancel
};

static void
seat_caps(void *d, struct wl_seat *seat, uint32_t caps)
{
	(void)d;
	if((caps & WL_SEAT_CAPABILITY_TOUCH) && wtouch == 0) {
		wtouch = wl_seat_get_touch(seat);
		wl_touch_add_listener(wtouch, &wt_listener, 0);
	} else if(!(caps & WL_SEAT_CAPABILITY_TOUCH) && wtouch != 0) {
		wl_touch_destroy(wtouch);
		wtouch = 0;
		wfinger[0] = wfinger[1] = -1;
		touch_cancel();
	}
}

static void
seat_name(void *d, struct wl_seat *seat, const char *name)
{
	(void)d; (void)seat; (void)name;
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_caps, .name = seat_name
};

static void
reg_global(void *d, struct wl_registry *r, uint32_t name, const char *iface, uint32_t version)
{
	struct wl_seat *seat;

	(void)d;
	if(strcmp(iface, "wl_seat") != 0 || wtouch != 0)
		return;	/* a seat: the first that has a touch screen */
	seat = wl_registry_bind(r, name, &wl_seat_interface, version < 5 ? version : 5);
	wl_seat_add_listener(seat, &seat_listener, 0);
}

static void
reg_remove(void *d, struct wl_registry *r, uint32_t name)
{
	(void)d; (void)r; (void)name;
}

static const struct wl_registry_listener reg_listener = {
	.global = reg_global, .global_remove = reg_remove
};

static void
touch_open(void)
{
	struct wl_display *dpy;
	struct wl_registry *reg;

	if(glfwGetPlatform() != GLFW_PLATFORM_WAYLAND || (dpy = glfwGetWaylandDisplay()) == 0)
		return;
	reg = wl_display_get_registry(dpy);
	wl_registry_add_listener(reg, &reg_listener, 0);
	wl_display_roundtrip(dpy);	/* the seats */
	wl_display_roundtrip(dpy);	/* their capabilities */
}
#else
static void
touch_open(void)
{
}
#endif

static void
pos_cb(GLFWwindow *w, double x, double y)
{
	int cx, cy;

	(void)w;
	cell_at(x, y, &cx, &cy);
	if(cx == mx && cy == my)
		return;		/* the same cell */
	mx = cx;
	my = cy;
	if(mbutton >= 0)
		mouse_event(M_DRAG, mbutton, 0);
	else
		mouse_event(M_MOVE, 3, 0);
}

/* the mouse out of the window: a move to no cell (no button under it) */
static void
enter_cb(GLFWwindow *w, int entered)
{
	(void)w;
	if(entered || mbutton >= 0)
		return;
	mx = my = -1;
	mouse_event(M_MOVE, 3, 0);
}

static void
scroll_cb(GLFWwindow *w, double dx, double dy)
{
	double x, y;

	glfwGetCursorPos(w, &x, &y);
	cell_at(x, y, &mx, &my);
	/* Ctrl and the wheel: the zoom, as Ctrl and + or - */
	if(glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
	   glfwGetKey(w, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
		wheely += dy;
		for(; wheely >= 1; wheely -= 1)
			push(K_CTRL | '=');
		for(; wheely <= -1; wheely += 1)
			push(K_CTRL | '-');
		return;
	}
	/* a notch of a wheel is 1; a touchpad gives parts of it */
	wheely += dy;
	wheelx += dx;
	while(wheely >= 1) {
		wheely -= 1;
		mouse_event(M_WHEELUP, 0, glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) ? GLFW_MOD_SHIFT : 0);
	}
	while(wheely <= -1) {
		wheely += 1;
		mouse_event(M_WHEELDOWN, 0, glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) ? GLFW_MOD_SHIFT : 0);
	}
	while(wheelx >= 1) {
		wheelx -= 1;
		mouse_event(M_WHEELLEFT, 0, 0);
	}
	while(wheelx <= -1) {
		wheelx += 1;
		mouse_event(M_WHEELRIGHT, 0, 0);
	}
}

static void
size_cb(GLFWwindow *w, int width, int height)
{
	(void)w;
	(void)width;
	(void)height;
	push_resize();
}

static void draw(void);
static void grid_dims(int fw, int fh, int *c, int *r);

static void
refresh_cb(GLFWwindow *w)
{
	(void)w;
	draw();		/* the grid as it is, if of the size */
}

static int nerrors;	/* of GLFW, since the start */

static void
close_cb(GLFWwindow *w)
{
	glfwSetWindowShouldClose(w, GLFW_FALSE);
	if(nerrors)	/* GLFW closes the windows of a display lost: said */
		fprintf(stderr, "xcedit: closed by the window system, after an error\n");
	push(K_CTRL | 'Q');	/* Exit: the unsaved changes are asked */
}

/* the errors of GLFW: to the log (the journal, from the menus of the
 * system), not lost */
static void
error_cb(int code, const char *msg)
{
	nerrors++;
	fprintf(stderr, "xcedit: %s (GLFW error %#x)\n", msg ? msg : "?", (unsigned)code);
}

static int
wayland(void)
{
#ifdef GLFW_PLATFORM_WAYLAND
	return glfwGetPlatform() == GLFW_PLATFORM_WAYLAND;
#else
	return 0;
#endif
}

/* --- the font: its glyphs in one texture (an atlas) ---------------------- */

static stbtt_fontinfo font;
static float fscale;
static int fontpx = FONT_PX;	/* the size of the text: the zoom */
static unsigned char *fontfile;	/* the bytes of a font read (0: the one in
				   the program) */
static int font_ready;		/* font holds one */

/* The glyphs drawn once into one texture, in shelves (rows of glyphs), and
 * kept: the text of a frame is then one draw of many quads.  Full, it is
 * emptied and the frame drawn again. */
#define ATLAS		2048
#define GMAX		8192
static GLuint atlas;
static unsigned char *atlas_px;
static int shelf_x, shelf_y, shelf_h;
static struct glyph {
	long c;			/* -1: none */
	char has;		/* the font has it (else a box is drawn) */
	int w, h, xoff, yoff;	/* its bitmap; 0 wide: blank */
	float s0, t0, s1, t1;	/* where in the atlas */
} glyphs[GMAX];
static int nglyphs;

static void
glyphs_flush(void)
{
	int i;

	for(i = 0; i < GMAX; i++)
		glyphs[i].c = -1;
	nglyphs = 0;
	shelf_x = shelf_y = shelf_h = 0;
	if(atlas_px == 0)
		atlas_px = calloc(ATLAS, ATLAS);
	if(atlas == 0) {
		glGenTextures(1, &atlas);
		glBindTexture(GL_TEXTURE_2D, atlas);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS, ATLAS, 0, GL_ALPHA,
		    GL_UNSIGNED_BYTE, 0);
	}
}

static struct glyph *
glyph(long c)
{
	struct glyph *g;
	unsigned h = (unsigned)c * 2654435761u % GMAX;
	int x0, y0, x1, y1, w, ht;

	while(glyphs[h].c >= 0 && glyphs[h].c != c)
		h = (h + 1) % GMAX;
	g = &glyphs[h];
	if(g->c == c)
		return g;
	if(nglyphs > GMAX * 3 / 4)
		return 0;	/* full: emptied by the frame */
	g->c = c;
	g->w = 0;
	nglyphs++;
	g->has = stbtt_FindGlyphIndex(&font, (int)c) != 0;
	if(!g->has)
		return g;
	stbtt_GetCodepointBitmapBox(&font, (int)c, fscale, fscale, &x0, &y0, &x1, &y1);
	w = x1 - x0;
	ht = y1 - y0;
	if(w <= 0 || ht <= 0)
		return g;	/* blank, as a space */
	if(shelf_x + w + 1 > ATLAS) {	/* the next shelf */
		shelf_x = 0;
		shelf_y += shelf_h + 1;
		shelf_h = 0;
	}
	if(shelf_y + ht + 1 > ATLAS || w > ATLAS) {
		g->c = -1;
		nglyphs--;
		return 0;	/* full: emptied by the frame */
	}
	stbtt_MakeCodepointBitmap(&font, atlas_px + shelf_y * ATLAS + shelf_x, w, ht, ATLAS,
	    fscale, fscale, (int)c);
	glBindTexture(GL_TEXTURE_2D, atlas);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, ATLAS);
	glTexSubImage2D(GL_TEXTURE_2D, 0, shelf_x, shelf_y, w, ht, GL_ALPHA, GL_UNSIGNED_BYTE,
	    atlas_px + shelf_y * ATLAS + shelf_x);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	g->w = w;
	g->h = ht;
	g->xoff = x0;
	g->yoff = y0;
	g->s0 = (float)shelf_x / ATLAS;
	g->t0 = (float)shelf_y / ATLAS;
	g->s1 = (float)(shelf_x + w) / ATLAS;
	g->t1 = (float)(shelf_y + ht) / ATLAS;
	shelf_x += w + 1;
	if(ht > shelf_h)
		shelf_h = ht;
	return g;
}

/* the cell size, for the font at its size and the scale of the screen */
static void
font_size(void)
{
	int a, d, gap, adv, lsb;

	fscale = stbtt_ScaleForPixelHeight(&font, fontpx * scale);
	stbtt_GetFontVMetrics(&font, &a, &d, &gap);
	stbtt_GetCodepointHMetrics(&font, 'M', &adv, &lsb);
	cellw = (int)(adv * fscale + 0.5f);
	cellh = (int)((a - d + gap) * fscale + 0.5f) + (int)(2 * scale);
	ascent = (float)(int)(a * fscale + scale + 0.5f);
	if(cellw < 1)
		cellw = 1;
	if(cellh < 1)
		cellh = 1;
}

/* --- the font of the settings ------------------------------------------ */

/* a file read whole: its bytes (freed with free), 0 if it cannot be */
static unsigned char *
read_all(const char *path)
{
	unsigned char *p;
	pos_t size;
	int h, n, k = 0;

	if((h = file_open(path, 0)) < 0)
		return 0;
	size = file_size(h);
	if(size == POS_NONE || size < 12 || size > ((pos_t)64 << 20) ||
	   (p = malloc((size_t)size)) == 0) {
		file_close(h);
		return 0;
	}
	while(k < (int)size && (n = file_read(h, (char *)p + k, (int)size - k)) > 0)
		k += n;
	file_close(h);
	if(k != (int)size) {
		free(p);
		return 0;
	}
	return p;
}

/* a and b the same name, but for the case and the blanks */
static int
same_name(const char *a, int na, const char *b, int nb)
{
	int i = 0, j = 0, x, y;

	for(;;) {
		while(i < na && a[i] == ' ')
			i++;
		while(j < nb && b[j] == ' ')
			j++;
		if(i == na || j == nb)
			return i == na && j == nb;
		x = a[i++];
		y = b[j++];
		if(x >= 'A' && x <= 'Z')
			x += 32;
		if(y >= 'A' && y <= 'Z')
			y += 32;
		if(x != y)
			return 0;
	}
}

/* --- a font by its name: the font folders, the name table of each file ---
 *
 * The folders fontconfig reads too: ~/.local/share/fonts (or
 * $XDG_DATA_HOME/fonts), ~/.fonts, and fonts/ in each of $XDG_DATA_DIRS
 * (/usr/local/share, /usr/share).  Of each .ttf, .otf and .ttc only its
 * table of names is read (a few KB): its family (name 16, else 1) and its
 * style (17, else 2), in English. */

#define FONT_DEPTH	8		/* folders in folders, at most */
#define NAME_MAX_LEN	128

struct font_want {
	const char *family, *style;	/* style: "" for Regular */
	int nfamily, nstyle;
	int rank;			/* of the best found: 0 none */
	char path[1024];
	int index;
};

static unsigned
be16(const unsigned char *p)
{
	return (unsigned)p[0] << 8 | p[1];
}

static unsigned long
be32(const unsigned char *p)
{
	return (unsigned long)p[0] << 24 | (unsigned long)p[1] << 16 |
	    (unsigned long)p[2] << 8 | p[3];
}

static int
read_at(FILE *f, unsigned long off, void *buf, size_t n)
{
	return fseek(f, (long)off, SEEK_SET) == 0 && fread(buf, 1, n, f) == n;
}

/* names id1 (else id2) of the name table at off, in out (ASCII; others
 * as '?'): its length, 0 if none */
static int
font_name(FILE *f, unsigned long off, unsigned long len, int id1, int id2, char *out)
{
	unsigned char h[6], r[12], s[2 * NAME_MAX_LEN];
	unsigned i, count, strs, pid, eid, lang, nid, n, o, best = 0, score;
	unsigned long at = 0;
	int k, len_out = 0, wide = 0;

	if(len < 6 || !read_at(f, off, h, 6))
		return 0;
	count = be16(h + 2);
	strs = be16(h + 4);
	for(i = 0; i < count && 6 + 12 * (i + 1) <= len; i++) {
		if(!read_at(f, off + 6 + 12 * i, r, 12))
			return 0;
		pid = be16(r);
		eid = be16(r + 2);
		lang = be16(r + 4);
		nid = be16(r + 6);
		if(nid != (unsigned)id1 && nid != (unsigned)id2)
			continue;
		/* Windows in English, Unicode, Mac Roman: the first the best; the
		   preferred name better than the other */
		score = pid == 3 && (eid == 1 || eid == 10) ? (lang == 0x409 ? 6 : 4) :
		    pid == 0 ? 5 : pid == 1 && eid == 0 && lang == 0 ? 3 : 0;
		if(score == 0)
			continue;
		score = score * 2 + (nid == (unsigned)id1);
		if(score <= best)
			continue;
		best = score;
		n = be16(r + 8);
		o = be16(r + 10);
		at = off + strs + o;
		wide = pid != 1;
		len_out = (int)(n < sizeof s ? n : sizeof s);
	}
	if(best == 0 || !read_at(f, at, s, (size_t)len_out))
		return 0;
	if(wide) {	/* UTF-16, big end first */
		for(k = 0; k + 1 < len_out; k += 2) {
			unsigned c = be16(s + k);

			*out++ = (char)(c >= 0x20 && c < 0x7F ? c : '?');
		}
		len_out /= 2;
	} else
		for(k = 0; k < len_out; k++)
			*out++ = (char)(s[k] >= 0x20 && s[k] < 0x7F ? s[k] : '?');
	*out = 0;
	return len_out;
}

/* how well a style fits the one wanted: 3 the same, 2 a regular one when
 * none is asked for, 1 another */
static int
style_rank(const struct font_want *w, const char *st)
{
	int n = (int)strlen(st);

	if(w->nstyle)
		return same_name(st, n, w->style, w->nstyle) ? 3 : 1;
	if(same_name(st, n, "Regular", 7) || same_name(st, n, "Book", 4) ||
	   same_name(st, n, "Normal", 6) || same_name(st, n, "Roman", 5))
		return 3;
	return 1;
}

/* the fonts of the file at path, each looked at */
static void
font_file(struct font_want *w, const char *path)
{
	unsigned char h[12], t[16];
	char fam[NAME_MAX_LEN + 1], sty[NAME_MAX_LEN + 1];
	unsigned long offs[64], noff = 1, i, j, ntab;
	FILE *f;
	int rank;

	if((f = fopen(path, "rb")) == 0)
		return;
	offs[0] = 0;
	if(read_at(f, 0, h, 12) && memcmp(h, "ttcf", 4) == 0) {	/* a collection */
		noff = be32(h + 8);
		if(noff > 64)
			noff = 64;
		for(i = 0; i < noff; i++) {
			if(!read_at(f, 12 + 4 * i, t, 4))
				noff = i;
			else
				offs[i] = be32(t);
		}
	}
	for(i = 0; i < noff; i++) {
		if(!read_at(f, offs[i], h, 12))
			break;
		ntab = be16(h + 4);
		for(j = 0; j < ntab && j < 128; j++) {
			if(!read_at(f, offs[i] + 12 + 16 * j, t, 16))
				break;
			if(memcmp(t, "name", 4) != 0)
				continue;
			if(font_name(f, be32(t + 8), be32(t + 12), 16, 1, fam) &&
			   same_name(fam, (int)strlen(fam), w->family, w->nfamily)) {
				if(!font_name(f, be32(t + 8), be32(t + 12), 17, 2, sty))
					sty[0] = 0;
				rank = style_rank(w, sty);
				if(rank > w->rank && strlen(path) < sizeof w->path) {
					w->rank = rank;
					strcpy(w->path, path);
					w->index = (int)i;
				}
			}
			break;
		}
	}
	fclose(f);
}

static int
font_suffix(const char *name)
{
	const char *d = strrchr(name, '.');
	char x[5];
	int i;

	if(d == 0 || strlen(d) != 4)
		return 0;
	for(i = 0; i < 4; i++)
		x[i] = (char)(d[i] >= 'A' && d[i] <= 'Z' ? d[i] + 32 : d[i]);
	x[4] = 0;
	return strcmp(x, ".ttf") == 0 || strcmp(x, ".otf") == 0 || strcmp(x, ".ttc") == 0;
}

/* the fonts under the folder dir, until the one wanted is found */
static void
font_folder(struct font_want *w, const char *dir, int depth)
{
	char name[256], *names = 0, *nn, path[1024];
	size_t used = 0, cap = 0, n;
	int kind;
	char *p;

	if(depth > FONT_DEPTH || !dir_open(dir))
		return;
	/* the entries first (one folder is open at a time): a kind, then the
	   name, each */
	while((kind = dir_entry(name, (int)sizeof name)) != DK_END) {
		if(name[0] == '.' || (kind == DK_FILE && !font_suffix(name)) || kind == DK_OTHER)
			continue;
		n = strlen(name) + 2;
		if(used + n > cap) {
			cap = cap ? cap * 2 : 4096;
			while(used + n > cap)
				cap *= 2;
			if((nn = realloc(names, cap)) == 0)
				break;
			names = nn;
		}
		names[used] = (char)kind;
		memcpy(names + used + 1, name, n - 1);
		used += n;
	}
	dir_close();
	for(p = names; p && p < names + used && w->rank < 3; p += strlen(p + 1) + 2) {
		size_t nd = strlen(dir), nf = strlen(p + 1);

		if(nd + nf + 2 > sizeof path)
			continue;
		memcpy(path, dir, nd);	/* dir/name */
		path[nd] = '/';
		memcpy(path + nd + 1, p + 1, nf + 1);
		if(*p == DK_DIR)
			font_folder(w, path, depth + 1);
		else
			font_file(w, path);
	}
	free(names);
}

/* the file of the font named "Family" or "Family:Style": 1, its path and
 * its index in the file; 0 if no font folder has that family */
static int
font_lookup(const char *name, char *path, int max, int *index)
{
	static struct font_want w;
	const char *home = getenv("HOME"), *data = getenv("XDG_DATA_HOME");
	const char *dirs = getenv("XDG_DATA_DIRS"), *c, *e;
	char dir[1024];

	memset(&w, 0, sizeof w);
	w.family = name;
	c = strchr(name, ':');
	w.nfamily = c ? (int)(c - name) : (int)strlen(name);
	w.style = c ? c + 1 : "";
	if(strncmp(w.style, "style=", 6) == 0)
		w.style += 6;
	w.nstyle = (int)strlen(w.style);
	if(data && *data && strlen(data) < sizeof dir - 8) {
		snprintf(dir, sizeof dir, "%s/fonts", data);
		font_folder(&w, dir, 0);
	} else if(home && strlen(home) < sizeof dir - 24) {
		snprintf(dir, sizeof dir, "%s/.local/share/fonts", home);
		font_folder(&w, dir, 0);
	}
	if(w.rank < 3 && home && strlen(home) < sizeof dir - 8) {
		snprintf(dir, sizeof dir, "%s/.fonts", home);
		font_folder(&w, dir, 0);
	}
	if(dirs == 0 || *dirs == 0)
		dirs = "/usr/local/share:/usr/share";
	for(c = dirs; w.rank < 3 && *c; c = *e ? e + 1 : e) {
		e = strchr(c, ':');
		if(e == 0)
			e = c + strlen(c);
		if(e > c && (size_t)(e - c) < sizeof dir - 8) {
			size_t n = (size_t)(e - c);

			while(n > 1 && c[n - 1] == '/')	/* /usr/share/ */
				n--;
			memcpy(dir, c, n);
			strcpy(dir + n, "/fonts");
			font_folder(&w, dir, 0);
		}
	}
	/* the fonts of the system where they are not in XDG_DATA_DIRS: those
	   of macOS; of X11 on OpenBSD and NetBSD */
	{
		static const char *const sys[] = {
#if defined(__APPLE__)
			"~/Library/Fonts", "/Library/Fonts", "/System/Library/Fonts",
#endif
			"/usr/X11R6/lib/X11/fonts", "/usr/pkg/share/fonts",
		};
		size_t i;

		for(i = 0; w.rank < 3 && i < sizeof sys / sizeof sys[0]; i++) {
			if(sys[i][0] == '~') {
				if(home == 0 || strlen(home) + strlen(sys[i]) >= sizeof dir)
					continue;
				snprintf(dir, sizeof dir, "%s%s", home, sys[i] + 1);
			} else
				snprintf(dir, sizeof dir, "%s", sys[i]);
			font_folder(&w, dir, 0);
		}
	}
	if(w.rank == 0 || (int)strlen(w.path) >= max)
		return 0;
	strcpy(path, w.path);
	*index = w.index;
	return 1;
}

int
term_set_font(const char *spec)
{
	char path[1024];
	const char *home;
	unsigned char *data;
	stbtt_fontinfo fi;
	int index = 0, off;

	if(spec[0] == 0) {	/* the one in the program */
		data = 0;
		off = stbtt_GetFontOffsetForIndex(font_regular, 0);
		if(!stbtt_InitFont(&fi, font_regular, off))
			return FONT_BAD;
	} else {
		if(spec[0] == '~' && spec[1] == '/' && (home = getenv("HOME")) != 0 &&
		   strlen(home) + strlen(spec) < sizeof path) {
			strcpy(path, home);
			strcat(path, spec + 1);
		} else if(strchr(spec, '/') && strlen(spec) < sizeof path)
			strcpy(path, spec);
		else if(!font_lookup(spec, path, (int)sizeof path, &index))
			return FONT_NOTFOUND;
		if(file_kind(path) != 1)
			return FONT_NOTFOUND;
		if((data = read_all(path)) == 0)
			return FONT_BAD;
		/* a font: TrueType or CFF outlines, with the letters */
		if((off = stbtt_GetFontOffsetForIndex(data, index)) < 0 ||
		   !stbtt_InitFont(&fi, data, off) || stbtt_FindGlyphIndex(&fi, 'M') == 0) {
			free(data);
			return FONT_BAD;
		}
	}
	free(fontfile);
	fontfile = data;
	font = fi;
	font_ready = 1;
	if(win) {	/* the cells of the new one, the glyphs drawn again */
		font_size();
		glyphs_flush();
		push_resize();
	}
	return FONT_OK;
}

/* --- the drawing --------------------------------------------------------- */

static void
colour(unsigned long c)
{
	glColor3ub((GLubyte)(c >> 16), (GLubyte)(c >> 8 & 0xFF), (GLubyte)(c & 0xFF));
}

static void
rect(float x0, float y0, float x1, float y1)
{
	glRectf(x0, y0, x1, y1);
}

static void
tri(float ax, float ay, float bx, float by, float cx, float cy)
{
	glBegin(GL_TRIANGLES);
	glVertex2f(ax, ay);
	glVertex2f(bx, by);
	glVertex2f(cx, cy);
	glEnd();
}

static void
line(float x0, float y0, float x1, float y1)
{
	glBegin(GL_LINES);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glEnd();
}

/* The quads of a frame, in two batches: the backgrounds, and the text
 * (textured from the atlas); each drawn at once */
struct vert {
	float x, y, s, t;
	unsigned char rgba[4];
};
static struct vert *bgv, *txv;
static size_t nbg, ntx, capbg, captx;

static int
quad(struct vert **v, size_t *n, size_t *cap, float x0, float y0, float x1, float y1,
    float s0, float t0, float s1, float t1, unsigned long c)
{
	struct vert *p, *nv;
	size_t nc;
	int i;

	if(*n + 4 > *cap) {
		nc = *cap ? *cap * 2 : 16384;
		if((nv = realloc(*v, nc * sizeof *nv)) == 0)
			return 0;
		*v = nv;
		*cap = nc;
	}
	p = *v + *n;
	p[0].x = x0; p[0].y = y0; p[0].s = s0; p[0].t = t0;
	p[1].x = x1; p[1].y = y0; p[1].s = s1; p[1].t = t0;
	p[2].x = x1; p[2].y = y1; p[2].s = s1; p[2].t = t1;
	p[3].x = x0; p[3].y = y1; p[3].s = s0; p[3].t = t1;
	for(i = 0; i < 4; i++) {
		p[i].rgba[0] = (unsigned char)(c >> 16);
		p[i].rgba[1] = (unsigned char)(c >> 8 & 0xFF);
		p[i].rgba[2] = (unsigned char)(c & 0xFF);
		p[i].rgba[3] = 255;
	}
	*n += 4;
	return 1;
}

static void
batch_draw(struct vert *v, size_t n, int textured)
{
	if(n == 0)
		return;
	if(textured) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, atlas);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof *v, &v->s);
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof *v, &v->x);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof *v, v->rgba);
	glDrawArrays(GL_QUADS, 0, (GLsizei)n);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	if(textured) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}
}

/* a character of the text, from the font, to the batch (0: the atlas
 * is full); a box, drawn at once, when the font has none */
static int
text_char(long c, float x, float y, unsigned long fg)
{
	struct glyph *g = glyph(c);
	float x0, y0;

	if(g == 0)
		return 0;
	if(!g->has) {
		glBegin(GL_LINE_LOOP);
		glVertex2f(x + 1.5f, y + cellh * 0.2f);
		glVertex2f(x + cellw - 1.5f, y + cellh * 0.2f);
		glVertex2f(x + cellw - 1.5f, y + cellh * 0.8f);
		glVertex2f(x + 1.5f, y + cellh * 0.8f);
		glEnd();
		return 1;
	}
	if(g->w == 0)
		return 1;	/* a blank */
	x0 = x + g->xoff;
	y0 = y + ascent + g->yoff;
	return quad(&txv, &ntx, &captx, x0, y0, x0 + g->w, y0 + g->h, g->s0, g->t0, g->s1, g->t1, fg);
}

/* a glyph of the interface: a shape that fills the cell to its edges */
static void
ui_glyph(int c, float x, float y)
{
	float w = (float)cellw, h = (float)cellh, cx = x + w / 2, cy = y + h / 2;
	float t = scale < 1.5f ? 1 : (float)(int)scale, a = w * 0.3f;

	switch(c) {
	case G_H:
		rect(x, cy - t / 2, x + w, cy + t / 2);
		break;
	case G_V:
		rect(cx - t / 2, y, cx + t / 2, y + h);
		break;
	case G_TL:
		rect(cx - t / 2, cy - t / 2, x + w, cy + t / 2);
		rect(cx - t / 2, cy - t / 2, cx + t / 2, y + h);
		break;
	case G_TR:
		rect(x, cy - t / 2, cx + t / 2, cy + t / 2);
		rect(cx - t / 2, cy - t / 2, cx + t / 2, y + h);
		break;
	case G_BL:
		rect(cx - t / 2, cy - t / 2, x + w, cy + t / 2);
		rect(cx - t / 2, y, cx + t / 2, cy + t / 2);
		break;
	case G_BR:
		rect(x, cy - t / 2, cx + t / 2, cy + t / 2);
		rect(cx - t / 2, y, cx + t / 2, cy + t / 2);
		break;
	case G_THUMB:
		rect(x, y, x + w, y + h);
		break;
	case G_SCROLLUP:
		tri(cx, cy - a, cx - a, cy + a * 0.6f, cx + a, cy + a * 0.6f);
		break;
	case G_SCROLLDOWN:
		tri(cx, cy + a, cx - a, cy - a * 0.6f, cx + a, cy - a * 0.6f);
		break;
	case G_SCROLLLEFT:
		tri(cx - a, cy, cx + a * 0.6f, cy - a, cx + a * 0.6f, cy + a);
		break;
	case G_SCROLLRIGHT:
		tri(cx + a, cy, cx - a * 0.6f, cy - a, cx - a * 0.6f, cy + a);
		break;
	case G_HTHUMB:		/* the band of the horizontal bar */
		rect(x, y + h / 3, x + w, y + h * 2 / 3);
		break;
	case G_HBANDR:
		rect(cx, y + h / 3, x + w, y + h * 2 / 3);
		break;
	case G_HBANDL:
		rect(x, y + h / 3, cx, y + h * 2 / 3);
		break;
	case G_CHECKED:		/* ✓ */
		glLineWidth(1.5f * scale);
		glBegin(GL_LINE_STRIP);
		glVertex2f(x + w * 0.15f, cy);
		glVertex2f(x + w * 0.4f, cy + h * 0.2f);
		glVertex2f(x + w * 0.85f, cy - h * 0.25f);
		glEnd();
		glLineWidth(1);
		break;
	case G_BOXON: case G_BOXOFF: {	/* ☑ ☐ */
		/* a square as tall as the capitals, over its cell and into the
		   blank after it (a check box is "[", the box, " ") */
		float s = h * 0.62f, bx, by, bt = t * 1.5f;

		if(s > w * 1.8f)
			s = w * 1.8f;
		bx = x + w * 0.15f;
		by = cy - s / 2;
		rect(bx, by, bx + s, by + bt);			/* the sides */
		rect(bx, by + s - bt, bx + s, by + s);
		rect(bx, by, bx + bt, by + s);
		rect(bx + s - bt, by, bx + s, by + s);
		if(c == G_BOXON) {	/* the tick, inside */
			glLineWidth(2 * (scale < 1 ? 1 : scale));
			glBegin(GL_LINE_STRIP);
			glVertex2f(bx + s * 0.22f, by + s * 0.52f);
			glVertex2f(bx + s * 0.43f, by + s * 0.73f);
			glVertex2f(bx + s * 0.78f, by + s * 0.28f);
			glEnd();
			glLineWidth(1);
		}
		break;
	}
	case G_WRAPDOT:		/* ∙ */
		rect(cx - t, cy - t, cx + t, cy + t);
		break;
	case G_ELLIPSIS:	/* … */
		rect(x + w * 0.15f, cy + h * 0.2f, x + w * 0.15f + t * 1.5f, cy + h * 0.2f + t * 1.5f);
		rect(cx - t * 0.75f, cy + h * 0.2f, cx + t * 0.75f, cy + h * 0.2f + t * 1.5f);
		rect(x + w * 0.85f - t * 1.5f, cy + h * 0.2f, x + w * 0.85f, cy + h * 0.2f + t * 1.5f);
		break;
	/* a tab: a line across its cells to an arrow in its last (a tab of
	   one cell: the arrow alone) */
	case G_TABVIEW:
		rect(x + w * 0.2f, cy - t / 2, x + w, cy + t / 2);
		rect(x + w * 0.2f, cy - h * 0.18f, x + w * 0.2f + t, cy + h * 0.18f);
		break;
	case G_TABLINE:
		rect(x, cy - t / 2, x + w, cy + t / 2);
		break;
	case G_SEPL:		/* ├ */
		rect(cx - t / 2, y, cx + t / 2, y + h);
		rect(cx - t / 2, cy - t / 2, x + w, cy + t / 2);
		break;
	case G_SEPR:		/* ┤ */
		rect(cx - t / 2, y, cx + t / 2, y + h);
		rect(x, cy - t / 2, cx + t / 2, cy + t / 2);
		break;
	case G_GUIDE:		/* an indent guide: a thin line down the cell */
		rect(cx - t / 2, y, cx + t / 2, y + h);
		break;
	case G_TABEND:
		rect(x, cy - t / 2, x + w * 0.55f, cy + t / 2);
		tri(x + w * 0.9f, cy, x + w * 0.45f, cy - h * 0.2f, x + w * 0.45f, cy + h * 0.2f);
		break;
	case G_EOLVIEW:		/* ↵ */
		line(x + w * 0.8f, cy - h * 0.2f, x + w * 0.8f, cy + h * 0.1f);
		line(x + w * 0.8f, cy + h * 0.1f, x + w * 0.2f, cy + h * 0.1f);
		line(x + w * 0.2f, cy + h * 0.1f, x + w * 0.4f, cy - w * 0.1f);
		line(x + w * 0.2f, cy + h * 0.1f, x + w * 0.4f, cy + h * 0.1f + w * 0.2f);
		break;
	case G_EOFVIEW:		/* ◆ */
		glBegin(GL_QUADS);
		glVertex2f(cx, cy - w * 0.35f);
		glVertex2f(cx + w * 0.35f, cy);
		glVertex2f(cx, cy + w * 0.35f);
		glVertex2f(cx - w * 0.35f, cy);
		glEnd();
		break;
	case G_CONTROL:	/* � */
		glBegin(GL_QUADS);
		glVertex2f(cx, cy - w * 0.45f);
		glVertex2f(cx + w * 0.45f, cy);
		glVertex2f(cx, cy + w * 0.45f);
		glVertex2f(cx - w * 0.45f, cy);
		glEnd();
		break;
	}
}

/* XCEDIT_SNAPSHOT=file.ppm: each frame drawn, also as an image (to see
 * what the window shows, where nothing can capture it) */
static void
snapshot(int fw, int fh)
{
	const char *path = getenv("XCEDIT_SNAPSHOT");
	unsigned char *px;
	FILE *f;
	int y;

	if(path == 0 || (px = malloc((size_t)fw * fh * 3)) == 0)
		return;
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, fw, fh, GL_RGB, GL_UNSIGNED_BYTE, px);
	if((f = fopen(path, "wb")) != 0) {
		fprintf(f, "P6\n%d %d\n255\n", fw, fh);
		for(y = fh - 1; y >= 0; y--)	/* GL's rows are from the bottom */
			fwrite(px + (size_t)y * fw * 3, 3, (size_t)fw, f);
		fclose(f);
	}
	free(px);
}

/* the looks of cell i: its colours */
static void
looks(int i, unsigned long *fg, unsigned long *bg)
{
	int at = gat[i], s = STYLE(at);
	unsigned long t;

	*fg = sfg[s];
	*bg = sbg[s];
	if(at & F_DIM) {	/* greyed (on a float: the text alone, an item
				   that does not apply) */
		*fg = colours_blend(*fg, s == S_FLOAT ? *bg : pal[BG], 1, 2);
		if(s != S_FLOAT)
			*bg = colours_blend(*bg, pal[BG], 1, 2);
	}
	if(at & F_REV) {
		t = *fg;
		*fg = *bg;
		*bg = t;
	}
	if(at & F_HOVER)	/* the mouse over it: a tint of the text's colour */
		*bg = colours_blend(*fg, *bg, 1, 5);
}

/* The frame: the backgrounds (a quad for each run of one colour), the
 * shapes of the interface, the text (one draw from the atlas), the cursor.
 * 0 if the atlas was full: emptied, to be drawn again. */
static int
frame(int fw, int fh)
{
	int i, j, k, at, low;
	unsigned long fg, bg, rbg;
	float x, y;
	cell_t c;

	glViewport(0, 0, fw, fh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, fw, fh, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor((pal[BG] >> 16) / 255.0f, (pal[BG] >> 8 & 0xFF) / 255.0f,
	    (pal[BG] & 0xFF) / 255.0f, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	nbg = ntx = 0;
	for(j = 0; j < rows; j++)	/* the backgrounds, by runs */
		for(i = 0; i < cols; i = k) {
			looks(j * cols + i, &fg, &rbg);
			low = j == lowy && i >= lowx1 && i <= lowx2;
			for(k = i + 1; k < cols; k++) {
				if(j == lowy && (k == lowx1 || k == lowx2 + 1))
					break;	/* the cells drawn lower: apart */
				looks(j * cols + k, &fg, &bg);
				if(bg != rbg)
					break;
			}
			if(low) {
				/* under them, the colour without the reversed
				   (the float's) */
				looks(j * cols + i, &fg, &bg);
				if(gat[j * cols + i] & F_REV)
					bg = fg;
				quad(&bgv, &nbg, &capbg, (float)(i * cellw), (float)(j * cellh),
				    (float)(k * cellw), (float)((j + 1) * cellh), 0, 0, 0, 0, bg);
			} else
				quad(&bgv, &nbg, &capbg, (float)(i * cellw), (float)(j * cellh),
				    (float)(k * cellw), (float)((j + 1) * cellh), 0, 0, 0, 0, rbg);
		}
	/* the cells drawn lower, theirs over the top half of the row under
	   them (its line, in its middle, stays clear) */
	if(lowy >= 0 && lowy < rows)
		for(i = lowx1 < 0 ? 0 : lowx1; i <= lowx2 && i < cols; i++) {
			looks(lowy * cols + i, &fg, &rbg);
			quad(&bgv, &nbg, &capbg, (float)(i * cellw), (lowy + LOWER) * cellh,
			    (float)((i + 1) * cellw), (lowy + 1 + LOWER) * cellh, 0, 0, 0, 0, rbg);
		}
	batch_draw(bgv, nbg, 0);
	for(j = 0; j < rows; j++)
		for(i = 0; i < cols; i++) {
			c = gch[j * cols + i];
			if(c == CELL_WIDE || c == ' ')
				continue;
			at = gat[j * cols + i];
			looks(j * cols + i, &fg, &bg);
			x = (float)(i * cellw);
			y = (float)(j * cellh);
			if(j == lowy && i >= lowx1 && i <= lowx2)
				y += cellh * LOWER;
			if(c < G_LAST) {	/* shapes: drawn now */
				/* the marks of tabs, line ends and the end: half of
				   the colour of the text, as marks are */
				colour(c == G_TABVIEW || c == G_TABLINE || c == G_TABEND ?
				    colours_blend(fg, bg, 1, 2) : fg);
				ui_glyph((int)c, x, y);
			} else if(!text_char((long)c, x, y, fg)) {
				glyphs_flush();
				return 0;
			}
			if(at & F_UNDER)
				quad(&txv, &ntx, &captx, x, y + ascent + scale, x + cellw,
				    y + ascent + 2 * scale, 0, 0, 0, 0, fg);
		}
	batch_draw(txv, ntx, 1);
	if(curon && curx >= 0 && curx < cols && cury >= 0 && cury < rows) {
		/* a bar, as editors have it, in the colour of the text of its
		   cell (dark on a light panel, light on the text) */
		looks(cury * cols + curx, &fg, &bg);
		colour(fg);
		x = (float)(curx * cellw);
		y = (float)(cury * cellh);
		rect(x, y, x + (scale < 1.5f ? 2 : 2 * (int)scale), y + cellh);
	}
	return 1;
}

static void
draw(void)
{
	int fw, fh, c, r;

	if(win == 0 || gch == 0 || glfwGetWindowAttrib(win, GLFW_ICONIFIED))
		return;		/* minimized: drawn when shown again */
	glfwGetFramebufferSize(win, &fw, &fh);
	grid_dims(fw, fh, &c, &r);
	if(c != cols || r != rows) {
		/* a size the grid has not taken yet: the frame of the old one
		   would be broken, the editor draws it again first */
		push_resize();
		return;
	}
	if(!frame(fw, fh))
		frame(fw, fh);	/* the atlas was full: emptied */
	snapshot(fw, fh);
	glfwSwapBuffers(win);
	dirty = 0;
	/* the first frame: the surface takes its size at this swap (Wayland),
	   so it is drawn again, at that size */
	if(lastfw == 0) {
		lastfw = fw;
		dirty = 1;
	}
}

/* --- the terminal the editor sees ---------------------------------------- */

/* the cells of a framebuffer of fw by fh pixels (a few at least) */
static void
grid_dims(int fw, int fh, int *c, int *r)
{
	*c = fw / cellw;
	*r = fh / cellh;
	if(*c < 20)
		*c = 20;
	if(*r < 5)
		*r = 5;
}

static int
grid(void)
{
	int fw, fh, c, r;
	cell_t *nc;
	attr_t *na;

	glfwGetFramebufferSize(win, &fw, &fh);
	grid_dims(fw, fh, &c, &r);
	if(gch && c == cols && r == rows)
		return 1;
	nc = calloc((size_t)c * r, sizeof *nc);
	na = calloc((size_t)c * r, sizeof *na);
	if(nc == 0 || na == 0) {
		free(nc);
		free(na);
		return gch != 0;
	}
	for(fw = 0; fw < c * r; fw++)
		nc[fw] = ' ';
	free(gch);
	free(gat);
	gch = nc;
	gat = na;
	cols = c;
	rows = r;
	return 1;
}

/* --- the icon of the window (X11: Wayland takes that of xcedit.desktop) -- */

/* (x, y) inside the polygon p of n corners (turned: >) */
static int
inside(const short (*p)[2], int n, int turned, float x, float y)
{
	int i, j, in = 0;
	float xi, yi, xj, yj;

	for(i = 0, j = n - 1; i < n; j = i++) {
		xi = turned ? ICON_SIZE + 1 - p[i][0] : p[i][0];
		yi = turned ? ICON_SIZE - p[i][1] : p[i][1];
		xj = turned ? ICON_SIZE + 1 - p[j][0] : p[j][0];
		yj = turned ? ICON_SIZE - p[j][1] : p[j][1];
		if((yi > y) != (yj > y) && x < (xj - xi) * (y - yi) / (yj - yi) + xi)
			in = !in;
	}
	return in;
}

/* the colour of the icon at (x, y) in its units, alpha 0 off the square */
static unsigned long
icon_at(float x, float y, int *alpha)
{
	float r = ICON_RADIUS, cx, cy, d;
	unsigned long a = ICON_BG_IN, b = ICON_BG_OUT, c = 0;
	int k;

	cx = x < r ? r : x > ICON_SIZE - r ? ICON_SIZE - r : x;
	cy = y < r ? r : y > ICON_SIZE - r ? ICON_SIZE - r : y;
	*alpha = (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r;
	if(inside(icon_lt, ICON_LT_N, 0, x, y) || inside(icon_lt, ICON_LT_N, 1, x, y))
		return ICON_BRACKETS;
	if(inside(icon_slash, ICON_SLASH_N, 0, x, y))
		return ICON_SLASH;
	d = (float)((x * x + y * y) / ((float)ICON_SIZE * ICON_SIZE));	/* the gradient */
	d = d > 1 ? 1 : d;
	for(k = 0; k < 24; k += 8)
		c |= (unsigned long)((float)(a >> k & 0xFF) * (1 - d) + (float)(b >> k & 0xFF) * d) << k;
	return c;
}

/* the icon drawn into pixels, 4 by 4 samples in each */
static void
icon_pixels(unsigned char *px, int size)
{
	int x, y, i, j, al, n;
	unsigned long c, sum[4];
	float s = (float)ICON_SIZE / size;

	for(y = 0; y < size; y++)
		for(x = 0; x < size; x++) {
			sum[0] = sum[1] = sum[2] = sum[3] = 0;
			for(j = 0; j < 4; j++)
				for(i = 0; i < 4; i++) {
					c = icon_at((x + (i + 0.5f) / 4) * s, (y + (j + 0.5f) / 4) * s, &al);
					if(!al)
						continue;
					sum[0] += c >> 16;
					sum[1] += c >> 8 & 0xFF;
					sum[2] += c & 0xFF;
					sum[3] += 255;
				}
			n = (int)(sum[3] / 255);
			px[0] = (unsigned char)(n ? sum[0] / n : 0);
			px[1] = (unsigned char)(n ? sum[1] / n : 0);
			px[2] = (unsigned char)(n ? sum[2] / n : 0);
			px[3] = (unsigned char)(sum[3] / 16);
			px += 4;
		}
}

static void
window_icon(void)
{
	static unsigned char p64[64 * 64 * 4], p32[32 * 32 * 4];
	GLFWimage im[2];

	icon_pixels(p64, 64);
	icon_pixels(p32, 32);
	im[0].width = im[0].height = 64;
	im[0].pixels = p64;
	im[1].width = im[1].height = 32;
	im[1].pixels = p32;
	glfwSetWindowIcon(win, 2, im);
}

/* XCEDIT_KEYS: keys typed at the start, for tests (~X is Alt+X, ^X Ctrl+X,
 * | Enter, ! Escape, <Down> <S-Right> <C-Space> <A-S-Up> a named key with
 * its modifiers, {x,y} a click at a cell, {a:x,y} one with Alt, {x,y>x2,y2}
 * a drag, {u:x,y} {d:x,y} the wheel) */
static void
test_keys(void)
{
	const char *p = getenv("XCEDIT_KEYS");

	for(; p && *p; p++) {
		if(*p == '<') {	/* <Down>, <S-Right>, <C-A-Up>, <C-Space>: a key */
			static const struct { const char *name; kcode k; } names[] = {
				{ "Up", K_UP }, { "Down", K_DOWN }, { "Left", K_LEFT },
				{ "Right", K_RIGHT }, { "Home", K_HOME }, { "End", K_END },
				{ "PgUp", K_PGUP }, { "PgDn", K_PGDN }, { "Del", K_DEL },
				{ "BS", 8 }, { "Tab", 9 }, { "Space", ' ' }, { "F3", K_F3 },
				{ "Bar", '|' }, { "Lt", '<' },
			};
			kcode m = 0;
			const char *e = strchr(p, '>');
			size_t n, i;

			if(e == 0)
				break;
			for(p++; p[1] == '-' && strchr("SCA", *p); p += 2)
				m |= *p == 'S' ? K_SHIFT : *p == 'C' ? K_CTRL : K_ALT;
			n = (size_t)(e - p);
			for(i = 0; i < sizeof names / sizeof names[0]; i++)
				if(strlen(names[i].name) == n && strncmp(names[i].name, p, n) == 0)
					break;
			if(i < sizeof names / sizeof names[0])
				push(names[i].k | m);
			else if(n == 1)
				push((kcode)(*p >= 'a' && *p <= 'z' && (m & (K_CTRL | K_ALT)) ? *p - 32 : *p) | m);
			p = e;
			continue;
		}
		if(*p == '{') {	/* {x,y} a click at a cell, {x,y>x2,y2} a drag;
				   {a:x,y} with Alt held */
			int x1, y1, x2 = -1, y2 = -1, m = 0;

			if(p[1] == 'a' && p[2] == ':') {
				m = GLFW_MOD_ALT;
				p += 2;
			} else if((p[1] == 'u' || p[1] == 'd') && p[2] == ':') {
				/* {u:x,y} {d:x,y}: the wheel, up or down */
				int up = p[1] == 'u';

				if(sscanf(p + 3, "%d,%d", &x1, &y1) < 2)
					break;
				mx = x1;
				my = y1;
				mouse_event(up ? M_WHEELUP : M_WHEELDOWN, MB_LEFT, 0);
				while(*p && *p != '}')
					p++;
				if(!*p)
					break;
				continue;
			}
			if(sscanf(p + 1, "%d,%d>%d,%d", &x1, &y1, &x2, &y2) < 2)
				break;
			mx = x1;
			my = y1;
			term_mouse.clicks = 1;
			mouse_event(M_PRESS, MB_LEFT, m);
			if(x2 >= 0) {
				mx = x2;
				my = y2;
				mouse_event(M_DRAG, MB_LEFT, m);
			}
			mouse_event(M_RELEASE, MB_LEFT, m);
			while(*p && *p != '}')
				p++;
			if(!*p)
				break;
			continue;
		}
		if((*p == '~' || *p == '^') && p[1]) {
			/* as the keyboard gives it (^V: the clipboard too) */
			mod_key(p[1] >= 'a' && p[1] <= 'z' ? p[1] - 32 : p[1], *p == '~' ? K_ALT : K_CTRL);
			p++;
		} else
			push(*p == '|' ? 13 : *p == '!' ? 27 : (kcode)(unsigned char)*p);
	}
}

/* --- the terminal it was started from ------------------------------------ */

static int stay;		/* the terminal waits for the window to close */
static int started = -1;	/* the pipe to the terminal's process, until
				   the window is open */

void
term_stay(int on)
{
	stay = on;
}

/* started from a terminal: the editor goes on in a process of its own,
 * apart from the terminal (closing it does not close the window); the
 * terminal's process ends when the window is open, giving the prompt
 * back, or with the status of the editor when it could not open one (what
 * it said, said in the terminal).  Before any thread or window is made. */
static void
background(void)
{
	pid_t pid;
	int p[2], status;
	char c;

	if(stay || !isatty(0) || pipe(p) < 0)
		return;
	if((pid = fork()) < 0) {
		close(p[0]);
		close(p[1]);
		return;
	}
	if(pid > 0) {	/* the terminal's: until the window is open */
		close(p[1]);
		if(read(p[0], &c, 1) == 1)
			_exit(0);
		while(waitpid(pid, &status, 0) < 0 && errno == EINTR)
			;
		_exit(WIFEXITED(status) ? WEXITSTATUS(status) : 1);
	}
	close(p[0]);
	setsid();
	started = p[1];
}

/* the window open: the terminal let go */
static void
opened(void)
{
	int fd;

	if(started < 0)
		return;
	if((fd = open("/dev/null", O_RDWR)) >= 0) {
		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
		if(fd > 2)
			close(fd);
	}
	/* (the terminal's process gone: nothing to tell) */
	while(write(started, "1", 1) < 0 && errno == EINTR)
		;
	close(started);
	started = -1;
}

int
term_open(int *c, int *r)
{
	float sx = 1, sy = 1;
	GLFWmonitor *mon;

	background();
	/* GIO of local files alone: on GNOME the title bar is GTK's (libdecor),
	   and GTK asks the gvfs daemons (gvfsd-metadata, ...) as it starts; one
	   of them that hangs would hold the window from opening (and git commit
	   waiting for it).  The editor reads its files itself: it needs none
	   of them.  Unless the environment says otherwise */
	setenv("GIO_USE_VFS", "local", 0);
	glfwSetErrorCallback(error_cb);
	if(!glfwInit())
		return 0;
	if(!font_ready && term_set_font("") != FONT_OK)
		return 0;
#ifdef GLFW_WAYLAND_APP_ID
	glfwWindowHintString(GLFW_WAYLAND_APP_ID, "xcedit");	/* xcedit.desktop */
#endif
#ifdef GLFW_X11_CLASS_NAME
	glfwWindowHintString(GLFW_X11_CLASS_NAME, "xcedit");
	glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "xcedit");
#endif
	/* made at its size: the cells are in pixels of the framebuffer, the
	   size of a window in its own units (pixels on X11, on Wayland those
	   of a scale of 1: the pixels divided by the scale) */
	if((mon = glfwGetPrimaryMonitor()) != 0)
		glfwGetMonitorContentScale(mon, &sx, &sy);
	scale = sy > 0.5f ? sy : 1;
	font_size();
	sx = wayland() ? scale : 1;
	win = glfwCreateWindow((int)(START_COLS * cellw / sx), (int)(START_ROWS * cellh / sx),
	    "Code Editor", 0, 0);
	if(win == 0) {
		glfwTerminate();
		return 0;
	}
	/* the events first: a size the window manager gives it (now or
	   later, as Wayland does) is seen */
	glfwSetKeyCallback(win, key_cb);
	glfwSetWindowFocusCallback(win, focus_cb);
	glfwSetCharModsCallback(win, char_cb);	/* with Alt too: the char callback is not */
	glfwSetMouseButtonCallback(win, button_cb);
	glfwSetCursorPosCallback(win, pos_cb);
	glfwSetCursorEnterCallback(win, enter_cb);
	glfwSetScrollCallback(win, scroll_cb);
	glfwSetFramebufferSizeCallback(win, size_cb);
	glfwSetWindowSizeCallback(win, size_cb);
	glfwSetWindowRefreshCallback(win, refresh_cb);
	glfwSetWindowCloseCallback(win, close_cb);
	touch_open();
	glfwSetCursor(win, glfwCreateStandardCursor(GLFW_IBEAM_CURSOR));
	if(!wayland())	/* Wayland takes the icon of xcedit.desktop */
		window_icon();
	glfwMakeContextCurrent(win);
	/* Wayland: no waiting for the compositor at a swap (it composes the
	   frames itself; a window out of sight would get no frame and hold
	   the editor, not answering) */
	glfwSwapInterval(wayland() ? 0 : 1);
	glfwGetWindowContentScale(win, &sx, &sy);	/* that of its screen */
	if(sy > 0.5f && sy != scale) {
		scale = sy;
		font_size();
	}
	glfwPollEvents();	/* its size, as the window manager gives it */
	glyphs_flush();
	colours_styles();	/* the default palette */
	if(!grid())
		return 0;
	qhead = qtail = 0;	/* the size is the grid's now */
	resize_queued = 0;
	test_keys();
	*c = cols;
	*r = rows;
	opened();
	return 1;
}

void
term_close(void)
{
	if(win == 0)
		return;
	glyphs_flush();
	glfwDestroyWindow(win);
	glfwTerminate();
	win = 0;
}

int
term_size(int *c, int *r)
{
	if(!grid())
		return 0;
	*c = cols;
	*r = rows;
	return 1;
}

void
term_write(int x, int y, const cell_t *ch, const attr_t *at, int n)
{
	if(y < 0 || y >= rows || x < 0)
		return;
	if(x + n > cols)
		n = cols - x;
	if(n <= 0)
		return;
	/* the same as shown (a row put again as it was: what a modal over the
	   text does at each pass): nothing to draw */
	if(memcmp(gch + y * cols + x, ch, (size_t)n * sizeof *ch) == 0 &&
	   memcmp(gat + y * cols + x, at, (size_t)n * sizeof *at) == 0)
		return;
	memcpy(gch + y * cols + x, ch, (size_t)n * sizeof *ch);
	memcpy(gat + y * cols + x, at, (size_t)n * sizeof *at);
	dirty = 1;
}

void
term_cursor(int x, int y, int visible)
{
	if(x != curx || y != cury || visible != curon)
		dirty = 1;
	curx = x;
	cury = y;
	curon = visible;
}

/* drawn once for each key (term_key, term_keyready), when changed: one
 * swap of the buffers, not one for each flush */
void
term_flush(void)
{
}

kcode
term_key(void)
{
	struct ev *e;
	double until = glfwGetTime() + 1.0, left;

	while(qhead == qtail) {
		if(dirty)
			draw();
		if(dirty)	/* drawn again at once: a new size */
			glfwPollEvents();
		else {
			/* the window has events of its own (a frame shown, the
			   focus): only a key, or a second gone, ends the wait */
			if((left = until - glfwGetTime()) <= 0)
				return K_NONE;	/* idle a second */
			glfwWaitEventsTimeout(left);
		}
	}
	e = &q[qhead];
	qhead = (qhead + 1) % QMAX;
	term_mouse = e->m;
	if(e->k == K_RESIZE)
		resize_queued = 0;
	return e->k;
}

int
term_keyready(void)
{
	if(dirty)
		draw();
	glfwPollEvents();
	return qhead != qtail;
}

/* the sizes of the zoom, in pixels at a scale of 1 */
static const int zooms[] = {
	8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 22, 24, 26, 28, 32, 36, 40, 48, 56, 64, 72
};
#define NZOOMS	((int)(sizeof zooms / sizeof zooms[0]))

int
term_font_size(void)
{
	return fontpx;
}

void
term_set_font_size(int px)
{
	fontpx = px >= zooms[0] && px <= zooms[NZOOMS - 1] ? px : FONT_PX;
	if(win) {
		font_size();
		glyphs_flush();
		push_resize();
	}
}

int
term_zoom(int how)
{
	int i, px = fontpx;

	if(how == 0)
		px = FONT_PX;
	else if(how > 0) {
		for(i = 0; i < NZOOMS && zooms[i] <= fontpx; i++)
			;
		if(i < NZOOMS)
			px = zooms[i];
	} else {
		for(i = NZOOMS - 1; i >= 0 && zooms[i] >= fontpx; i--)
			;
		if(i >= 0)
			px = zooms[i];
	}
	if(px == fontpx)
		return 0;
	term_set_font_size(px);
	return 1;
}

void
term_lower(int x1, int x2, int y)
{
	if(x1 == lowx1 && x2 == lowx2 && y == lowy)
		return;
	lowx1 = x1;
	lowx2 = x2;
	lowy = y;
	dirty = 1;
}

void
term_beep(void)
{
}

void
term_title(const char *name, int dirty)
{
	static char last[300];
	char t[300];

	snprintf(t, sizeof t, "%s%s - Code Editor", dirty ? "\xe2\x97\x8f " : "", name);
	if(strcmp(t, last) == 0)
		return;
	strcpy(last, t);
	glfwSetWindowTitle(win, t);
}

const char *
term_paste(size_t *n)
{
	*n = pastelen;
	return pastebuf;
}

void
term_clipboard(const char *s, size_t n)
{
	char *p = malloc(n + 1);

	if(p == 0)
		return;
	memcpy(p, s, n);
	p[n] = 0;
	glfwSetClipboardString(win, p);
	free(lastclip);
	lastclip = p;	/* what the editor copied: Ctrl+V keeps its lines */
}
