/* The modals: About, Go to Line:Column, Unsaved Changes, the
 * errors and Go to File; and what Save and Exit do. */
#include <string.h>
#include "app.h"
#include "../core/mem.h"

/* the words of s in lines of w columns from (x, y): the count of lines;
 * y < 0 only counts */
int
wrap_text(int x, int y, int w, const char *s, int at)
{
	const char *p = s, *brk, *q;
	int n = 0;
	char line[100];
	size_t k;

	while(*p) {
		brk = 0;
		for(q = p; *q && q - p < w; q++)
			if(*q == ' ')
				brk = q;
		if(*q == 0)
			brk = q;
		else if(brk == 0)
			brk = q;
		k = (size_t)(brk - p) < sizeof line ? (size_t)(brk - p) : sizeof line - 1;
		memcpy(line, p, k);
		line[k] = 0;
		if(y >= 0)
			scr_text(x + (w - text_width(line)) / 2, y + n, line, at, x + w);
		n++;
		p = *brk == ' ' ? brk + 1 : brk;
	}
	return n;
}

/* a row of buttons, centered from x0 in w columns, two blanks between */
int
buttons(int x0, int y, int w, const char *const *text, const char *accel,
    const int *ids, int n, int at)
{
	int i, tw = 0, x, r = -1;

	for(i = 0; i < n; i++)
		tw += text_width(text[i]) + 2 + (i ? 2 : 0);
	x = x0 + (w - tw) / 2;
	for(i = 0; i < n; i++) {
		if(ui_button(ids[i], x, y, text[i], accel ? accel[i] : 0, at))
			r = i;
		x += text_width(text[i]) + 4;
	}
	return r;
}

int
dialog_width(int want)
{
	return want > scr_cols ? scr_cols : want;
}

static const char *const btn_text[] = { "Ok", "Save", "Don't Save", "Cancel", "Yes", "No" };
static const int btn_ids[] = { ID_OK, ID_YES, ID_NO, ID_CANCEL, ID_YES, ID_NO };
static const char btn_first[] = { 0, 1, 4 }, btn_count[] = { 1, 3, 2 };
static const char *const btn_accel[] = { "", "SN", "YN" };

int
msgbox(const char *title, const char *const *line, int n, int set, int at)
{
	int w = 20, h = 0, i, x, y, r, nb = btn_count[set];
	const char *acc = btn_accel[set];

	for(i = 0; i < n; i++)
		if(text_width(line[i]) + 6 > w)
			w = text_width(line[i]) + 6;
	w = dialog_width(w);
	for(i = 0; i < n; i++)
		h += wrap_text(0, -1, w - 6, line[i], 0);
	ui_modal_begin(title, w, h + 6, at, &x, &y);
	for(y += 2, i = 0; i < n; i++)
		y += wrap_text(x + 3, y, w - 6, line[i], at);
	r = buttons(x + 1, y + 1, w - 2, btn_text + btn_first[set], set == MB_SAVE ? "SN\0" : 0,
	    btn_ids + btn_first[set], nb, at);
	for(i = 0; acc[i]; i++)
		if(ui_key == acc[i] || ui_key == (acc[i] | 0x20))
			r = i;
	if(ui_modal_end())
		r = nb - 1;
	return r;
}

void
draw_about(void)
{
	static const char *const text[] = {
		"Code Editor",
		"Version: 1.0",
		"Copyright (c) 2026 Leandro Vital"
	};

	if(msgbox("About", text, 3, MB_OK, S_FLOAT) >= 0)
		st.wants_about = 0;
}

/* line[:column], both from 1 */
static int
parse_goto(const char *s, pos_t *line, pos_t *col)
{
	pos_t v[2] = { 0, 1 };
	int i = 0, digits = 0;
	const char *p;

	for(p = s; *p; p++) {
		if(*p >= '0' && *p <= '9') {
			v[i] = v[i] * 10 + (pos_t)(*p - '0');
			digits++;
		} else if(*p == ':' && i == 0 && digits) {
			i = 1;
			v[1] = 0;
			digits = 0;
		} else if(*p != ' ')
			return 0;
	}
	if(!digits || v[0] < 1 || v[1] < 1)
		return 0;
	*line = v[0] - 1;
	*col = v[1] - 1;
	return 1;
}

void
draw_goto(void)
{
	int x, y, done = 0;
	pos_t line, col;

	if(docs == 0) {
		st.wants_goto = 0;
		return;
	}
	ui_modal_begin("Go to Line:Column" DOTS, dialog_width(26), 3, S_FLOAT, &x, &y);
	ui_steal(ID_GOTO);
	if(ui_edit(ID_GOTO, x + 1, y + 1, dialog_width(26) - 2, &st.gototarget,
	    st.goto_invalid ? S_FAIL : S_FLOAT))
		st.goto_invalid = 0;
	if(ui_key == 13) {
		ui_key = 0;
		if(parse_goto(st.gototarget.s, &line, &col)) {
			cmd_goto(line, col);
			cursor_visible();
			done = 1;
		} else
			st.goto_invalid = 1;
		ui_again = 1;
	}
	if(ui_modal_end() || done) {
		st.wants_goto = 0;
		edl_set(&st.gototarget, "");
		st.goto_invalid = 0;
	}
}

/* the file of the active document changed on the disk (and it has no
 * unsaved changes): Yes reads it again, No keeps the text as it is */
void
draw_reload(void)
{
	static char line[PATH_MAX_ + 64];
	const char *msg[2];
	int r;

	if(docs == 0 || D->path == 0 || buf_dirty(B)) {
		st.wants_reload = 0;
		return;
	}
	strcpy(line, D->name);
	strcat(line, " was changed by another program.");
	msg[0] = line;
	msg[1] = "Do you want to reload it?";
	r = msgbox("File Changed", msg, 2, MB_YESNO, S_ALERT);
	if(r < 0)
		return;
	st.wants_reload = 0;
	ui_again = 1;
	if(r == 0) {
		doc_reload(D->path);
		cursor_visible();
	}
}

void
draw_unsaved(void)
{
	static const char *const msg[] = { "Do you want to save the changes you made?" };
	int act;

	if(docs == 0) {
		st.wants_close = 0;
		return;
	}
	if(!buf_dirty(B)) {
		act = 1;
		ui_again = 1;
	} else
		act = msgbox("Unsaved Changes", msg, 1, MB_SAVE, S_ALERT);
	if(act == 0)
		st.wants_save = 1;
	else if(act == 2)
		st.wants_close = st.wants_exit = 0;
	if(act >= 0)
		ui_again = 1;
	if(act != 1)
		return;
	doc_close();
	st.wants_close = 0;
}

void
draw_errors(void)
{
	int i;

	if(msgbox("Error", (const char *const *)st.errors, st.nerr, MB_OK, S_ALERT) >= 0) {
		for(i = 0; i < st.nerr; i++)
			mem_free(st.errors[i]);
		st.nerr = 0;
		ui_again = 1;
	}
}

/* --- Go to File ---------------------------------------------------------- */

static char item[PATH_MAX_ + 48];

static struct doc *
nthdoc(int i)
{
	struct doc *d = docs;

	while(d && i--)
		d = d->next;
	return d;
}

static const char *
doc_item(int i)
{
	struct doc *d = nthdoc(i);
	size_t n;

	strcpy(item, buf_dirty(DBUF(d)) ? "* " : "  ");
	strcat(item, d->name);
	if(d->path) {
		strcat(item, "   ");
		n = strlen(item);
		strncat(item, d->path, sizeof item - n - 1);
		n = strlen(item) - strlen(d->name);
		if(n > 0 && strcmp(item + n, d->name) == 0)
			item[n] = 0;	/* the directory alone */
	}
	return item;
}

void
draw_gotofile(void)
{
	static int sel, top;
	int w = scr_cols - 20, h = scr_rows - 10, x, y;

	if(w < 10)
		w = scr_cols < 10 ? scr_cols : 10;
	if(h < 3)
		h = 3;
	ui_modal_begin("Go to File" DOTS, w + 2, h + 2, S_FLOAT, &x, &y);
	if(ui_list(ID_LIST, x + 1, y + 1, w, h, ndocs, doc_item, &sel, &top) == 2) {
		doc_activate(nthdoc(sel));
		st.wants_gotofile = 0;
		sel = top = 0;
	}
	if(ui_modal_end()) {
		st.wants_gotofile = 0;
		sel = top = 0;
	}
}


/* before a folder or a file of the file picker goes to the Trash (Del,
   [Trash]) */
void
draw_trash(void)
{
	static char line[PATH_MAX_ + 64];
	const char *msg[2];
	int r;

	strcpy(line, file_kind(st.trash) == 2 ? "Move the folder \"" : "Move the file \"");
	strcat(line, path_last(st.trash));
	strcat(line, "\" to the Trash?");
	msg[0] = line;
	msg[1] = "It can be restored from the Trash of the system.";
	r = msgbox("Move to Trash", msg, 2, MB_YESNO, S_ALERT);
	if(r < 0)
		return;
	if(r == 0)
		picker_trash(st.trash);
	mem_free(st.trash);
	st.trash = 0;
	ui_steal(ID_P_LIST);
	ui_again = 1;
}

/* the warning before Save As writes over a file */
void
draw_overwrite(void)
{
	static const char *const msg = "File already exists. Do you want to overwrite it?";
	char path[PATH_MAX_ + 2];
	int r;

	r = msgbox("Confirm Save As", &msg, 1, MB_YESNO, S_ALERT);
	if(r == 0) {
		strcpy(path, st.overwrite);
		mem_free(st.overwrite);
		st.overwrite = 0;
		picker_finish(path);
		return;
	}
	if(r == 1) {
		mem_free(st.overwrite);
		st.overwrite = 0;
		ui_steal(ID_P_NAME);
	}
}
