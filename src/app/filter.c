/* Filter Through Command (Alt+|): the selection given to a command of the
 * shell, and put back as it writes it (sort, fmt, column -t, jq .); with
 * no selection, what the command writes is put in at the cursor (date,
 * ls).  It runs in the folder of the document.  A command that fails
 * (its status is not 0) changes nothing: what it said is shown.  The
 * change is undone in one step. */
#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "../core/mem.h"

static struct edl cmdline;	/* the command, kept for the next time */

/* the first line of what the command said, for the message */
static void
say(int status, const char *e, size_t n)
{
	char msg[200];
	size_t i, k;

	if(status == -2) {
		error("The command ran too long and was stopped", 0);
		return;
	}
	if(status < 0) {
		error("The command could not be run", 0);
		return;
	}
	strcpy(msg, "The command failed: ");
	k = strlen(msg);
	for(i = 0; i < n && e[i] != '\n' && k < sizeof msg - 1; i++)
		msg[k++] = e[i] == '\t' || e[i] == '\r' ? ' ' : e[i];
	if(i == 0) {	/* it said nothing: its status */
		strcpy(msg, "The command failed with status ");
		k = strlen(msg);
		k += (size_t)udec(msg + k, (pos_t)status);
	}
	msg[k] = 0;
	error(msg, 0);
}

static void
run(const char *cmd)
{
	char dir[PATH_MAX_], *in = 0, *out, *err;
	size_t outn, errn, i, o;
	pos_t a = D->cur, n = 0;
	int status;

	dir[0] = 0;
	if(D->path) {	/* the document's folder */
		o = (size_t)(path_last(D->path) - D->path);
		if(o < sizeof dir) {
			memcpy(dir, D->path, o);
			dir[o] = 0;
		}
	}
	if(has_sel()) {
		a = sel_begin();
		n = sel_end() - a;
		if((in = mem_alloc(n)) == 0) {
			error("Out of memory", 0);
			return;
		}
		buf_copy(a, n, in);
	}
	status = plat_run(cmd, dir[0] ? dir : 0, in, (size_t)n, &out, &outn, &err, &errn);
	mem_free(in);
	if(status != 0)
		say(status, err, errn);
	else {
		for(i = o = 0; i < outn; i++)	/* CR LF as LF */
			if(!(out[i] == '\r' && i + 1 < outn && out[i + 1] == '\n'))
				out[o++] = out[i];
		buf_begin();
		if(edit(a, n, out ? out : "", (pos_t)o, U_OTHER)) {
			if(n) {		/* the result, selected */
				D->anchor = a;
				D->cur = a + (pos_t)o;
			} else {
				D->anchor = POS_NONE;
				D->cur = a + (pos_t)o;
			}
			B->merge = 0;
		}
		cursor_visible();
	}
	free(out);
	free(err);
}

void
draw_filter(void)
{
	int x, y, w = dialog_width(60), done = 0;
	const char *title = has_sel() ? "Filter Selection Through Command" :
	    "Insert Command Output";

	if(docs == 0) {
		st.wants_filter = 0;
		return;
	}
	ui_modal_begin(title, w, 4, S_FLOAT, &x, &y);
	ui_steal(ID_FILTER);
	ui_edit(ID_FILTER, x + 1, y + 1, w - 2, &cmdline, S_FLOAT);
	scr_text(x + 1, y + 2, has_sel() ? "The selection in, the result in its place (Ctrl+U clears)" :
	    "What it writes goes in at the cursor (Ctrl+U clears)", S_FLOAT | F_DIM, x + w - 1);
	if(ui_key == 13) {
		ui_key = 0;
		if(cmdline.len > 0) {
			cmdline.s[cmdline.len] = 0;
			done = 1;
		}
		ui_again = 1;
	}
	if(ui_modal_end() || done) {
		st.wants_filter = 0;
		rows_stale = 1;
		if(done)
			run(cmdline.s);
	}
}
