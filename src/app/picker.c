/* The file picker of Open File, Save As and Open Project Folder: the
 * folder, the name being typed (with the names that start with it, as a
 * list under it), the entries of the folder (.., the directories, the
 * files; for Open Project Folder, the directories alone) and the warning before a
 * file is written over.  At the root of
 * a drive, .. lists the drives (on a platform that has them). */
#include <stdlib.h>
#include <string.h>
#include "finder.h"
#include "../core/mem.h"


/* the state, in pickfile.c */
#define names		pk_names
#define ents		pk_ents
#define nents		pk_n
#define loaded		pk_loaded
#define sel		pk_sel
#define top		pk_top
static int sugg[5], nsugg, ssel, stop_;

static const char *
ent(int i)
{
	return names + ents[i];
}

static int
lower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

static int
cmpname(const char *a, const char *b)
{
	for(; *a && lower((unsigned char)*a) == lower((unsigned char)*b); a++, b++)
		;
	return lower((unsigned char)*a) - lower((unsigned char)*b);
}

/* .. first, then the directories, then the files: by name */
static int
rank(const char *s)
{
	size_t n = strlen(s);

	if(strcmp(s, "..") == 0)
		return 0;
	return n && (s[n - 1] == '/' || s[n - 1] == '\\' || s[n - 1] == ':') ? 1 : 2;
}

static int
before(int i, int j)
{
	int a = rank(ent(i)), b = rank(ent(j));

	return a != b ? a < b : cmpname(ent(i), ent(j)) < 0;
}

static void
add(const char *s, unsigned *used)
{
	size_t n = strlen(s) + 1;

	if(nents >= ENT_MAX || *used + n > NAMES_MAX)
		return;
	ents[nents++] = *used;
	memcpy(names + *used, s, n);
	*used += (unsigned)n;
}

static void
refresh(void)
{
	char name[PATH_MAX_ + 2];
	unsigned used = 0;
	int i, isdir;
	unsigned t;
	int j;
	unsigned drives;
	size_t n;

	loaded = 1;
	nents = sel = top = 0;
	nsugg = 0;
	if(names == 0) {
		names = mem_alloc(NAMES_MAX);
		ents = mem_alloc(ENT_MAX * sizeof(unsigned));
		if(names == 0 || ents == 0) {
			mem_free(names);
			mem_free(ents);
			names = 0;
			ents = 0;
			error("Out of memory", 0);
			return;
		}
	}
	if(st.pdir[0] == 0) {	/* the drives */
		drives = plat_drives();
		for(i = 0; i < 26; i++)
			if(drives & (1u << i)) {
				name[0] = (char)('A' + i);
				strcpy(name + 1, ":\\");
				add(name, &used);
			}
		return;
	}
	n = strlen(st.pdir);
	if(plat_drives() || !(n == 1 && st.pdir[0] == path_sep))
		add("..", &used);
	if(dir_open(st.pdir)) {
		while(dir_next(name, PATH_MAX_, &isdir)) {
			if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
				continue;
			if(name[0] == '.' && !st.phidden)	/* hidden: Ctrl+H */
				continue;
			if(!isdir && st.picker == PICK_FOLDER)
				continue;
			if(isdir) {
				n = strlen(name);
				name[n] = path_sep;
				name[n + 1] = 0;
			}
			add(name, &used);
		}
		dir_close();
	}
	for(i = 1; i < nents; i++)	/* an insertion sort: few entries */
		for(j = i; j > 0 && before(j, j - 1); j--) {
			t = ents[j];
			ents[j] = ents[j - 1];
			ents[j - 1] = t;
		}
}

static void
suggest(void)
{
	int i;
	size_t n = (size_t)st.pname.len;

	nsugg = ssel = stop_ = 0;
	if(n == 0)
		return;
	for(i = 0; i < nents && nsugg < 5; i++)
		if(rank(ent(i)) && strlen(ent(i)) > n) {
			const char *e = ent(i);
			size_t k;

			for(k = 0; k < n && lower((unsigned char)e[k]) == lower((unsigned char)st.pname.s[k]); k++)
				;
			if(k == n)
				sugg[nsugg++] = i;
		}
}

static const char *
sugg_item(int i)
{
	return ent(sugg[i]);
}


/* the name typed, on the folder: a new folder, or a file to do it with
 * (1) */
static int
update_path(char *path)
{
	size_t n = strlen(st.pdir);
	const char *name = st.pname.s;

	if(st.pdir[0] && strcmp(name, "..") == 0 && plat_drives() && n == 3 && st.pdir[1] == ':') {
		st.pdir[0] = 0;	/* the root of a drive: the drives */
		edl_set(&st.pname, "");
		loaded = 0;
		return 0;
	}
	path_join(path, st.pdir, name);
	path_normalize(path);
	if(file_kind(path) == 2) {
		n = strlen(path);
		if(n && path[n - 1] != path_sep && n + 1 < PATH_MAX_) {
			path[n] = path_sep;
			path[n + 1] = 0;
		}
		strcpy(st.pdir, path);
		edl_set(&st.pname, "");
		loaded = 0;
		return 0;
	}
	n = strlen(st.pname.s);
	if(n == 0)
		return 0;
	if(st.pname.s[n - 1] == path_sep) {
		error("No such directory: ", path);
		return 0;
	}
	/* a name with a directory in it: the picker goes there */
	n = path_last(path) - path;
	if(n && n < PATH_MAX_) {
		memcpy(st.pdir, path, n);
		st.pdir[n] = 0;
	}
	return 1;
}

/* the hidden files shown or not: the folder listed again, and kept */
static void
hidden_changed(void)
{
	loaded = 0;
	ui_again = 1;
	settings_find_changed();
}

/* --- Open File and Open Project Folder -----------------------------------
 *
 * The entries of the folder shown: its folders and files (Open File), or
 * its folders alone, the first row "./" being that folder itself (Open
 * Project Folder).  What is typed narrows the list (a fuzzy match, as
 * Alt+P has it), the arrows choose, and the keys say what to do with the
 * one chosen:
 *
 *   Enter       a folder: into it, as a file manager does (..: up); a
 *               file: it is opened; ./: the folder shown is the project
 *   Alt+Enter   Open Project Folder: the one chosen is the project, not
 *               gone into
 *   Backspace   up, with nothing typed (Alt+Up too)
 *   name/       into that folder; ~/ the home; / the root
 *   a name      nothing matching it, Enter: that folder gone into, or
 *               that file opened (a new one, if there is none)
 *
 * Tab and Shift+Tab go from one item to the next, as in each dialog: the
 * search, the list, Hidden, [Open This Folder].
 *
 * The mouse: a click chooses, a double click opens it (a folder: into it),
 * [Open This Folder] takes the folder shown.  Save As is the dialog of
 * before: a name is typed there. */

#define HERE		(-1)	/* the row of the folder shown */

static int frow[ENT_MAX + 1], nfrow, fsel, ftop, fscore[ENT_MAX];

static int
by_fscore(const void *a, const void *b)
{
	int x = *(const int *)a, y = *(const int *)b;

	if(fscore[x] != fscore[y])
		return fscore[x] > fscore[y] ? -1 : 1;
	return x - y;	/* as listed: by name */
}

/* the rows: all, or those the text typed matches, the best first */
static void
folder_rows(void)
{
	const char *e;
	int i, n;

	nfrow = fsel = ftop = 0;
	if(st.pname.len == 0) {
		if(st.picker == PICK_FOLDER)
			frow[nfrow++] = HERE;
		for(i = 0; i < nents; i++)
			frow[nfrow++] = i;
		if(nfrow > 1 && strcmp(ent(0), "..") == 0)
			fsel = 0;
		return;
	}
	st.pname.s[st.pname.len] = 0;
	fz_set(st.pname.s);
	for(i = 0; i < nents; i++) {
		e = ent(i);
		if(strcmp(e, "..") == 0)
			continue;
		n = (int)strlen(e);
		if(n > 0 && e[n - 1] == path_sep)
			n--;	/* the name, not its separator */
		if((fscore[i] = fz_score(e, n)) >= 0)
			frow[nfrow++] = i;
	}
	qsort(frow, (size_t)nfrow, sizeof *frow, by_fscore);
}

static const char *
row_name(int r)
{
	return frow[r] == HERE ? "./" : ent(frow[r]);
}

/* the folder shown is now dir (ended by its separator, or not) */
static void
folder_go(const char *dir)
{
	size_t n = strlen(dir);

	if(n + 2 > sizeof st.pdir)
		return;
	memcpy(st.pdir, dir, n + 1);
	if(n == 0 || st.pdir[n - 1] != path_sep) {
		st.pdir[n] = path_sep;
		st.pdir[n + 1] = 0;
	}
	edl_set(&st.pname, "");
	loaded = 0;
}

/* into the folder name (of the folder shown; .. up): 0 if it is none */
static int
folder_into(const char *name)
{
	char path[PATH_MAX_ + 2];

	edl_set(&st.pname, name);
	return update_path(path) == 0 && st.pname.len == 0;
}

/* what was typed ends with a separator: into that folder (~ the home,
 * nothing before it the root) */
static void
folder_typed_sep(void)
{
	char name[EDL_MAX], root[2];
	const char *home;
	int n = st.pname.len - 1;

	memcpy(name, st.pname.s, (size_t)n);
	name[n] = 0;
	if(n == 0) {
		root[0] = path_sep;
		root[1] = 0;
		folder_go(root);
	} else if(strcmp(name, "~") == 0 && (home = getenv("HOME")) != 0)
		folder_go(home);
	else if(!folder_into(name)) {
		st.pname.s[n] = 0;	/* none of that name: the separator goes */
		st.pname.len = st.pname.cur = n;
		folder_rows();
	}
}

/* row r taken: a folder gone into, or (project) the project (./ always;
 * another with Alt+Enter); a file opened */
static void
folder_take(int r, int project)
{
	char path[PATH_MAX_ + 2];
	const char *e;
	size_t n;

	if(r < 0 || r >= nfrow)
		return;
	if(frow[r] == HERE) {
		if(project) {
			strcpy(path, st.pdir);
			picker_finish(path);
		}
		return;
	}
	e = ent(frow[r]);
	n = strlen(e);
	if(strcmp(e, "..") == 0 || (!project && n > 0 && e[n - 1] == path_sep)) {
		folder_into(e);
		return;
	}
	path_join(path, st.pdir, e);
	path_normalize(path);
	picker_finish(path);
}

static void
draw_browse(int w, int h)
{
	static const char *const hint =
	    "Enter: go in   ./: open here   Alt+Enter: open";
	static const char *const fhint = "Enter: open   Backspace: up";
	static char shown;	/* opened: the search has the focus first */
	int folders = st.picker == PICK_FOLDER;
	int x, y, lw = 11, lh = h - 6, i, r, cur, act = 0, row = 0, at, inlist, lf;
	kcode k;

	if(!loaded) {
		refresh();
		folder_rows();
	}
	ui_modal_begin(folders ? "Open Project Folder" DOTS : "Open File" DOTS, w, h, S_FLOAT,
	    &x, &y);
	if(!shown)
		ui_steal(ID_P_NAME);
	shown = 1;
	/* the list has the focus and a character is typed: to the search */
	k = ui_key;
	if(ui_focus == ID_P_LIST && k >= ' ' && k < 0x110000 && k != 127)
		ui_steal(ID_P_NAME);
	inlist = ui_focus == ID_P_NAME || ui_focus == ID_P_LIST;
	/* the keys of the list (from the search too), and Enter */
	if(k == (K_CTRL | 'H') || k == (K_ALT | 'H')) {
		st.phidden = !st.phidden;
		hidden_changed();
		ui_key = 0;
	} else if(inlist && (k == K_UP || k == (K_CTRL | 'P') || k == K_DOWN ||
	    k == (K_CTRL | 'N') || k == K_PGUP || k == K_PGDN)) {
		fsel += k == K_UP || k == (K_CTRL | 'P') ? -1 : k == K_DOWN || k == (K_CTRL | 'N') ?
		    1 : k == K_PGUP ? -(lh - 1) : lh - 1;
		ui_key = 0;
	} else if(inlist && (k == 13 || (folders && k == (K_ALT | 13)))) {
		/* Enter: into it, or open it (./: the project); Alt+Enter: the
		   project */
		act = k == (K_ALT | 13) || (fsel < nfrow && frow[fsel] == HERE) ? 2 : 1;
		row = fsel;
		ui_key = 0;
	} else if(inlist && (k == (K_ALT | K_UP) || (k == 8 && st.pname.len == 0))) {
		folder_into("..");
		ui_key = 0;
	}
	scr_text(x + 2, y + 1, "Folder:", S_FLOAT, x + 2 + lw);
	ui_trunc(x + 2 + lw, y + 1, st.pdir, S_FLOAT, x + w - 2);
	scr_text(x + 2, y + 2, "Search:", S_FLOAT, x + 2 + lw);
	if(ui_edit(ID_P_NAME, x + 2 + lw, y + 2, w - 4 - lw, &st.pname, S_FLOAT) && loaded) {
		if(st.pname.len > 0 && st.pname.s[st.pname.len - 1] == path_sep)
			folder_typed_sep();
		else
			folder_rows();
	}
	if(!loaded) {		/* another folder: its entries */
		refresh();
		folder_rows();
	}
	/* the list: the one chosen in sight */
	if(fsel >= nfrow)
		fsel = nfrow - 1;
	if(fsel < 0)
		fsel = 0;
	if(fsel < ftop)
		ftop = fsel;
	if(fsel >= ftop + lh)
		ftop = fsel - lh + 1;
	lf = ui_focusable(ID_P_LIST, W_LIST, x + 1, y + 3, x + w - 2);
	if(ui_key == K_MOUSE && ui_mouse_ok() && term_mouse.x > x && term_mouse.x < x + w - 1 &&
	   term_mouse.y >= y + 3 && term_mouse.y < y + 3 + lh) {
		i = ftop + term_mouse.y - (y + 3);
		if(term_mouse.kind == M_WHEELUP || term_mouse.kind == M_WHEELDOWN)
			fsel += term_mouse.kind == M_WHEELUP ? -3 : 3;
		else if(term_mouse.kind == M_PRESS && term_mouse.button == MB_LEFT && i < nfrow) {
			fsel = i;
			ui_steal(ID_P_LIST);
			if(term_mouse.clicks == 2) {
				act = 1;	/* into it */
				row = i;
			}
		}
		ui_key = 0;
		ui_again = 1;
	}
	for(i = 0; i < lh; i++) {
		r = ftop + i;
		cur = r == fsel;
		/* the one chosen: marked, brighter when the list or the search
		   has the focus */
		at = cur && (lf || ui_focus == ID_P_NAME) ? S_FOCUS : cur ? S_SELECTION : S_LIST;
		scr_fill(x + 1, y + 3 + i, x + w - 2, y + 3 + i, ' ', at);
		if(r >= nfrow)
			continue;
		scr_put(x + 1, y + 3 + i, cur ? '>' : ' ', at);
		scr_text(x + 3, y + 3 + i, row_name(r), at, x + w - 2);
		if(frow[r] == HERE)
			scr_text(x + 6, y + 3 + i, "(open here)", at | F_DIM, x + w - 2);
	}
	if(nfrow == 0)
		scr_text(x + 3, y + 3, folders ? "No folder matches" :
		    "Nothing matches: Enter opens a new file of that name", S_LIST | F_DIM, x + w - 2);
	scr_text(x + 2, y + h - 3, folders ? hint : fhint, S_FLOAT | F_DIM, x + w - 2);
	if(ui_check(ID_P_HIDDEN, x + 2, y + h - 2, "Hidden (Alt+H)", &st.phidden, S_FLOAT))
		hidden_changed();
	scr_lower(x + 2, x + 6 + text_width("Hidden (Alt+H)"), y + h - 2);
	if(folders && ui_button(ID_P_FOLDER, x + w - 4 - text_width("Open This Folder"), y + h - 2,
	    "Open This Folder", 0, S_FLOAT)) {
		act = 2;	/* the folder shown */
		row = -2;
	}
	if(ui_modal_end()) {
		shown = 0;
		close_picker();
		return;
	}
	if(act)
		shown = act == 2 ? 0 : shown;	/* done: the next one starts again */
	if(act && (row == -2 || (nfrow == 0 && st.pname.len > 0))) {
		/* the folder shown (the button); or, nothing in the list, a path
		   typed: into it (Enter), or the project (Alt+Enter) */
		char path[PATH_MAX_ + 2];

		if(row == -2)
			strcpy(path, st.pdir);
		else {
			path_join(path, st.pdir, st.pname.s);
			path_normalize(path);
		}
		if(file_kind(path) == 2 && act != 2)
			folder_go(path);
		else if(!folders)	/* a file: opened, or a new one */
			picker_finish(path);
		else if(file_kind(path) != 2)
			error("No such folder: ", path);
		else
			picker_finish(path);
	} else if(act)
		folder_take(row, act == 2);
	ui_again = 1;
}

void
draw_picker(void)
{
	char path[PATH_MAX_ + 2];
	int w = scr_cols >= 60 ? scr_cols - 20 : scr_cols - 2, h = scr_rows - 10;
	int x, y, act = 0, lw = 11, r, done = 0;
	int tw;

	if(w < 10)
		w = scr_cols;
	if(h < 8)
		h = scr_rows - 2 < 8 ? scr_rows - 2 : 8;
	if(lw > w / 3)
		lw = w / 3;
	tw = w - 4 - lw;
	if(st.picker == PICK_FOLDER || st.picker == PICK_OPEN) {	/* a search, a list */
		draw_browse(w, h);
		return;
	}
	if(!loaded)
		refresh();
	ui_nograb = st.overwrite != 0;
	ui_modal_begin(st.picker == PICK_OPEN ? "Open File" DOTS : "Save As" DOTS,
	    w, h, S_FLOAT, &x, &y);
	/* the hidden files (.name): Ctrl+H (Alt+H; on the list of a terminal,
	   where Ctrl+H is Backspace's, that too), or the box */
	if(ui_key == (K_CTRL | 'H') || ui_key == (K_ALT | 'H') ||
	   (ui_key == (K_CTRL | 8) && ui_focus == ID_P_LIST)) {
		ui_key = 0;
		st.phidden = !st.phidden;
		hidden_changed();
	}
	scr_text(x + 2, y + 1, "Folder:", S_FLOAT, x + 2 + lw);
	ui_trunc(x + 2 + lw, y + 1, st.pdir, S_FLOAT, x + w - 2);
	scr_text(x + 2, y + 2, "File name:", S_FLOAT, x + 2 + lw);
	if(nsugg && ui_focus == ID_P_NAME && ui_key == K_DOWN) {
		ui_key = 0;
		ui_steal(ID_P_SUGGEST);
	}
	if(ui_edit(ID_P_NAME, x + 2 + lw, y + 2, tw, &st.pname, S_FLOAT))
		suggest();
	if(ui_focus == ID_P_NAME && ui_key == 13) {
		ui_key = 0;
		act = 1;
	}
	r = ui_list(ID_P_LIST, x + 1, y + 3, w - 2, h - 5, nents, ent, &sel, &top);
	/* under the list, one row: the hidden files, shown or not (in line
	   with Folder: and File name:); in a window, a third of a cell lower, in
	   the middle between the list and the bottom line of the frame */
	if(ui_check(ID_P_HIDDEN, x + 2, y + h - 2, "Hidden Files (Ctrl+H)", &st.phidden, S_FLOAT))
		hidden_changed();
	scr_lower(x + 2, x + 6 + text_width("Hidden Files (Ctrl+H)"), y + h - 2);
	if(r == 2)
		act = 1;
	if(r && nents) {
		edl_set(&st.pname, ent(sel));	/* the one selected */
		nsugg = 0;
	}
	if(ui_focus == ID_P_LIST && (ui_key == 8 || ui_key == (K_ALT | K_UP))) {
		ui_key = 0;
		edl_set(&st.pname, "..");
		act = 1;
	}
	if(nsugg && (ui_focus == ID_P_NAME || ui_focus == ID_P_SUGGEST)) {
		scr_frame(x + 1 + lw, y + 3, x + 2 + lw + tw, y + 4 + nsugg, S_LIST);
		r = ui_list(ID_P_SUGGEST, x + 2 + lw, y + 4, tw, nsugg, nsugg, sugg_item, &ssel, &stop_);
		if(r)
			edl_set(&st.pname, ent(sugg[ssel]));
		if(r == 2 || (ui_focus == ID_P_SUGGEST && (ui_key == 27 || (ui_key == K_UP && ssel == 0)))) {
			if(r != 2)
				ui_key = 0;
			nsugg = 0;
			ui_steal(ID_P_NAME);
		}
	} else if(ui_focus != ID_P_NAME)
		nsugg = 0;
	if(ui_modal_end())
		done = 1;
	if(act && !done && update_path(path)) {
		if(st.picker == PICK_SAVEAS && file_kind(path) == 1) {
			mem_free(st.overwrite);
			st.overwrite = mem_strdup(path);	/* dialogs.c asks */
		} else
			picker_finish(path);
		return;
	}
	if(done)
		close_picker();
}
