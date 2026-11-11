/* The POSIX terminal: raw termios, the alternate screen, truecolor in the
 * colours of the terminal itself (asked with OSC 4, 10 and 11), UTF-8, the
 * mouse (SGR), bracketed paste, OSC 52 for the clipboard and OSC 0 for the
 * title. */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "../platform.h"
#include "../colours.h"

struct mouse term_mouse;
const char plat_name[] = "cedit";

static struct termios saved;
static char out[16384];
static int nout, curat = -1, isopen;
static volatile sig_atomic_t resized;


static void
emit(const char *s, int n)
{
	if(nout + n > (int)sizeof out)
		term_flush();
	if(n > (int)sizeof out) {
		(void)!write(1, s, n);
		return;
	}
	memcpy(out + nout, s, n);
	nout += n;
}

static void
emits(const char *s)
{
	emit(s, (int)strlen(s));
}

void
term_flush(void)
{
	int n = 0, r;

	while(n < nout) {
		r = (int)write(1, out + n, nout - n);
		if(r <= 0)
			break;
		n += r;
	}
	nout = 0;
}

static int
parse_rgb(const char *s, unsigned long *c)
{
	unsigned long v[3];
	int i, n;
	const char *p = strstr(s, "rgb:");

	if(p == 0)
		return 0;
	p += 4;
	for(i = 0; i < 3; i++) {
		v[i] = 0;
		for(n = 0; (*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'); p++, n++)
			v[i] = v[i] * 16 + (unsigned long)(*p <= '9' ? *p - '0' : (*p | 0x20) - 'a' + 10);
		if(n == 0)
			return 0;
		if(n > 2)
			v[i] >>= 4 * (n - 2);
		else if(n == 1)
			v[i] *= 17;
		if(i < 2 && *p++ != '/')
			return 0;
	}
	*c = v[0] << 16 | v[1] << 8 | v[2];
	return 1;
}

static int
waitkey(int ms)
{
	struct pollfd p;

	p.fd = 0;
	p.events = POLLIN;
	return poll(&p, 1, ms) > 0;
}

/* the palette, as the terminal answers: the answers end at the one of
 * DA1 (ESC [ ? ... c), which every terminal gives */
static void
ask_colours(void)
{
	char b[4096], *p, *e;
	int n = 0, r, idx;
	unsigned long c;

	emits("\033]4;0;?;1;?;2;?;3;?;4;?;5;?;6;?;7;?\a"
	    "\033]4;8;?;9;?;10;?;11;?;12;?;13;?;14;?;15;?\a"
	    "\033]10;?\a\033]11;?\a\033[c");
	term_flush();
	while(n < (int)sizeof b - 1 && waitkey(300)) {
		r = (int)read(0, b + n, sizeof b - 1 - n);
		if(r <= 0)
			break;
		n += r;
		b[n] = 0;
		if((p = strstr(b, "\033[?")) != 0 && strchr(p, 'c'))
			break;
	}
	b[n] = 0;
	for(p = b; (p = strstr(p, "\033]")) != 0; p = e) {
		p += 2;
		for(e = p; *e && *e != '\a' && *e != '\033'; e++)
			;
		if(*e)
			*e++ = 0;
		if(strncmp(p, "4;", 2) == 0) {
			idx = atoi(p + 2);
			if(idx >= 0 && idx < 16 && parse_rgb(p, &c))
				pal[idx] = c;
		} else if(strncmp(p, "10;", 3) == 0 && parse_rgb(p, &c))
			pal[FG] = c;
		else if(strncmp(p, "11;", 3) == 0 && parse_rgb(p, &c))
			pal[BG] = c;
	}
}

/* --- the screen ---------------------------------------------------------- */

static void
onwinch(int sig)
{
	(void)sig;
	resized = 1;
}

int
term_size(int *cols, int *rows)
{
	struct winsize ws;

	*cols = 80;
	*rows = 25;
	if(ioctl(1, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
	}
	if(*cols < 20)
		*cols = 20;
	if(*rows < 8)
		*rows = 8;
	curat = -1;
	return 1;
}

int
term_open(int *cols, int *rows)
{
	struct termios t;
	struct sigaction sa;

	if(!isatty(0) || !isatty(1) || tcgetattr(0, &saved) < 0)
		return 0;
	t = saved;
	t.c_iflag &= ~(IXON | ICRNL | INLCR | ISTRIP | BRKINT);
	t.c_oflag &= ~OPOST;
	t.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &t);
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = onwinch;
	sigaction(SIGWINCH, &sa, 0);
	isopen = 1;
	ask_colours();
	colours_styles();
	term_size(cols, rows);
	emits("\033[?1049h\033[?1003;1006;2004h\033[>4;1m\033[H\033[2J");
	return 1;
}

void
term_close(void)
{
	if(!isopen)
		return;
	emits("\033[0m\033[0 q\033[?25h\033]0;\a\033[>4;0m\033[?1003;1006;2004l\033[?1049l");
	term_flush();
	tcsetattr(0, TCSAFLUSH, &saved);
	isopen = 0;
}

static void
style(int at)
{
	char b[80];
	unsigned long fg = sfg[STYLE(at)], bg = sbg[STYLE(at)], t;

	if(at == curat)
		return;
	curat = at;
	if(at & F_DIM) {	/* greyed (on a float: the text alone, an item
				   that does not apply) */
		fg = colours_blend(fg, STYLE(at) == S_FLOAT ? bg : pal[BG], 1, 2);
		if(STYLE(at) != S_FLOAT)
			bg = colours_blend(bg, pal[BG], 1, 2);
	}
	if(at & F_REV) {
		t = fg;
		fg = bg;
		bg = t;
	}
	snprintf(b, sizeof b, "\033[0;%s38;2;%lu;%lu;%lu;48;2;%lu;%lu;%lum", at & F_UNDER ? "4;" : "",
	    fg >> 16, fg >> 8 & 0xFF, fg & 0xFF, bg >> 16, bg >> 8 & 0xFF, bg & 0xFF);
	emits(b);
}

static const char *const glyphs[G_LAST] = {
	" ", "\xe2\x94\x8c", "\xe2\x94\x90", "\xe2\x94\x94", "\xe2\x94\x98",
	"\xe2\x94\x80", "\xe2\x94\x82",		/* ┌ ┐ └ ┘ ─ │ */
	"\xe2\x96\x88",				/* █ */
	"\xe2\x9c\x93", " ",			/* ✓, and none */
	"\xe2\x88\x99", "\xe2\x80\xa6",		/* ∙ … */
	"\xef\xbf\xbd",				/* � */
	"\xe2\x94\x80", "\xe2\x86\xb5", "\xe2\x97\x86",	/* ─ (a tab's first cell) ↵ ◆ */
	"\xe2\x96\xb2", "\xe2\x96\xbc",		/* ▲ ▼ */
	"\xe2\x98\x91", "\xe2\x98\x90",		/* ☑ ☐ */
	"\xe2\x97\x80", "\xe2\x96\xb6",		/* ◀ ▶ */
	"\xf0\x9f\xac\x8b",		/* the middle third of a cell (U+1FB0B):
					   level with the arrows */
	"\xf0\x9f\xac\x87", "\xf0\x9f\xac\x83",	/* its right half, its left
					   half (U+1FB07, U+1FB03) */
	"\xe2\x94\x80", "\xe2\x86\x92",		/* a tab: ─ to its last cell, → */
	"\xe2\x94\x82",				/* │ an indent guide */
	"\xe2\x94\x9c", "\xe2\x94\xa4",		/* ├ ┤ */
};

static int
utf8(unsigned long c, char *s)
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

void
term_write(int x, int y, const cell_t *ch, const unsigned char *at, int n)
{
	char b[32];
	int i;

	snprintf(b, sizeof b, "\033[%d;%dH", y + 1, x + 1);
	emits(b);
	for(i = 0; i < n; i++) {
		if(ch[i] == CELL_WIDE)
			continue;
		style(at[i]);
		if(ch[i] < G_LAST)
			emits(glyphs[ch[i]]);
		else
			emit(b, utf8(ch[i], b));
	}
}

void
term_cursor(int x, int y, int visible)
{
	char b[48];

	if(!visible) {
		emits("\033[?25l");
		term_flush();
		return;
	}
	snprintf(b, sizeof b, "\033[%d;%dH\033[?25h", y + 1, x + 1);
	emits(b);
	term_flush();
}

int
term_font_size(void)
{
	return 0;	/* the terminal's own */
}

void
term_set_font_size(int px)
{
	(void)px;
}

void
term_stay(int on)
{
	(void)on;	/* a window's alone */
}

int
term_set_font(const char *spec)
{
	(void)spec;	/* the terminal's own */
	return FONT_OK;
}

int
term_zoom(int how)
{
	(void)how;
	return 0;
}

void
term_lower(int x1, int x2, int y)
{
	(void)x1;
	(void)x2;
	(void)y;
}

void
term_beep(void)
{
	emits("\a");
	term_flush();
}

void
term_title(const char *name, int dirty)
{
	static char last[64];
	static int lastdirty = -1;
	char b[128];
	const char *p;
	int n = 0;

	if(strncmp(last, name, sizeof last - 1) == 0 && dirty == lastdirty)
		return;
	strncpy(last, name, sizeof last - 1);
	lastdirty = dirty;
	emits("\033]0;");
	if(dirty)
		emits("\xe2\x97\x8f ");		/* ● */
	for(p = name; *p && n < (int)sizeof b - 1; p++)
		if((unsigned char)*p >= ' ' && *p != 127)
			b[n++] = *p;
	b[n] = 0;
	emits(b);
	emits(" - cedit\033\\");
	term_flush();
}

void
term_clipboard(const char *s, size_t n)
{
	static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char q[4];
	size_t i;
	unsigned long v;

	if(n > 128 * 1024)
		return;		/* too large for the terminal to take */
	emits("\033]52;c;");
	for(i = 0; i < n; i += 3) {
		v = (unsigned long)(unsigned char)s[i] << 16;
		if(i + 1 < n)
			v |= (unsigned long)(unsigned char)s[i + 1] << 8;
		if(i + 2 < n)
			v |= (unsigned char)s[i + 2];
		q[0] = b64[v >> 18 & 63];
		q[1] = b64[v >> 12 & 63];
		q[2] = i + 1 < n ? b64[v >> 6 & 63] : '=';
		q[3] = i + 2 < n ? b64[v & 63] : '=';
		emit(q, 4);
	}
	emits("\a");
	term_flush();
}

/* --- the keys ------------------------------------------------------------ */

static unsigned char inbuf[4096];
static int inlen, inpos;
static char *pastebuf;
static size_t pastelen, pastecap;

static int
getbyte(int ms)
{
	int r;

	if(inpos < inlen)
		return inbuf[inpos++];
	if(ms >= 0 && !waitkey(ms))
		return -1;
	for(;;) {
		r = (int)read(0, inbuf, sizeof inbuf);
		if(r > 0)
			break;
		if(r == 0)
			return -1;
		if(resized)
			return -2;
	}
	inlen = r;
	inpos = 1;
	return inbuf[0];
}

int
term_keyready(void)
{
	return inpos < inlen || waitkey(0);
}

const char *
term_paste(size_t *n)
{
	*n = pastelen;
	return pastebuf;
}

static void
paste_add(int c)
{
	char *p;

	if(pastelen == pastecap) {
		p = realloc(pastebuf, pastecap ? pastecap * 2 : 4096);
		if(p == 0)
			return;
		pastebuf = p;
		pastecap = pastecap ? pastecap * 2 : 4096;
	}
	pastebuf[pastelen++] = (char)c;
}

/* the text until ESC [ 201 ~ */
static long
read_paste(void)
{
	static const char end[] = "\033[201~";
	int c, m = 0, i;

	pastelen = 0;
	while((c = getbyte(-1)) >= 0) {
		if(c == (unsigned char)end[m]) {
			if(++m == 6)
				break;
			continue;
		}
		for(i = 0; i < m; i++)	/* a part of the end that was not: text */
			paste_add(end[i]);
		m = c == end[0];
		if(!m)
			paste_add(c);
	}
	return K_PASTE;
}

static kcode
mods(int m)
{
	kcode k = 0;

	if(m > 1) {
		if((m - 1) & 1)
			k |= K_SHIFT;
		if((m - 1) & 2)
			k |= K_ALT;
		if((m - 1) & 4)
			k |= K_CTRL;
	}
	return k;
}

static long
mouse(int b, int x, int y, int up)
{
	static struct timespec last;
	static int lx, ly, lclicks;
	struct timespec now;
	long ms;

	term_mouse.x = x - 1;
	term_mouse.y = y - 1;
	term_mouse.mods = (b & 4 ? K_SHIFT : 0) | (b & 8 ? K_ALT : 0) | (b & 16 ? K_CTRL : 0);
	term_mouse.clicks = 1;
	term_mouse.button = b & 3;	/* 3: none (a move) */
	if(b & 64) {
		if(b & 2)	/* the wheel sideways */
			term_mouse.kind = (b & 1) ? M_WHEELRIGHT : M_WHEELLEFT;
		else
			term_mouse.kind = (b & 1) ? M_WHEELDOWN : M_WHEELUP;
	} else if(up)
		term_mouse.kind = M_RELEASE;
	else if(b & 32)
		term_mouse.kind = (b & 3) == 3 ? M_MOVE : M_DRAG;
	else {
		term_mouse.kind = M_PRESS;
		if((b & 3) == 3)
			return K_NONE;
		/* the clicks in a row at one place: 1, 2, 3, then 1 again */
		clock_gettime(CLOCK_MONOTONIC, &now);
		ms = (now.tv_sec - last.tv_sec) * 1000 + (now.tv_nsec - last.tv_nsec) / 1000000;
		if(ms < 400 && lx == term_mouse.x && ly == term_mouse.y && term_mouse.button == MB_LEFT)
			term_mouse.clicks = lclicks % 3 + 1;
		lclicks = term_mouse.clicks;
		last = now;
		lx = term_mouse.x;
		ly = term_mouse.y;
	}
	return K_MOUSE;
}

/* a key of the PC: CSI params final */
static long
csi(void)
{
	int p[4] = { 0, 0, 0, 0 }, np = 0, c, priv = 0;

	c = getbyte(50);
	if(c == '<' || c == '?' || c == '>') {
		priv = c;
		c = getbyte(50);
	}
	for(;;) {
		if(c >= '0' && c <= '9') {
			if(np == 0)
				np = 1;
			p[np - 1] = p[np - 1] * 10 + c - '0';
		} else if(c == ';') {
			if(np == 0)
				np = 1;
			if(np < 4)
				np++;
		} else
			break;
		c = getbyte(50);
	}
	if(c < 0)
		return 27;
	if(priv == '<' && (c == 'M' || c == 'm'))
		return mouse(p[0], p[1], p[2], c == 'm');
	if(priv)
		return K_NONE;	/* an answer to a question */
	switch(c) {
	case 'A': return K_UP | mods(p[1]);
	case 'B': return K_DOWN | mods(p[1]);
	case 'C': return K_RIGHT | mods(p[1]);
	case 'D': return K_LEFT | mods(p[1]);
	case 'H': return K_HOME | mods(p[1]);
	case 'F': return K_END | mods(p[1]);
	case 'R': return K_F3 | mods(p[1]);
	case 'Z': return K_SHIFT | 9;
	case 'u':	/* CSI code ; mods u */
		if(p[0] >= 'a' && p[0] <= 'z' && (mods(p[1]) & (K_CTRL | K_ALT)))
			return (p[0] - 32) | mods(p[1]);
		return p[0] | mods(p[1]);
	case '~':
		break;
	default:
		return K_NONE;
	}
	switch(p[0]) {
	case 1: case 7: return K_HOME | mods(p[1]);
	case 4: case 8: return K_END | mods(p[1]);
	case 2: return K_INS | mods(p[1]);
	case 3: return K_DEL | mods(p[1]);
	case 5: return K_PGUP | mods(p[1]);
	case 6: return K_PGDN | mods(p[1]);
	case 13: return K_F3 | mods(p[1]);
	case 21: return K_F10 | mods(p[1]);
	case 200: return read_paste();
	case 27:	/* modifyOtherKeys: 27 ; mods ; code */
		if(p[2] >= 'a' && p[2] <= 'z')
			return (p[2] - 32) | mods(p[1]);
		if(p[2] >= 'A' && p[2] <= 'Z')
			return p[2] | mods(p[1]) | K_SHIFT;
		if(p[2] == 127)
			return 8 | mods(p[1]);
		return p[2] | mods(p[1]);
	}
	return K_NONE;
}

static long
utf8key(int c)
{
	int n, i, d;
	long cp;

	if(c >= 0xF0) {
		n = 3;
		cp = c & 7;
	} else if(c >= 0xE0) {
		n = 2;
		cp = c & 15;
	} else if(c >= 0xC0) {
		n = 1;
		cp = c & 31;
	} else
		return 0xFFFD;
	for(i = 0; i < n; i++) {
		d = getbyte(50);
		if(d < 0 || (d & 0xC0) != 0x80)
			return 0xFFFD;
		cp = cp << 6 | (d & 0x3F);
	}
	return cp;
}

long
term_key(void)
{
	int c, d;

	term_flush();
	for(;;) {
		if(resized) {
			resized = 0;
			return K_RESIZE;
		}
		if(inpos >= inlen && !waitkey(1000)) {
			if(resized)
				continue;	/* K_RESIZE, above */
			return K_NONE;	/* idle a second */
		}
		c = getbyte(-1);
		if(c == -2)
			continue;
		if(c < 0)
			return K_CTRL | 'Q';	/* the terminal went */
		break;
	}
	if(c == 127)
		return 8;
	if(c == 8)
		return K_CTRL | 8;
	if(c == 9 || c == 13)
		return c;
	if(c == 0)
		return K_CTRL | ' ';
	if(c < 27)
		return K_CTRL | ('A' + c - 1);
	if(c > 27 && c < 32)	/* Ctrl+\ ] ^ _ */
		return K_CTRL | ('A' + c - 1);
	if(c >= 0x80)
		return utf8key(c);
	if(c != 27)
		return c;
	d = getbyte(30);
	if(d == 27)
		inpos--;	/* Esc, then another key: that one is read next */
	if(d < 0 || d == 27)
		return 27;
	if(d == '[')
		return csi();
	if(d == 'O') {
		d = getbyte(50);
		switch(d) {
		case 'A': return K_UP;
		case 'B': return K_DOWN;
		case 'C': return K_RIGHT;
		case 'D': return K_LEFT;
		case 'H': return K_HOME;
		case 'F': return K_END;
		case 'R': return K_F3;
		}
		return K_NONE;
	}
	if(d >= 'a' && d <= 'z')
		return K_ALT | (d - 32);
	if(d == 13)
		return K_ALT | 13;
	if(d < 27)
		return K_CTRL | K_ALT | ('A' + d - 1);
	return K_ALT | d;
}
