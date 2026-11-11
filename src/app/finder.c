/* The project finder: the files of the project by a fuzzy match of
 * their paths (Alt+P), or the lines of their text that match a pattern
 * (Alt+/), each with a preview of the file.
 *
 * The project is the repository the project's folder is in (the first
 * folder above it with a .git), else that folder (main.c: project_set;
 * the folder the editor was started in, until one is chosen).  Its files are
 * found a step at a time (walk.c) while the editor is idle, matched as
 * they come and shown at once; typing filters what was found, and the
 * walk goes on.  Enter opens the one selected (at its line, for a match
 * of the text); Esc closes it.
 *
 *   +- query --------- 12/3456 -+ +- preview ---------------+
 *   | > src/app/finder.c        | | the lines of the file,   |
 *   |   src/app/fuzzy.c         | | highlighted, the line of |
 *   |   ...                     | | a match in the middle    |
 *   +---------------------------+ +--------------------------+
 */
#include <stdlib.h>
#include <string.h>
#include "finder.h"
#include "syntax.h"

#define PREVIEW_MIN	72	/* narrower: no preview */
#define SLICE		40	/* ms: the work of one turn */
#define RESORT		250	/* ms: the order, while matching */
#define WAIT		30	/* ms: for the threads' results, nothing else to do */
#define STEP_ENTRIES	256	/* of the walk, between two looks at the clock */
#define STEP_PATHS	8192	/* matched, the same */
#define RESULTS_MAX	1000000
#define LEX_BACK	((pos_t)256 << 10)	/* lexed before the preview, at most */
#define PREVIEW_LINE	4096	/* the bytes of a line shown, at most */

/* a result: a file (its index in the walk), with its score (a match of
 * its path) or a line and column (a match of its text) */
struct result {
	int file, score, len;	/* len: of its path */
	pos_t line, col;
};

static int mode;		/* FINDER_FILES, FINDER_TEXT */
static struct edl query;
static struct result *res, *merged;	/* merged: for sort_results */
static int nres, rescap, nsorted;	/* nsorted: the first ones, in order */
static int looked;		/* the files of the walk matched (or searched) */
static char words;		/* the query has words: the results are scored */
static char unsorted;		/* results came since they were sorted */
static long sorted_at;
static char searching;		/* FINDER_TEXT: a pattern is being looked for */
static const char *bad;		/* the pattern is not one: why */
static int sel, top;

/* the file previewed */
static struct buf pv;
static char pv_inited;
static int pv_file = -1;
static enum { PV_NONE, PV_TEXT, PV_BINARY, PV_UNREAD } pv_kind;
static int pv_lang;
static long pv_scroll;		/* lines the wheel moved it, from where it starts */
static int pv_sel = -1;		/* the result it is of: another one starts again */
#define WHEEL_LINES	3

/* --- the project and the results --------------------------------------- */

static void
full_path(int file, char *out, int max)
{
	const char *r = walk_root(), *p = walk_path(file);
	size_t a = strlen(r), b = strlen(p);

	if(a + b + 1 > (size_t)max) {
		out[0] = 0;
		return;
	}
	memcpy(out, r, a);
	memcpy(out + a, p, b + 1);
}

static void
add(int file, int score, int len, pos_t line, pos_t col)
{
	struct result *n;

	if(nres >= RESULTS_MAX)
		return;
	if(nres == rescap) {
		rescap = rescap ? rescap * 2 : 1024;
		if((n = realloc(res, (size_t)rescap * sizeof *res)) == 0) {
			rescap = nres;
			return;
		}
		res = n;
	}
	res[nres].file = file;
	res[nres].score = score;
	res[nres].len = len;
	res[nres].line = line;
	res[nres++].col = col;
}

/* the better first; then the shorter path; then as the walk found them */
static int
by_score(const void *a, const void *b)
{
	const struct result *x = a, *y = b;

	if(x->score != y->score)
		return x->score > y->score ? -1 : 1;
	if(x->len != y->len)
		return x->len < y->len ? -1 : 1;
	return x->file - y->file;
}

/* the results in order: those come since the last time sorted, then
 * merged with the others (in order already) */
static void
sort_results(void)
{
	struct result *m;
	int i = 0, j = nsorted, k = 0;

	qsort(res + nsorted, (size_t)(nres - nsorted), sizeof *res, by_score);
	if(nsorted > 0 && nsorted < nres) {
		/* as large as res: the two swap */
		if((m = realloc(merged, (size_t)rescap * sizeof *m)) == 0) {
			qsort(res, (size_t)nres, sizeof *res, by_score);	/* the slow way */
		} else {
			merged = m;
			while(i < nsorted && j < nres)
				m[k++] = by_score(&res[i], &res[j]) <= 0 ? res[i++] : res[j++];
			while(i < nsorted)
				m[k++] = res[i++];
			while(j < nres)
				m[k++] = res[j++];
			merged = res;	/* the two swap: no copy */
			res = m;
		}
	}
	nsorted = nres;
	unsorted = 0;
	sorted_at = plat_ms();
}

static void
text_hit(int file, pos_t line, pos_t col)
{
	add(file, 0, 0, line, col);
}

/* the query changed: the results again, from the files found */
static void
requery(void)
{
	query.s[query.len] = 0;
	nres = nsorted = 0;
	looked = 0;
	sel = top = 0;
	pv_sel = -1;
	unsorted = 0;
	bad = 0;
	if(mode == FINDER_FILES)
		words = fz_set(query.s) > 0;
	else
		searching = (char)scan_start(query.s, &bad);
	rows_stale = 1;
}

/* some work: the walk, and the files found matched or searched */
static void
work(void)
{
	const char *p;
	char path[PATH_MAX_];
	pos_t n;
	int sc;

	if(!walk_done())
		walk_step(STEP_ENTRIES);
	if(mode == FINDER_FILES) {
		for(n = 0; looked < walk_count() && n < STEP_PATHS; looked++, n++) {
			int len;

			p = walk_path(looked);
			len = (int)strlen(p);
			if((sc = fz_score(p, len)) >= 0) {
				add(looked, sc, len, 0, 0);
				unsorted = words;
			}
		}
		if(unsorted && ((walk_done() && looked == walk_count()) ||
		    plat_ms() - sorted_at > RESORT))
			sort_results();
		return;
	}
	/* the files found to the threads, as many as they take; what they
	   found back */
	if(!searching)
		return;
	for(; looked < walk_count(); looked++) {
		full_path(looked, path, sizeof path);
		if(!scan_feed(looked, path))
			break;
	}
	scan_take(text_hit, 0);
}

int
finder_busy(void)
{
	return st.finder && (!walk_done() || unsorted ||
	    (mode == FINDER_FILES && looked < walk_count()) ||
	    (mode == FINDER_TEXT && searching && (scan_busy() || looked < walk_count())));
}

/* the editor's own work, and that of the threads: only theirs left, it
 * waits for their results (not turning round meanwhile), a key ends it */
void
finder_work(void)
{
	long t0 = plat_ms();

	do {
		work();
		/* the walk done, the files given (or the queue full): woken by a
		   result, or by room in the queue */
		if(mode == FINDER_TEXT && searching && walk_done())
			scan_take(text_hit, WAIT);
	} while(finder_busy() && !term_keyready() && plat_ms() - t0 < SLICE);
	ui_again = 1;
}

void
finder_open(int m)
{
	char root[PATH_MAX_];
	pos_t a, n;

	if(st.finder)
		finder_close();
	mode = m;
	edl_set(&query, "");
	/* a search of the text starts from the selection, if it is a line */
	if(m == FINDER_TEXT && docs && has_sel() && (n = sel_end() - (a = sel_begin())) < 200 &&
	   buf_count(a, a + n, 1) == 0) {
		char s[200];

		buf_copy(a, n, s);
		s[n] = 0;
		edl_set(&query, s);
	}
	project_root(root, sizeof root);
	walk_start(root);
	st.finder = (char)m;
	requery();
	pv_file = pv_sel = -1;
	ui_steal(ID_FINDER);
}

void
finder_close(void)
{
	struct buf *keep = B;

	scan_stop();		/* the threads ended first */
	walk_stop();
	free(res);
	free(merged);
	res = merged = 0;
	nres = rescap = nsorted = 0;
	searching = 0;
	if(pv_inited) {
		B = &pv;
		buf_free();
		B = keep;
	}
	pv_file = pv_sel = -1;
	st.finder = 0;
	rows_stale = 1;
	ui_again = 1;
}

/* the result selected opened: the file, at the line of a match */
static void
open_selected(void)
{
	char path[PATH_MAX_];
	struct result r;

	if(sel >= nres)
		return;
	r = res[sel];
	full_path(r.file, path, sizeof path);
	finder_close();
	if(!doc_open(path))
		return;
	if(mode == FINDER_TEXT)
		cmd_goto(r.line, r.col);
	cursor_visible();
}

/* --- the preview --------------------------------------------------------- */

static void
preview_load(int file)
{
	char path[PATH_MAX_];
	struct buf *keep = B;
	pos_t i, n;

	if(file == pv_file)
		return;
	pv_file = file;
	B = &pv;
	if(!pv_inited) {
		buf_init();
		pv_inited = 1;
	}
	full_path(file, path, sizeof path);
	pv_kind = PV_UNREAD;
	if(buf_load_search(path) == B_OK) {	/* large: mapped, never copied */
		pv_kind = PV_TEXT;
		n = buf_len() < 8192 ? buf_len() : 8192;
		for(i = 0; i < n; i++)
			if(BCH(i) == 0) {
				pv_kind = PV_BINARY;
				break;
			}
		pv_lang = lang_detect(path);
	}
	if(pv_kind != PV_TEXT)
		buf_free();
	B = keep;
}

static int
u8len(int c)
{
	return c < 0xC0 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
}

/* the line [ls, le) of the preview at row y from x to x2, lexed from
 * state *lex; cur: the line of the match */
static void
preview_line(int x, int y, int x2, pos_t ls, pos_t le, int *lex, int cur)
{
	static char text[PREVIEW_LINE];
	static unsigned char cls[PREVIEW_LINE];
	int n = (int)(le - ls < PREVIEW_LINE ? le - ls : PREVIEW_LINE), i, k, at, x0 = x;
	int tab = pv.tabsize > 0 ? pv.tabsize : 4;

	buf_copy(ls, (pos_t)n, text);
	if(syn_has(pv_lang))
		*lex = syn_line(pv_lang, text, n, *lex, cls);
	else
		memset(cls, HL_TEXT, (size_t)n);
	if(cur)
		scr_fill(x, y, x2, y, ' ', S_CURLINE);
	for(i = 0; i < n && x <= x2; i += k) {
		k = u8len((unsigned char)text[i]);
		if(i + k > n)
			k = n - i;
		at = hl_style(cls[i], cur);
		if(text[i] == '\t') {
			do
				scr_put(x++, y, ' ', at);
			while((x - x0) % tab && x <= x2);
		} else if((unsigned char)text[i] < 0x20)
			scr_put(x++, y, ' ', at);
		else
			x = scr_ntext(x, y, text + i, k, at, x2 + 1);
	}
}

/* the file of the result selected, from row y1 to y2: the line of a match
 * in the middle, marked */
static void
draw_preview(int x1, int y1, int x2, int y2)
{
	struct buf *keep = B;
	const char *say = 0;
	pos_t line = 0, first, ls, le, i;
	int h = y2 - y1 + 1, row, lex = 0;

	if(nres == 0)
		return;
	if(sel != pv_sel) {	/* another result: as it starts */
		pv_sel = sel;
		pv_scroll = 0;
	}
	preview_load(res[sel].file);
	if(pv_kind == PV_BINARY)
		say = "<Binary file>";
	else if(pv_kind == PV_UNREAD)
		say = "<Cannot read the file>";
	if(say) {
		scr_text(x1 + (x2 - x1 + 1 - text_width(say)) / 2, y1 + h / 2, say,
		    S_COMMENT, x2 + 1);
		return;
	}
	B = &pv;
	if(mode == FINDER_TEXT)
		line = res[sel].line;
	first = line > (pos_t)(h / 2) ? line - (pos_t)(h / 2) : 0;
	/* moved by the wheel: not above the first line, nor further down
	   than the last one at the bottom (a line past the last starts where
	   the last does: buf_lineoff) */
	if(pv_scroll < 0 && (pos_t)-pv_scroll > first)
		pv_scroll = -(long)first;
	first += (pos_t)pv_scroll;
	while(pv_scroll > 0 && h > 1 &&
	    buf_lineoff(first + (pos_t)h - 1) == buf_lineoff(first + (pos_t)h - 2)) {
		first--;
		pv_scroll--;
	}
	ls = buf_lineoff(first);
	/* the state of the lexer there: from the start, if not too far */
	if(syn_has(pv_lang) && ls <= LEX_BACK)
		for(i = 0; i < ls; i = le + 1) {
			le = buf_lineend(i);
			preview_line(x2 + 1, y1, x2, i, le, &lex, 0);	/* lexed, not shown */
		}
	for(row = 0; row < h && ls <= buf_len(); row++) {
		le = buf_lineend(ls);
		preview_line(x1, y1 + row, x2, ls, le, &lex,
		    mode == FINDER_TEXT && first + (pos_t)row == line);
		if(le >= buf_len())
			break;
		ls = le + 1;
	}
	B = keep;
}

/* --- the list ------------------------------------------------------------ */

/* a result at row y, from x to x2: its folders dim, its name bright, what
 * the query matched marked, then its line; cut at the left (…) when
 * too long */
static void
draw_result(int x, int y, int x2, int i, int cur)
{
	static unsigned char mark[PATH_MAX_];
	const char *p = walk_path(res[i].file);
	char num[24];
	int n = (int)strlen(p), name, k, j, w, at, room = x2 - x + 1;

	num[0] = 0;
	if(mode == FINDER_TEXT) {
		num[0] = ':';
		udec(num + 1, res[i].line + 1);
	}
	if(n >= (int)sizeof mark)
		n = (int)sizeof mark - 1;
	if(mode == FINDER_FILES)
		fz_marks(p, n, mark);
	else
		memset(mark, 0, (size_t)n);
	for(name = n; name > 0 && p[name - 1] != '/'; name--)
		;
	j = 0;
	w = text_width(p) + text_width(num);
	if(w > room) {		/* the start goes */
		scr_put(x++, y, G_ELLIPSIS, cur ? S_CURLINE : S_TEXT);
		for(; j < n && w > room - 1; j += k) {
			k = u8len((unsigned char)p[j]);
			w -= cwidth((unsigned char)p[j] < 0x80 ? (unsigned char)p[j] : 0x100);
		}
	}
	for(; j < n && x <= x2; j += k) {
		k = u8len((unsigned char)p[j]);
		if(j + k > n)
			k = n - j;
		at = mark[j] ? hl_style(HL_KEYWORD, cur) :
		    j < name ? hl_style(HL_COMMENT, cur) : hl_style(HL_TEXT, cur);
		x = scr_ntext(x, y, p + j, k, at, x2 + 1);
	}
	if(num[0])
		scr_text(x, y, num, hl_style(HL_NUMBER, cur), x2 + 1);
}

/* the keys of the list (the query's line has the others): 1 if one */
static int
list_key(int h)
{
	int s = sel;

	switch(ui_key) {
	case K_UP:
	case K_CTRL | 'P':
		s--;
		break;
	case K_DOWN:
	case K_CTRL | 'N':
		s++;
		break;
	case K_PGUP:
		s -= h - 1;
		break;
	case K_PGDN:
		s += h - 1;
		break;
	case K_CTRL | K_HOME:
		s = 0;
		break;
	case K_CTRL | K_END:
		s = nres - 1;
		break;
	default:
		return 0;
	}
	sel = s >= nres ? nres - 1 : s < 0 ? 0 : s;
	ui_key = 0;
	return 1;
}

/* the mouse on the list (rows y to y + h - 1, columns x to x2) */
static int
list_mouse(int x, int y, int x2, int h)
{
	int row = term_mouse.y - y;

	if(ui_key != K_MOUSE || !ui_mouse_ok() || term_mouse.x < x || term_mouse.x > x2 ||
	   row < 0 || row >= h)
		return 0;
	if(term_mouse.kind == M_WHEELUP || term_mouse.kind == M_WHEELDOWN) {
		sel += term_mouse.kind == M_WHEELUP ? -3 : 3;
		sel = sel >= nres ? nres - 1 : sel < 0 ? 0 : sel;
	} else if(term_mouse.kind == M_PRESS && term_mouse.button == MB_LEFT && top + row < nres) {
		sel = top + row;
		if(term_mouse.clicks == 2)
			return 2;
	}
	ui_key = 0;
	return 1;
}

void
draw_finder(void)
{
	char count[48];
	int mx = scr_cols >= 60 ? scr_cols / 20 : 0, my = scr_rows >= 20 ? scr_rows / 20 + 1 : 0;
	int x1 = mx, y1 = my, x2 = scr_cols - 1 - mx, y2 = scr_rows - 1 - my;
	int lx2 = x2, ly, lh, cw, row, act = 0;

	if(x2 - x1 + 1 >= PREVIEW_MIN)
		lx2 = x1 + (x2 - x1) / 2 - 1;
	ly = y1 + 3 + (mode == FINDER_TEXT);	/* a search of the text: a header */
	lh = y2 - ly;
	if(lh < 1 || lx2 - x1 < 12) {	/* no room */
		finder_close();
		return;
	}
	ui_modal_enter();
	scr_fill(x1, y1, lx2, y2, ' ', S_TEXT);
	scr_frame(x1, y1, lx2, y2, S_TEXT);
	{	/* the project, on the frame: where it looks */
		char name[PATH_MAX_ + 2];
		const char *r = walk_root(), *b;
		size_t n = strlen(r);

		while(n > 1 && r[n - 1] == '/')
			n--;
		for(b = r + n; b > r && b[-1] != '/'; b--)
			;
		name[0] = ' ';
		memcpy(name + 1, b, (size_t)(r + n - b));
		strcpy(name + 1 + (r + n - b), " ");
		ui_trunc(x1 + 2, y1, name, S_TEXT, lx2 - 1);
	}
	scr_put(x1, y1 + 2, G_SEPL, S_TEXT);
	scr_fill(x1 + 1, y1 + 2, lx2 - 1, y1 + 2, G_H, S_TEXT);
	scr_put(lx2, y1 + 2, G_SEPR, S_TEXT);

	/* the wheel over the preview scrolls it */
	if(lx2 != x2 && ui_key == K_MOUSE && ui_mouse_ok() && term_mouse.x > lx2 + 1 &&
	   term_mouse.x <= x2 && term_mouse.y >= y1 && term_mouse.y <= y2 &&
	   (term_mouse.kind == M_WHEELUP || term_mouse.kind == M_WHEELDOWN)) {
		pv_scroll += term_mouse.kind == M_WHEELUP ? -WHEEL_LINES : WHEEL_LINES;
		ui_key = 0;
		ui_again = 1;
	}
	/* the keys: the list's, Enter, then the query's */
	if(nres && (list_key(lh) || (act = list_mouse(x1 + 1, ly, lx2 - 1, lh)) != 0))
		ui_again = 1;
	if(ui_key == 13) {
		ui_key = 0;
		act = 2;
	}
	/* the count, at the right of the query */
	count[0] = 0;
	if(bad)
		strcpy(count, "Invalid pattern");
	else {
		if(finder_busy())
			strcpy(count, "(running) ");
		/* as Helix: what is shown of all there is (a search of the
		   text shows all it found) */
		udec(count + strlen(count), (pos_t)nres);
		strcat(count, "/");
		udec(count + strlen(count), (pos_t)(mode == FINDER_FILES ? walk_count() : nres));
	}
	cw = text_width(count);
	scr_text(lx2 - 1 - cw, y1 + 1, count, bad ? S_FAIL : S_COMMENT, lx2);
	ui_steal(ID_FINDER);
	if(ui_edit(ID_FINDER, x1 + 2, y1 + 1, lx2 - x1 - 4 - cw, &query, S_TEXT))
		requery();

	/* the list: the selected one in sight */
	if(sel >= nres)
		sel = nres ? nres - 1 : 0;
	if(sel < top)
		top = sel;
	if(sel >= top + lh)
		top = sel - lh + 1;
	if(mode == FINDER_TEXT)
		scr_text(x1 + 3, y1 + 3, nres || query.len ? "path" :
		    "A text, or a regular expression", S_COMMENT, lx2);
	for(row = 0; row < lh && top + row < nres; row++) {
		int cur = top + row == sel;

		if(cur)
			scr_fill(x1 + 1, ly + row, lx2 - 1, ly + row, ' ', S_CURLINE);
		scr_text(x1 + 1, ly + row, cur ? ">" : " ", cur ? S_CURLINE : S_TEXT, lx2);
		draw_result(x1 + 3, ly + row, lx2 - 1, top + row, cur);
	}

	if(lx2 != x2) {		/* the preview */
		scr_fill(lx2 + 2, y1, x2, y2, ' ', S_TEXT);
		scr_frame(lx2 + 2, y1, x2, y2, S_TEXT);
		draw_preview(lx2 + 4, y1 + 1, x2 - 2, y2 - 1);
	}
	if(ui_modal_end())
		finder_close();
	else if(act == 2)
		open_selected();
}
