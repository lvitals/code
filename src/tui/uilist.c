/* The list widget. */
#include "ui.h"

static int sbdrag = -1;	/* the list whose scrollbar is being dragged */

int
ui_list(int id, int x, int y, int w, int h, int n,
    const char *(*item)(int i), int *sel, int *top)
{
	int f = ui_focusable(id, W_LIST, x, y, x + w - 1), s = *sel, t = *top, r = 0, i, a, used = 1;
	kcode k = ui_key;

	if(s >= n)
		s = n - 1;
	if(s < 0)
		s = 0;
	if(f && n && k) {
		if(k == K_UP)
			s = s ? s - 1 : n - 1;
		else if(k == K_DOWN)
			s = s + 1 < n ? s + 1 : 0;
		else if(k == K_PGUP)
			s = s > h - 1 ? s - (h - 1) : 0;
		else if(k == K_PGDN)
			s = s + h - 1 < n ? s + h - 1 : n - 1;
		else if(k == K_HOME)
			s = 0;
		else if(k == K_END)
			s = n - 1;
		else if(k == 13)
			r = 2;
		else
			used = 0;
		if(used) {
			ui_key = 0;
			ui_again = 1;
		}
	}
	/* its scrollbar (the last column, when not all is shown): an arrow
	   scrolls a row, a click on the track or a drag puts the thumb there
	   (off the bar too, until let go); the selection stays in sight */
	if(k == K_MOUSE && term_mouse.kind == M_RELEASE && sbdrag == id)
		sbdrag = -1;
	if(k == K_MOUSE && ui_mouse_ok() && n > h && h >= 3 &&
	   ((term_mouse.kind == M_PRESS && term_mouse.button == MB_LEFT && term_mouse.x == x + w - 1 &&
	     term_mouse.y >= y && term_mouse.y < y + h) || (term_mouse.kind == M_DRAG && sbdrag == id))) {
		int row = term_mouse.y - y;

		if(term_mouse.kind == M_PRESS && row == 0)
			t--;
		else if(term_mouse.kind == M_PRESS && row == h - 1)
			t++;
		else {
			sbdrag = id;
			t = (int)ui_scrollbar_at(h, n, h, row < 1 ? 1 : row > h - 2 ? h - 2 : row);
		}
		if(t > n - h)
			t = n - h;
		if(t < 0)
			t = 0;
		if(s < t)
			s = t;
		if(s >= t + h)
			s = t + h - 1;
		ui_steal(id);
		ui_key = 0;
		ui_again = 1;
	} else if(k == K_MOUSE && ui_mouse_ok() && term_mouse.x >= x && term_mouse.x < x + w &&
	   term_mouse.y >= y && term_mouse.y < y + h) {
		if((term_mouse.kind == M_WHEELUP || term_mouse.kind == M_WHEELDOWN) && n) {
			/* the wheel moves the selection, a row a notch (the list
			   following it) */
			s += term_mouse.kind == M_WHEELUP ? -1 : 1;
			if(s < 0)
				s = 0;
			if(s >= n)
				s = n - 1;
			ui_steal(id);
			if(s < t)
				t = s;
			if(s >= t + h)
				t = s - h + 1;
		} else if(term_mouse.kind == M_PRESS && term_mouse.button == MB_LEFT &&
		    t + term_mouse.y - y < n) {
			s = t + term_mouse.y - y;
			ui_steal(id);
			if(term_mouse.clicks == 2)
				r = 2;
		}
		ui_key = 0;
		ui_again = 1;
	} else
	{
		if(s < t)
			t = s;
		if(s >= t + h)
			t = s - h + 1;
	}
	if(t > n - h)
		t = n - h;
	if(t < 0)
		t = 0;
	if(s != *sel && r == 0)
		r = 1;
	*sel = s;
	*top = t;
	for(i = 0; i < h; i++) {
		a = t + i == s && f ? S_FOCUS : S_LIST;
		scr_fill(x, y + i, x + w - 1, y + i, ' ', a);
		if(t + i < n) {
			scr_put(x, y + i, t + i == s ? '>' : ' ', a);
			ui_trunc(x + 2, y + i, item(t + i), a, x + w - (n > h ? 1 : 0));
		}
	}
	ui_scrollbar_again();	/* the rows above were filled over it */
	ui_scrollbar(x + w - 1, y, h, t, n, h, 0);
	return r;
}
