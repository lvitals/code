#include <string.h>
#include "ui.h"

kcode ui_key;
int ui_focus, ui_cx, ui_cy, ui_again, ui_nograb;
const char *ui_clip;
size_t ui_cliplen;

static int seen;			/* a widget had the focus */
static int depth, inside, nmodal;	/* the modals of this pass, of the last */

#define WELL_MAX	24
static struct {
	int id[WELL_MAX];
	char kind[WELL_MAX];
	int n, grab;
} well;

void
ui_pass(kcode key)
{
	ui_key = key == K_NONE ? 0 : key;
	ui_cx = ui_cy = -1;
	ui_again = 0;
	seen = 0;
	depth = inside = 0;
	well.n = 0;
}

int
ui_end(int deflt)
{
	if(!seen && ui_focus != deflt) {
		ui_focus = deflt;
		ui_again = 1;
	}
	nmodal = depth;
	return ui_again;
}

void
ui_steal(int id)
{
	if(ui_focus != id) {
		ui_focus = id;
		ui_again = 1;
	}
	seen = 1;
}

int
ui_mouse_ok(void)
{
	return nmodal == 0 || inside == nmodal;
}

int
ui_mouse(int x1, int y, int x2)
{
	return ui_key == K_MOUSE && term_mouse.kind == M_PRESS && term_mouse.button == MB_LEFT &&
	    term_mouse.y == y &&
	    term_mouse.x >= x1 && term_mouse.x <= x2 && ui_mouse_ok();
}

int
ui_utf8(rune c, char *s)
{
	if(c < 0x80) {
		s[0] = (char)c;
		return 1;
	}
	if(c < 0x800) {
		s[0] = (char)(0xC0 | c >> 6);
		s[1] = (char)(0x80 | (c & 0x3F));
		return 2;
	}
	if(c < 0x10000) {
		s[0] = (char)(0xE0 | c >> 12);
		s[1] = (char)(0x80 | (c >> 6 & 0x3F));
		s[2] = (char)(0x80 | (c & 0x3F));
		return 3;
	}
	s[0] = (char)(0xF0 | c >> 18);
	s[1] = (char)(0x80 | (c >> 12 & 0x3F));
	s[2] = (char)(0x80 | (c >> 6 & 0x3F));
	s[3] = (char)(0x80 | (c & 0x3F));
	return 4;
}

/* --- the focus ----------------------------------------------------------- */

void
ui_well_begin(int grab)
{
	well.n = 0;
	well.grab = grab;
}

int
ui_focusable(int id, int kind, int x1, int y, int x2)
{
	if(well.n < WELL_MAX) {
		well.id[well.n] = id;
		well.kind[well.n++] = (char)kind;
	}
	if(ui_mouse(x1, y, x2) && ui_focus != id) {
		ui_focus = id;
		ui_again = 1;
	}
	if(ui_focus == id)
		seen = 1;
	return ui_focus == id;
}

int
ui_well_end(void)
{
	int i, j, n = well.n;
	kcode k = ui_key;

	for(i = 0; i < n && well.id[i] != ui_focus; i++)
		;
	if(i == n) {
		if(!well.grab || n == 0)
			return 0;
		ui_steal(well.id[0]);
		return 1;
	}
	j = i;
	if(k == 9 || (k == K_RIGHT && well.kind[i] == W_BUTTON))
		j = (i + 1) % n;
	else if(k == (K_SHIFT | 9) || (k == K_LEFT && well.kind[i] == W_BUTTON))
		j = (i + n - 1) % n;
	else
		return 1;
	ui_key = 0;
	ui_steal(well.id[j]);
	return 1;
}

/* --- the widgets --------------------------------------------------------- */

int
ui_accel(int x, int y, const char *s, int accel, int at, int lim)
{
	int i, n = (int)strlen(s), k = -1, cx;
	char a[2];

	for(i = 0; accel && i < n; i++) {
		if(s[i] == accel) {
			k = i;
			break;
		}
		if(k < 0 && (s[i] & ~0x20) == accel)
			k = i;
	}
	if(k < 0) {
		cx = scr_text(x, y, s, at, lim);
		if(!accel)
			return cx;
		cx = scr_text(cx, y, "(", at, lim);
		a[0] = (char)accel;
		a[1] = 0;
		cx = scr_text(cx, y, a, at | F_UNDER, lim);
		return scr_text(cx, y, ")", at, lim);
	}
	cx = scr_ntext(x, y, s, k, at, lim);
	cx = scr_ntext(cx, y, s + k, 1, at | F_UNDER, lim);
	return scr_text(cx, y, s + k + 1, at, lim);
}

static int
pressed(int f, int x1, int y, int x2)
{
	if((f && (ui_key == 13 || ui_key == ' ')) || ui_mouse(x1, y, x2)) {
		ui_key = 0;
		ui_again = 1;
		return 1;
	}
	return 0;
}

int
ui_button(int id, int x, int y, const char *s, int accel, int at)
{
	int w = text_width(s) + 2 + (accel && !strchr(s, accel) && !strchr(s, accel | 0x20) ? 3 : 0);
	int f = ui_focusable(id, W_BUTTON, x, y, x + w - 1), a = f ? at | F_REV : at;

	scr_put(x, y, '[', a);
	scr_put(ui_accel(x + 1, y, s, accel, a, scr_cols), y, ']', a);
	return pressed(f, x, y, x + w - 1);
}

int
ui_check(int id, int x, int y, const char *s, char *v, int at)
{
	int w = text_width(s) + 5;
	int f = ui_focusable(id, W_BUTTON, x, y, x + w - 1), a = f ? at | F_REV : at;

	/* [, the box, two blanks (a window draws the box wider, into the
	   first), the label, ] */
	scr_put(x, y, '[', a);
	scr_put(x + 1, y, *v ? G_BOXON : G_BOXOFF, a);
	scr_put(x + 2, y, ' ', a);
	scr_put(x + 3, y, ' ', a);
	scr_put(scr_text(x + 4, y, s, a, scr_cols), y, ']', a);
	if(pressed(f, x, y, x + w - 1)) {
		*v = !*v;
		return 1;
	}
	return 0;
}

int
ui_trunc(int x, int y, const char *s, int at, int lim)
{
	int cx;

	if(x + text_width(s) <= lim)
		return scr_text(x, y, s, at, lim);
	cx = scr_text(x, y, s, at, lim - 1);
	scr_put(cx, y, G_ELLIPSIS, at);
	return cx + 1;
}

/* --- the edit line ------------------------------------------------------- */

static int
enext(struct edl *e, int i)
{
	int j = i + 1;

	while(j < e->len && (e->s[j] & 0xC0) == 0x80)
		j++;
	return j > e->len ? e->len : j;
}

static int
eprev(struct edl *e, int i)
{
	int j = i - 1;

	while(j > 0 && (e->s[j] & 0xC0) == 0x80)
		j--;
	return j < 0 ? 0 : j;
}

static void
einsert(struct edl *e, const char *s, int n)
{
	int k = n, i;

	for(i = 0; i < n; i++)
		if(s[i] == '\r' || s[i] == '\n') {
			k = i;	/* the first line */
			break;
		}
	if(e->len + k >= EDL_MAX)
		k = EDL_MAX - 1 - e->len;
	if(k <= 0)
		return;
	memmove(e->s + e->cur + k, e->s + e->cur, e->len - e->cur);
	memcpy(e->s + e->cur, s, k);
	e->len += k;
	e->cur += k;
	e->s[e->len] = 0;
}

static void
edelete(struct edl *e, int from, int to)
{
	memmove(e->s + from, e->s + to, e->len - to);
	e->len -= to - from;
	e->cur = from;
	e->s[e->len] = 0;
}

void
edl_set(struct edl *e, const char *s)
{
	size_t n = strlen(s);

	if(n >= EDL_MAX)
		n = EDL_MAX - 1;
	memcpy(e->s, s, n);
	e->s[n] = 0;
	e->len = e->cur = (int)n;
	e->off = 0;
}

static int
ewidth(struct edl *e, int from, int to)
{
	char c = e->s[to];
	int w;

	e->s[to] = 0;
	w = text_width(e->s + from);
	e->s[to] = c;
	return w;
}

int
ui_edit(int id, int x, int y, int w, struct edl *e, int at)
{
	int f = ui_focusable(id, W_EDIT, x, y, x + w - 1), changed = 0, i;
	kcode k = ui_key, c;
	const char *p;
	size_t n;
	char u[4];

	if(ui_mouse(x, y, x + w - 1)) {
		for(i = e->off; i < e->len && x + ewidth(e, e->off, enext(e, i)) <= term_mouse.x; i = enext(e, i))
			;
		e->cur = i;
		ui_key = 0;
		k = 0;
	}
	if(f && k) {
		c = K_CODE(k);
		ui_key = 0;
		if(k == 8) {
			if(e->cur > 0) {
				edelete(e, eprev(e, e->cur), e->cur);
				changed = 1;
			}
		} else if(k == K_DEL) {
			if(e->cur < e->len) {
				i = e->cur;
				edelete(e, i, enext(e, i));
				changed = 1;
			}
		} else if(k == K_LEFT)
			e->cur = eprev(e, e->cur);
		else if(k == K_RIGHT)
			e->cur = enext(e, e->cur);
		else if(k == K_HOME)
			e->cur = 0;
		else if(k == K_END)
			e->cur = e->len;
		else if(k == (K_CTRL | 'U')) {	/* what is before the cursor goes */
			if(e->cur > 0) {
				edelete(e, 0, e->cur);
				changed = 1;
			}
		} else if(k == (K_CTRL | 'V') || k == (K_SHIFT | K_INS) || k == K_PASTE) {
			if(k == K_PASTE)
				p = term_paste(&n);
			else {
				p = ui_clip;
				n = ui_cliplen;
			}
			if(p && n) {
				einsert(e, p, n > EDL_MAX ? EDL_MAX : (int)n);
				changed = 1;
			}
		} else if(!(k & (K_CTRL | K_ALT)) && c >= ' ' && c < K_UP && c != 127) {
			einsert(e, u, ui_utf8(c, u));
			changed = 1;
		} else
			ui_key = k;	/* not for it */
		if(ui_key == 0)
			ui_again = 1;
	}
	if(e->cur < e->off)
		e->off = e->cur;
	while(e->off < e->cur && ewidth(e, e->off, e->cur) > w - 1)
		e->off = enext(e, e->off);
	scr_fill(x, y, x + w - 1, y, ' ', at);
	scr_ntext(x, y, e->s + e->off, e->len - e->off, at, x + w);
	if(f) {
		ui_cx = x + ewidth(e, e->off, e->cur);
		ui_cy = y;
	}
	return changed;
}

/* --- the list ------------------------------------------------------------ */

static int sb_x = -1, sb_y, sb_h, sb_start, sb_size;	/* the one drawn last */

void
ui_scrollbar_again(void)
{
	sb_x = -1;
}

/* the thumb of a bar of h rows (h - 2 of track, between the arrows):
 * as long as the part shown is of the total, where pos is of the range */
static void
thumb(int h, long pos, long total, long shown, int *start, int *size)
{
	unsigned long long track = (unsigned long long)(h - 2), t, s, range, p;

	t = total > 0 ? (unsigned long long)total : 1;
	s = shown < 1 ? 1 : (unsigned long long)shown > t ? t : (unsigned long long)shown;
	*size = (int)((track * s + t / 2) / t);
	if(*size < 1)
		*size = 1;
	if((unsigned long long)*size > track)
		*size = (int)track;
	range = t - s;
	p = pos < 0 ? 0 : (unsigned long long)pos > range ? range : (unsigned long long)pos;
	*start = range ? (int)(((track - (unsigned long long)*size) * p + range / 2) / range) : 0;
}

int
ui_scrollbar(int x, int y, int h, long pos, long total, long shown, int always)
{
	int i, start, size, same, in, was;

	if(h < 3 || (!always && total <= shown))
		return 0;
	thumb(h, pos, total, shown, &start, &size);
	if(x == sb_x && y == sb_y && h == sb_h && start == sb_start && size == sb_size)
		return 1;	/* as it is */
	same = x == sb_x && y == sb_y && h == sb_h;	/* the cells that changed */
	for(i = 0; i < h; i++) {
		in = i >= 1 + start && i < 1 + start + size;
		was = i >= 1 + sb_start && i < 1 + sb_start + sb_size;
		if(same && i != 0 && i != h - 1 && in == was)
			continue;
		if(i == 0)
			scr_put(x, y + i, G_SCROLLUP, S_TRACK);
		else if(i == h - 1)
			scr_put(x, y + i, G_SCROLLDOWN, S_TRACK);
		else if(in)
			scr_put(x, y + i, G_THUMB, S_THUMB);
		else
			scr_put(x, y + i, ' ', S_TRACK);
	}
	sb_x = x;
	sb_y = y;
	sb_h = h;
	sb_start = start;
	sb_size = size;
	return 1;
}

static int hb_x = -1, hb_y, hb_w, hb_start, hb_size;	/* the one drawn last */

int
ui_hscrollbar(int x, int y, int w, long pos, long total, long shown)
{
	int i, start, size, in, was, same;

	if(w < 3)
		return 0;
	thumb(w, pos, total, shown, &start, &size);
	if(x == hb_x && y == hb_y && w == hb_w && start == hb_start && size == hb_size)
		return 1;	/* as it is */
	same = x == hb_x && y == hb_y && w == hb_w;
	for(i = 0; i < w; i++) {
		in = i >= 1 + start && i < 1 + start + size;
		was = i >= 1 + hb_start && i < 1 + hb_start + hb_size;
		if(same && i != 0 && i != w - 1 && in == was)
			continue;
		if(i == 0)
			scr_put(x + i, y, G_SCROLLLEFT, S_HTHUMB);
		else if(i == w - 1)
			scr_put(x + i, y, G_SCROLLRIGHT, S_HTHUMB);
		else	/* the band: half of it next to an arrow, for a gap */
			scr_put(x + i, y, i == 1 ? G_HBANDR : i == w - 2 ? G_HBANDL : G_HTHUMB,
			    in ? S_HTHUMB : S_HTRACK);
	}
	hb_x = x;
	hb_y = y;
	hb_w = w;
	hb_start = start;
	hb_size = size;
	return 1;
}

void
ui_hscrollbar_again(void)
{
	hb_x = -1;
}

long
ui_scrollbar_at(int h, long total, long shown, int row)
{
	int start, size, track = h - 2;
	long long range;

	if(h < 3)
		return 0;
	thumb(h, 0, total, shown, &start, &size);
	range = total > shown ? (long long)total - shown : 0;
	if(track <= size || range <= 0)
		return 0;
	start = row - 1 - size / 2;	/* the middle of the thumb at the mouse */
	if(start < 0)
		start = 0;
	if(start > track - size)
		start = track - size;
	return (long)((range * start + (track - size) / 2) / (track - size));
}


/* --- the modals ---------------------------------------------------------- */

void
ui_modal_enter(void)
{
	scr_attr(0, 0, scr_cols - 1, scr_rows - 1, 0xFF, F_DIM);
	depth++;
	inside = depth;
	ui_well_begin(!ui_nograb);
	ui_nograb = 0;
}

void
ui_modal_begin(const char *title, int w, int h, int at, int *x, int *y)
{
	int x0 = (scr_cols - w) / 2, y0 = (scr_rows - h) / 2;

	if(x0 < 0)
		x0 = 0;
	if(y0 < 0)
		y0 = 0;
	ui_modal_enter();
	scr_fill(x0, y0, x0 + w - 1, y0 + h - 1, ' ', at);
	scr_frame(x0, y0, x0 + w - 1, y0 + h - 1, at);
	if(*title) {
		scr_put(x0 + 2, y0, ' ', at);
		scr_put(scr_text(x0 + 3, y0, title, at, x0 + w - 2), y0, ' ', at);
	}
	*x = x0;
	*y = y0;
}

int
ui_modal_end(void)
{
	int in = ui_well_end(), esc = in && ui_key == 27;

	inside = 0;
	/* none for what is behind; the mouse is the topmost modal's (one drawn
	   after this one, over it, has it: a question, an error) */
	if(in || (ui_key == K_MOUSE && depth >= nmodal))
		ui_key = 0;
	if(esc)
		ui_again = 1;
	return esc;
}
