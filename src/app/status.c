/* The status bar: the language, the newlines (a
 * button that switches them), the encoding (a button that opens its
 * popup: Reopen with encoding, Convert to encoding),
 * the indentation (a button that opens its picker), the project (a
 * button: Open Project Folder), the line and column, OVR, the * of a changed
 * text and the name of the file (a button: Go to File). */
#include <stdlib.h>
#include <string.h>
#include "finder.h"

static int indentx;		/* where the indentation button is */
static int encx;		/* where the encoding button is */

static const char *const widths[] = { "1", "2", "3", "4", "5", "6", "7", "8" };
static const char *const types[] = { "Tabs", "Spaces" };

static const char *
width_item(int i)
{
	return widths[i];
}

static const char *
type_item(int i)
{
	return types[i];
}


#define num(s, n)	udec(s, n)

char bar_stale = 1;		/* the bar is drawn again */
pos_t cur_line = POS_NONE;	/* the line of the cursor and where it starts,
				   as the text area found them; POS_NONE: not */
pos_t cur_lstart = POS_NONE;

/* the buttons of the bar: where, and which */
#define NBTN		6
static struct {
	int id, x, w, at;	/* at: their bytes in the bar */
} btn[NBTN];
static int nbtn;
#define BARMAX		1100
static char bar[BARMAX];
static int lastfocus;
static char lastname[40];
static unsigned char lastlang;
static int lc_x, lc_len;	/* where the line and column are */

/* s to the bar at column x (byte at p): a button when id; then gap blanks */
static int
put(int *p, int x, int id, const char *s, int gap)
{
	int n = (int)strlen(s), w = text_width(s), k = *p;

	if(id && nbtn < NBTN) {
		btn[nbtn].id = id;
		btn[nbtn].x = x;
		btn[nbtn].w = w + 2;
		btn[nbtn++].at = k;
		bar[k++] = '[';
	}
	memcpy(bar + k, s, n);
	k += n;
	if(id)
		bar[k++] = ']';
	memset(bar + k, ' ', gap);
	*p = k + gap;
	return x + w + (id ? 2 : 0) + gap;
}

/* The bar, fitted to the screen: the line and column, OVR, the *
 * and the name always; then, while there is room, the newlines, the
 * indentation (shorter when narrow), the encoding and the language.  It
 * is put together as one line and drawn at once, when it changed. */
void
draw_statusbar(int y)
{
	char lc[24], ind[12], name[44], enc[24], proj[24];
	char lng[24];
	int x, p, w, room, nl, in, en, la, pr, pw = 0, iw, nw, ovr, gap, i, focus, hit = 0;
	pos_t ls, col;

	if(docs == 0) {
		scr_fill(0, y, scr_cols - 1, y, ' ', S_BAR);
		st.wants_status = st.wants_indent = st.wants_enc = 0;
		return;
	}
	/* the line: when counted, or quick to count; else how far the count
	   is (POS_NONE - the percent) */
	if(!D->linenums)
		cur_line = 0;
	else if(!buf_lineno_try(D->cur, (pos_t)4 << 20, &cur_line))
		cur_line = POS_NONE - (pos_t)(buf_len() ? buf_counted() / (buf_len() / 100 + 1) : 0);
	/* the column, in characters: from where it was last on the same
	   line, if the text did not change */
	ls = buf_linestart(D->cur);
	{
		static struct buf *cb;
		static pos_t cls, coff, ccol;
		static unsigned cgen;

		if(cb == B && cls == ls && cgen == edit_gen)
			col = D->cur >= coff ? ccol + buf_chars(coff, D->cur) :
			    ccol - buf_chars(D->cur, coff);
		else
			col = buf_chars(ls, D->cur);
		cb = B;
		cls = ls;
		coff = D->cur;
		ccol = col;
		cgen = edit_gen;
	}
	{	/* nothing it shows changed: nothing to do */
		static pos_t l_line, l_col;
		static int l_bits;
		static unsigned l_project;
		int bits = B->crlf | B->tabs << 1 | D->overtype << 2 | buf_dirty(B) << 3 |
		    B->tabsize << 4 | B->enc << 8 | ndocs << 15;

		int same = !bar_stale && !st.wants_status && bits == l_bits &&
		    ui_focus == lastfocus && !(ui_focus >= ID_ST_NL && ui_focus <= ID_ST_OVR) &&
		    strcmp(D->name, lastname) == 0
		    && D->lang == lastlang
		    && project_gen == l_project
		    && ui_key != K_MOUSE
		    ;

		if(same && cur_line == l_line && col == l_col)
			return;
		if(!D->linenums)
			strcpy(lc, "Col ");
		else if(cur_line > POS_NONE - 101) {	/* still counting */
			strcpy(lc, DOTS);
			num(lc + strlen(lc), POS_NONE - cur_line);
			strcat(lc, "%:");
		} else {
			num(lc, cur_line + 1);
			strcat(lc, ":");
		}
		num(lc + strlen(lc), col + 1);
		if(same && (int)strlen(lc) == lc_len) {
			/* only the line and column, as long: put where they were */
			l_line = cur_line;
			l_col = col;
			scr_ntext(lc_x, y, lc, lc_len, S_BAR, scr_cols);
			return;
		}
		l_line = cur_line;
		l_col = col;
		l_bits = bits;
		l_project = project_gen;
		strcpy(lastname, D->name);
		lastlang = D->lang;
	}
	strcpy(name, D->name);
	if(ndocs > 1) {
		strcat(name, " + ");
		num(name + strlen(name), (pos_t)ndocs - 1);
	}
	nw = text_width(name) + 2;
	ovr = D->overtype;
	gap = scr_cols < 60 ? 1 : 2;
	room = scr_cols - 2 - ((int)strlen(lc) + gap) - (ovr ? 5 + gap : 0) -
	    (buf_dirty(B) ? 1 + gap : 0) - (nw < 12 ? nw : 12);
	nl = room >= (w = (B->crlf ? 6 : 4) + gap);
	if(nl)
		room -= w;
	strcpy(ind, B->tabs ? "Tabs:" : "Spaces:");
	num(ind + strlen(ind), (pos_t)B->tabsize);
	if(room < (iw = text_width(ind) + 2 + gap)) {
		strcpy(ind, B->tabs ? "Tab:" : "Spc:");	/* narrow */
		num(ind + strlen(ind), (pos_t)B->tabsize);
		iw = text_width(ind) + 2 + gap;
	}
	in = room >= iw;
	if(in)
		room -= iw;
	strcpy(enc, enc_name(B->enc));
	en = room >= (w = text_width(enc) + 2 + gap);
	if(en)
		room -= w;
	lng[0] = '[';
	strcpy(lng + 1, lang_name(D->lang));
	strcat(lng, "]");
	la = room >= (w = (int)strlen(lng) + gap);
	if(la)
		room -= w;
	/* the project, at the right before the file's name: the last to
	   have room, cut to 20 columns at most */
	proj[0] = 0;
	if(project_name[0]) {
		for(i = 0; project_name[i] && i < (int)sizeof proj - 1; i++)
			proj[i] = project_name[i];
		proj[i] = 0;
		while(text_width(proj) > 20 && i > 0)
			proj[--i] = 0;
		pw = text_width(proj) + 2 + gap;
	}
	pr = la && proj[0] && room >= pw;	/* after the language */

	nbtn = 0;
	p = 1;
	bar[0] = ' ';
	x = 1;
	if(la)
		x = put(&p, x, 0,
		    lng
		    , gap);
	if(nl)
		x = put(&p, x, ID_ST_NL, B->crlf ? "CRLF" : "LF", gap);
	if(en) {
		encx = x;
		x = put(&p, x, ID_ST_ENC, enc, gap);
	} else
		encx = 1;
	if(in) {
		indentx = x;
		x = put(&p, x, ID_ST_INDENT, ind, gap);
	}
	lc_x = x;
	lc_len = (int)strlen(lc);
	x = put(&p, x, 0, lc, gap);
	if(ovr)
		x = put(&p, x, ID_ST_OVR, "OVR", gap);
	if(buf_dirty(B))
		x = put(&p, x, 0, "*", gap);
	w = scr_cols - 2 - x - (pr ? pw : 0);	/* the name, at the right */
	if(w < 3)
		w = 3;
	if(nw > w) {
		name[w - 2] = 0;	/* cut: the end is lost */
		nw = w;
	}
	i = scr_cols - 1 - nw - x - (pr ? pw : 0);	/* the blanks before them */
	if(i > 0) {
		memset(bar + p, ' ', i);
		p += i;
		x += i;
	}
	if(pr)
		x = put(&p, x, ID_ST_PROJECT, proj, gap);
	put(&p, x, ID_ST_FILE, name, 1);
	bar[p] = 0;

	/* the focus and the mouse: only when the bar has the one, or the
	   other may click it */
	focus = ui_focus >= ID_ST_NL && ui_focus <= ID_ST_OVR;
	if(st.wants_status) {
		st.wants_status = 0;
		ui_steal(nl ? ID_ST_NL : ID_ST_FILE);
		focus = 1;
	}
	focus |= ui_key == K_MOUSE;
	if(focus) {
		ui_well_begin(0);
		for(i = 0; i < nbtn; i++)
			if(ui_focusable(btn[i].id, W_BUTTON, btn[i].x, y, btn[i].x + btn[i].w - 1) &&
			   (ui_key == 13 || ui_key == ' '))
				hit = btn[i].id;
		for(i = 0; i < nbtn; i++)
			if(ui_mouse(btn[i].x, y, btn[i].x + btn[i].w - 1))
				hit = btn[i].id;
		if(hit)
			ui_key = 0;
		if(ui_well_end() && ui_key == 27) {
			ui_key = 0;
			ui_steal(ID_EDITOR);
		}
	}
	/* drawn (only what changed is sent) */
	{
		bar_stale = 0;
		lastfocus = ui_focus;
		scr_text(0, y, bar, S_BAR, scr_cols);
		for(i = 0; i < nbtn; i++)
			if(btn[i].id == ui_focus)
				scr_ntext(btn[i].x, y, bar + btn[i].at,
				    (int)(strchr(bar + btn[i].at, ']') - (bar + btn[i].at)) + 1,
				    S_BAR | F_REV, scr_cols);
	}
	switch(hit) {
	case ID_ST_NL:
		B->crlf = !B->crlf;
		buf_touch();
		break;
	case ID_ST_ENC:
		st.wants_enc = !st.wants_enc;
		st.wants_indent = 0;
		break;
	case ID_ST_INDENT:
		st.wants_indent = !st.wants_indent;
		st.wants_enc = 0;
		break;
	case ID_ST_OVR:
		D->overtype = 0;
		break;
	case ID_ST_PROJECT:
		picker_open(PICK_FOLDER);
		break;
	case ID_ST_FILE:
		st.wants_gotofile = 1;
		break;
	}
	if(hit)
		ui_again = 1;
}

/* the indentation: tabs or spaces, and how wide (a popup over the bar) */
void
draw_indent_picker(int y)
{
	int x = indentx, sel, top = 0, h = 8, in;

	static char shown;

	if(!st.wants_indent || docs == 0) {
		st.wants_indent = shown = 0;
		return;
	}
	if(x + 21 > scr_cols)
		x = scr_cols - 21;
	if(x < 0)
		x = 0;
	ui_well_begin(0);
	if(!shown)
		ui_steal(ID_IND_TYPE);
	shown = 1;
	scr_fill(x, y - h - 2, x + 20, y - 1, ' ', S_FLOAT);
	scr_frame(x, y - h - 2, x + 20, y - 1, S_FLOAT);
	sel = B->tabs ? 0 : 1;
	if(ui_list(ID_IND_TYPE, x + 2, y - h - 1, 9, 2, 2, type_item, &sel, &top) == 1)
		B->tabs = sel == 0;
	sel = B->tabsize - 1;
	top = 0;
	if(ui_list(ID_IND_WIDTH, x + 13, y - h - 1, 6, h, 8, width_item, &sel, &top) == 1)
		B->tabsize = (char)(sel + 1);
	in = ui_well_end();
	if(in && (ui_key == 13 || ui_key == 27)) {
		ui_key = 0;
		st.wants_indent = shown = 0;
		ui_steal(ID_ST_INDENT);
	} else if(!in || (ui_key == K_MOUSE && term_mouse.kind == M_PRESS)) {
		st.wants_indent = shown = 0;	/* the focus went: it closes */
		ui_again = 1;
	}
}

/* --- the encoding ----------------------------------------------------------
 *
 * Its button opens a popup over it with two actions; each opens the list
 * of the encodings (Find: narrows it, a fuzzy match), and the one chosen
 * is applied:
 *
 *   Reopen with encoding   the file read again in it (a file saved, with
 *                          no changes)
 *   Convert to encoding    the text kept, written in it at the next save
 *
 * An untitled document has nothing to read again: no popup, the list of
 * Convert opens at once. */

#define ENC_REOPEN_T	"Reopen with encoding" DOTS
#define ENC_CONVERT_T	"Convert to encoding" DOTS

static void
enc_dialog_open(int kind)
{
	st.wants_enc = 0;
	st.enc_dialog = (char)kind;
	edl_set(&st.enc_needle, "");
	ui_steal(ID_ENC_FIND);
	ui_again = 1;
}

/* the popup over the bar: the two actions (an untitled document: none,
 * the list of Convert at once) */
void
draw_enc_picker(int y)
{
	int w = text_width(ENC_REOPEN_T) + 6, x = encx, in, up, down;

	static char shown;

	if(!st.wants_enc || docs == 0) {
		st.wants_enc = shown = 0;
		return;
	}
	if(D->path == 0) {	/* untitled: nothing to read again */
		shown = 0;
		enc_dialog_open(ENCD_CONVERT);
		return;
	}
	if(x + w > scr_cols)
		x = scr_cols - w;
	if(x < 0)
		x = 0;
	ui_well_begin(0);
	if(!shown)
		ui_steal(ID_ENC_REOPEN);
	shown = 1;
	/* Up and Down from one to the other, as Tab does */
	up = ui_key == K_UP && ui_focus == ID_ENC_CONVERT;
	down = ui_key == K_DOWN && ui_focus == ID_ENC_REOPEN;
	if(up || down) {
		ui_key = 0;
		ui_steal(up ? ID_ENC_REOPEN : ID_ENC_CONVERT);
		ui_again = 1;
	}
	scr_fill(x, y - 4, x + w - 1, y - 1, ' ', S_FLOAT);
	scr_frame(x, y - 4, x + w - 1, y - 1, S_FLOAT);
	if(ui_button(ID_ENC_REOPEN, x + 2, y - 3, ENC_REOPEN_T, 0, S_FLOAT)) {
		shown = 0;
		enc_dialog_open(ENCD_REOPEN);
		ui_well_end();
		return;
	}
	if(ui_button(ID_ENC_CONVERT, x + 2, y - 2, ENC_CONVERT_T, 0, S_FLOAT)) {
		shown = 0;
		enc_dialog_open(ENCD_CONVERT);
		ui_well_end();
		return;
	}
	in = ui_well_end();
	if(in && ui_key == 27) {
		ui_key = 0;
		st.wants_enc = shown = 0;
		bar_stale = 1;
		ui_steal(ID_ST_ENC);
		ui_again = 1;
	} else if(!in || (ui_key == K_MOUSE && term_mouse.kind == M_PRESS)) {
		st.wants_enc = shown = 0;	/* the focus went: it closes */
		ui_again = 1;
	}
}

static int eall[ENC_LIST_MAX], nall;	/* the encodings that can be chosen */
static int erow[ENC_LIST_MAX], nerow, escore[ENC_LIST_MAX];

static int
by_escore(const void *a, const void *b)
{
	int x = *(const int *)a, y = *(const int *)b;

	if(escore[x] != escore[y])
		return escore[x] > escore[y] ? -1 : 1;
	return x - y;	/* as listed */
}

/* the rows: all (indexes of eall), or those Find: matches, the best
 * first */
static void
enc_rows(void)
{
	const char *name;
	int i;

	if(nall == 0)
		nall = enc_list(eall, ENC_LIST_MAX);
	nerow = 0;
	st.enc_needle.s[st.enc_needle.len] = 0;
	if(st.enc_needle.len == 0 || !fz_set(st.enc_needle.s)) {
		for(i = 0; i < nall; i++)
			erow[nerow++] = i;
		return;
	}
	for(i = 0; i < nall; i++) {
		name = enc_name(eall[i]);
		if((escore[i] = fz_score(name, (int)strlen(name))) >= 0)
			erow[nerow++] = i;
	}
	qsort(erow, (size_t)nerow, sizeof *erow, by_escore);
}

static const char *
enc_row(int r)
{
	return enc_name(eall[erow[r]]);
}

/* the list of the encodings, for Reopen or Convert (a modal as wide as
 * the screen but 20 columns, as high but 10 rows: as edit has it) */
void
draw_enc_dialog(void)
{
	int lw = scr_cols - 20, lh = scr_rows - 10, x, y, fw = 6, pick = -1, done = 0, e;
	kcode k;

	static char shown;
	static int sel, top;

	if(docs == 0) {
		st.enc_dialog = shown = 0;
		return;
	}
	if(lw < 10)
		lw = 10;
	if(lw > scr_cols - 2)
		lw = scr_cols - 2;
	if(lh < 10)
		lh = 10;
	if(lh > scr_rows - 5)
		lh = scr_rows - 5 < 1 ? 1 : scr_rows - 5;
	if(!shown) {	/* the current one chosen */
		enc_rows();
		for(sel = 0; sel < nerow && eall[erow[sel]] != B->enc; sel++)
			;
		if(sel >= nerow)
			sel = 0;
		top = sel >= lh ? sel - lh / 2 : 0;
		ui_steal(ID_ENC_FIND);
	}
	shown = 1;
	ui_modal_begin(st.enc_dialog == ENCD_REOPEN ? ENC_REOPEN_T : ENC_CONVERT_T,
	    lw + 2, lh + 3, S_FLOAT, &x, &y);
	/* a character typed in the list: to Find:; the keys of the list from
	   Find: too */
	k = ui_key;
	if(ui_focus == ID_ENC_LIST && k >= ' ' && k < 0x110000 && k != 127)
		ui_steal(ID_ENC_FIND);
	if(ui_focus == ID_ENC_FIND && nerow) {
		if(k == K_UP || k == (K_CTRL | 'P'))
			sel = sel ? sel - 1 : nerow - 1;
		else if(k == K_DOWN || k == (K_CTRL | 'N'))
			sel = sel + 1 < nerow ? sel + 1 : 0;
		else if(k == K_PGUP)
			sel = sel > lh - 1 ? sel - (lh - 1) : 0;
		else if(k == K_PGDN)
			sel = sel + lh - 1 < nerow ? sel + lh - 1 : nerow - 1;
		else if(k == 13)
			pick = sel;
		else
			k = 0;
		if(k) {
			ui_key = 0;
			ui_again = 1;
		}
	}
	scr_text(x + 1, y + 1, "Find:", S_FLOAT, x + 1 + fw);
	if(ui_edit(ID_ENC_FIND, x + 1 + fw, y + 1, lw - fw, &st.enc_needle, S_FLOAT)) {
		enc_rows();
		sel = top = 0;
	}
	if(ui_list(ID_ENC_LIST, x + 1, y + 2, lw, lh, nerow, enc_row, &sel, &top) == 2)
		pick = sel;
	if(nerow == 0)
		scr_text(x + 3, y + 2, "No encoding matches", S_LIST | F_DIM, x + lw);
	if(ui_modal_end())
		done = 1;
	if(pick >= 0 && pick < nerow) {
		e = eall[erow[pick]];
		if(st.enc_dialog == ENCD_REOPEN)
			doc_reopen(e);
		else if(B->enc != e) {	/* written so at the next save */
			B->enc = (char)e;
			B->nobom = 0;
			buf_touch();
		}
		done = 1;
	}
	if(done) {
		st.enc_dialog = shown = 0;
		edl_set(&st.enc_needle, "");
		bar_stale = 1;
		ui_steal(ID_EDITOR);
		ui_again = 1;
	}
}
