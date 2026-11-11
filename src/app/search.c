/* The Find/Replace panel and what it does: drawn while the panel is
 * open. */
#include <string.h>
#include "app.h"
#include "../core/mem.h"
#include "../core/regex.h"

/* --- the matches: plain text, or a regular expression -------------------- */

static struct re *re;		/* the expression, compiled */
static char re_pat[EDL_MAX + 1];
static char re_case = -1;
static pos_t groups[2 * RE_GROUPS];	/* of the last match */

/* the expression of the panel compiled (once): 0 if it is not one */
static int
re_ready(void)
{
	const char *err;

	if(re && re_case == st.mcase && strcmp(re_pat, st.needle.s) == 0)
		return 1;
	re_free(re);
	re = re_compile(st.needle.s, !st.mcase, &err);
	strcpy(re_pat, st.needle.s);
	re_case = st.mcase;
	return re != 0;
}

/* a word ends on each side of [a, b) */
static int
whole(pos_t a, pos_t b)
{
	return (a == 0 || !isword(BCH(a - 1)) || !isword(BCH(a))) &&
	    (b >= buf_len() || b == a || !isword(BCH(b)) || !isword(BCH(b - 1)));
}

/* a match at a: its length */
static int
match_at(pos_t a, pos_t *n)
{
	int flags = (st.mcase ? FIND_CASE : 0) | (st.wword ? FIND_WORD : 0);
	pos_t e;

	if(!st.regex) {
		*n = strlen(st.needle.s);
		return buf_match(a, st.needle.s, flags);
	}
	if(!re_ready() || !re_match(re, a, &e, groups) || (st.wword && !whole(a, e)))
		return 0;
	*n = e - a;
	return 1;
}

/* the first match from off (past the end: from the start) */
static int
find_from(pos_t off, int wrap, pos_t *at, pos_t *n)
{
	int flags = (st.mcase ? FIND_CASE : 0) | (st.wword ? FIND_WORD : 0);
	pos_t a, e, from = off, to = buf_len();

	if(!st.regex) {
		*n = strlen(st.needle.s);
		if(!buf_find(off, st.needle.s, flags, at))
			return 0;
		return wrap || *at >= off;
	}
	if(!re_ready())
		return 0;
	for(;;) {
		while(re_find(re, from, to, &a, &e, groups)) {
			if(!st.wword || whole(a, e)) {
				*at = a;
				*n = e - a;
				return 1;
			}
			from = a + 1;
		}
		if(!wrap || to == off)
			return 0;
		from = 0;	/* from the start, up to off */
		to = off;
	}
}

/* the replacement of a match: with $0 to $9 (or \1 to \9) as the groups
 * of an expression; mem_alloc'ed, its length at *n */
static char *
replacement(pos_t *n)
{
	const char *r = st.repl.s, *p;
	pos_t k = 0, g;
	char *s, *o;
	int i, pass;

	if(!st.regex) {
		*n = strlen(r);
		return mem_strdup(r);
	}
	for(pass = 0, s = 0; pass < 2; pass++) {	/* the length, then the bytes */
		o = s;
		k = 0;
		for(p = r; *p; p++) {
			if((*p == '$' || *p == '\\') && p[1] >= '0' && p[1] <= '9' &&
			   groups[2 * (i = p[1] - '0')] != POS_NONE) {
				g = groups[2 * i + 1] - groups[2 * i];
				if(o)
					buf_copy(groups[2 * i], g, o + k);
				k += g;
				p++;
				continue;
			}
			if(*p == '\\' && (p[1] == 'n' || p[1] == 't' || p[1] == '\\')) {
				if(o)
					o[k] = p[1] == 'n' ? '\n' : p[1] == 't' ? '\t' : '\\';
				k++;
				p++;
				continue;
			}
			if(o)
				o[k] = *p;
			k++;
		}
		if(pass == 0 && (s = mem_alloc(k + 1)) == 0)
			return 0;
	}
	s[k] = 0;
	*n = k;
	return s;
}

/* --- find and replace ---------------------------------------------------- */

void
search_execute(int action)
{
	const char *s = st.needle.s;
	char *r;
	pos_t at, from, off, n = 0, nr;
	long count = 0;

	if(docs == 0)
		return;
	ui_again = 1;
	if(s[0] == 0) {		/* nothing to look for */
		if(has_sel())
			D->cur = sel_begin();
		D->anchor = POS_NONE;
		st.sfail = 0;
		hit = POS_NONE;
		return;
	}
	if(st.regex && !re_ready()) {	/* not an expression */
		st.sfail = 1;
		return;
	}
	if(action == REPLACE_ALL) {
		buf_begin();
		for(off = 0; off <= buf_len() && find_from(off, 0, &at, &n); ) {
			if((r = replacement(&nr)) == 0 || !edit(at, n, r, nr, U_OTHER)) {
				mem_free(r);
				break;
			}
			mem_free(r);
			count++;
			off = at + nr + (n == 0);	/* past an empty match */
		}
		st.sfail = count == 0;
		D->anchor = POS_NONE;
		if(D->cur > buf_len())
			D->cur = buf_len();
		cursor_visible();
		return;
	}
	if(action == REPLACE && has_sel() && hit == sel_begin() &&
	   match_at(hit, &n) && sel_end() - hit == n && (r = replacement(&nr)) != 0) {
		at = hit;
		write_text(r, nr, 0);
		mem_free(r);
		hit = POS_NONE;
		D->cur = at + nr;
	}
	if(has_sel() && hit == sel_begin())
		from = sel_end() + (sel_end() == hit);
	else
		from = has_sel() ? sel_begin() : D->cur;
	if(find_from(from, 1, &at, &n)) {
		/* the search starts: where it was, to go back to */
		if(!(has_sel() && hit == sel_begin()) &&
		   buf_linestart(at) != buf_linestart(D->cur))
			jump_push();
		D->anchor = at;
		D->cur = at + n;
		hit = at;
		st.sfail = 0;
	} else
		st.sfail = 1;
	D->pref = -1;
	cursor_visible();
}

void
draw_search(int y)
{
	int lw = st.search == SEARCH_REPLACE ? 9 : 6, x, act = -1, changed = 0, narrow;
	char *s;

	if(docs == 0) {
		st.search = SEARCH_HIDDEN;
		return;
	}
	if(st.search_focus) {
		st.search_focus = 0;
		ui_steal(ID_FIND);
		if((s = sel_text()) != 0) {
			if(!strchr(s, '\n')) {
				edl_set(&st.needle, s);
				hit = sel_begin();
				if(st.search == SEARCH_REPLACE)
					ui_steal(ID_REPLACE);
			}
			mem_free(s);
		}
	}
	ui_well_begin(0);
	scr_fill(0, y, scr_cols - 1, y + (st.search == SEARCH_REPLACE ? 2 : 1), ' ', S_PANEL);
	scr_text(0, y, "Find:", S_PANEL, scr_cols);
	if(ui_edit(ID_FIND, lw, y, scr_cols - lw, &st.needle, st.sfail ? S_FAIL : S_PANEL)) {
		hit = POS_NONE;
		act = SEARCH;
	}
	if(ui_focus == ID_FIND && ui_key == 13) {
		ui_key = 0;
		act = SEARCH;
	}
	if(st.search == SEARCH_REPLACE) {
		y++;
		scr_text(0, y, "Replace:", S_PANEL, scr_cols);
		ui_edit(ID_REPLACE, lw, y, scr_cols - lw, &st.repl, S_PANEL);
		if(ui_focus == ID_REPLACE && ui_key == 13) {
			ui_key = 0;
			act = REPLACE;
		} else if(ui_focus == ID_REPLACE && ui_key == (K_CTRL | K_ALT | 13)) {
			ui_key = 0;
			act = REPLACE_ALL;
		}
	}
	y++;
	x = 0;
	/* the options: shorter, closer, on a narrow screen */
	narrow = scr_cols < 50;
	changed |= ui_check(ID_CASE, x, y, narrow ? "Case" : "Match Case", &st.mcase, S_PANEL);
	x += text_width(narrow ? "Case" : "Match Case") + (narrow ? 6 : 7);
	changed |= ui_check(ID_WORD, x, y, narrow ? "Word" : "Whole Word", &st.wword, S_PANEL);
	x += text_width(narrow ? "Word" : "Whole Word") + (narrow ? 6 : 7);
	changed |= ui_check(ID_REGEX, x, y, narrow ? "Regex" : "Use Regex", &st.regex, S_PANEL);
	x += text_width(narrow ? "Regex" : "Use Regex") + (narrow ? 6 : 7);
	if(changed) {
		hit = POS_NONE;
		act = SEARCH;
		settings_find_changed();	/* kept for the next time */
	}
	if(st.search == SEARCH_REPLACE) {
		if(ui_button(ID_REPLALL, x, y, narrow ? "All" : "Replace All", 0, S_PANEL))
			act = REPLACE_ALL;
		x += text_width(narrow ? "All" : "Replace All") + (narrow ? 3 : 4);
	}
	if(ui_button(ID_CLOSE, x, y, "Close", 0, S_PANEL))
		st.search = SEARCH_HIDDEN;
	if(ui_well_end() && ui_key == 27) {
		ui_key = 0;
		st.search = SEARCH_HIDDEN;
		ui_again = 1;
	}
	if(act >= 0)
		search_execute(act);
}
