/* File > Explorer (Ctrl+E): a file manager in the editor.  As Open File,
 * the folder shown (Folder:) and a search (Search:); under them the
 * folder as a tree, and at its right a preview of the file under the mouse
 * (or of the one chosen), as Alt+P has it; of a folder, what is in it.
 *
 *   Up, Down, PgUp, PgDn  choose        Enter   a folder: opened or closed
 *   Right   a folder opened (again: into its first)  a file: opened in
 *   Left    closed (again: to the folder it is in)   the editor
 *   Backspace, Alt+Up  the folder above (nothing typed)
 *   Alt+N   New Folder       F2  Rename        Del  Move to Trash
 *   Ctrl+X  Cut   Ctrl+C  Copy   Ctrl+V  Paste (into the folder chosen, or
 *           the one the file chosen is in)
 *   Alt+Enter  Properties    F5  Read again    Alt+H  Hidden
 *   Shift+F10  the menu of the one chosen (the right button: of the one
 *           under the mouse; on no row, of the folder shown)
 *
 * The menu: Open, New Folder, Cut, Copy, Paste, Copy To..., Move To...,
 * Rename..., Move to Trash, Properties.  What asks for a name (a new
 * folder, a new name, where to copy or move to) asks it in the field of
 * the search: Enter does it, Esc does not.  The search narrows the tree to
 * the names that match (and the folders they are in), of the folders
 * open. */
#include <stdlib.h>
#include <string.h>
#include "finder.h"
#include "../core/mem.h"

#define PREVIEW_MIN	72	/* narrower: no preview */
#define WHEEL		3

/* a folder or a file of the tree */
struct node {
	struct node *up, **kid;	/* kid: of a folder read (loaded) */
	int nkid, depth;	/* depth: 0 in the folder shown */
	char dir, open, loaded, kept;
	char name[1];		/* the root: its whole path */
};

static struct node *root;
static char last_root[PATH_MAX_];	/* where it opens again */
static struct node **rows;	/* those shown; 0 is .. */
static unsigned char *rmatch;	/* the search: the name matches (else, a
				   folder it is in) */
static int nrows, rowcap, sel, top, hov = -1;
static char stale;		/* the rows made again */
static char keep[PATH_MAX_];	/* to be chosen once they are */
static struct edl query;

enum { ASK_NONE, ASK_NEWDIR, ASK_RENAME, ASK_COPYTO, ASK_MOVETO };
static int ask;			/* the field asks for a name */
static struct edl answer;
static char ask_path[PATH_MAX_];	/* of what: the folder of a new one, or
					   what is renamed, copied, moved */

static char *xclip;		/* Cut or Copy: the path; Paste takes it */
static char xcut;

static char props;		/* Properties is shown */
#define PROPS_MAX	9
static char plabel[PROPS_MAX][16], pvalue[PROPS_MAX][PATH_MAX_ + 32];
static int nprops;

static char cm;			/* the menu is open */
static int cmx, cmy, cmsel, cm_row;	/* cm_row: of what (-1: the folder shown) */
static char cm_drag;		/* the button moved, held, since it opened */

static long pv_scroll;		/* the preview, moved by the wheel */
static char pv_shown[PATH_MAX_];	/* of what: another starts again */

/* --- the tree ------------------------------------------------------------ */

static int
lower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

/* the folders first, then by name (any case, then as bytes) */
static int
order(int adir, const char *a, int bdir, const char *b)
{
	const char *p = a, *q = b;

	if(adir != bdir)
		return adir ? -1 : 1;
	for(; *p && lower((unsigned char)*p) == lower((unsigned char)*q); p++, q++)
		;
	if(lower((unsigned char)*p) != lower((unsigned char)*q))
		return lower((unsigned char)*p) - lower((unsigned char)*q);
	return strcmp(a, b);
}

static int
by_order(const void *a, const void *b)
{
	const struct node *x = *(struct node *const *)a, *y = *(struct node *const *)b;

	return order(x->dir, x->name, y->dir, y->name);
}

/* the kid of n of that name and kind, -1 if none (they are in order) */
static int
find_kid(const struct node *n, const char *name, int dir)
{
	int lo = 0, hi = n->nkid - 1, m, c;

	while(lo <= hi) {
		m = (lo + hi) / 2;
		c = order(dir, name, n->kid[m]->dir, n->kid[m]->name);
		if(c == 0)
			return m;
		if(c < 0)
			hi = m - 1;
		else
			lo = m + 1;
	}
	return -1;
}

static struct node *
node_new(struct node *up, const char *name, int dir)
{
	size_t n = strlen(name);
	struct node *p = malloc(sizeof *p + n);

	if(p == 0)
		return 0;
	memset(p, 0, sizeof *p);
	p->up = up;
	p->dir = (char)dir;
	p->depth = up ? up->depth + 1 : -1;
	memcpy(p->name, name, n + 1);
	return p;
}

static void
node_free(struct node *n)
{
	int i;

	if(n == 0)
		return;
	for(i = 0; i < n->nkid; i++)
		node_free(n->kid[i]);
	free(n->kid);
	free(n);
}

/* the whole path of n: no separator at its end (but the root of all) */
static void
node_path(const struct node *n, char *out)
{
	char up[PATH_MAX_ + 2];

	if(n->up == 0) {
		strcpy(out, n->name);
		return;
	}
	node_path(n->up, up);
	path_join(out, up, n->name);
}

/* the entries of folder n read (again): the ones it had kept, with what
 * was open in them */
static void
load(struct node *n)
{
	char path[PATH_MAX_ + 2], name[PATH_MAX_ + 2];
	struct node **kid = 0, **k, *p;
	int nk = 0, cap = 0, isdir, i;

	n->loaded = 1;
	node_path(n, path);
	if(dir_open(path)) {
		while(dir_next(name, PATH_MAX_, &isdir)) {
			if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
				continue;
			if(name[0] == '.' && !st.phidden)
				continue;
			if((i = find_kid(n, name, isdir)) >= 0 && !n->kid[i]->kept) {
				p = n->kid[i];
				p->kept = 1;
			} else if((p = node_new(n, name, isdir)) == 0)
				break;
			if(nk == cap) {
				cap = cap ? cap * 2 : 64;
				if((k = realloc(kid, (size_t)cap * sizeof *kid)) == 0) {
					if(!p->kept)
						free(p);
					break;
				}
				kid = k;
			}
			kid[nk++] = p;
		}
		dir_close();
	}
	for(i = 0; i < n->nkid; i++)	/* those that are gone */
		if(!n->kid[i]->kept)
			node_free(n->kid[i]);
	free(n->kid);
	for(i = 0; i < nk; i++)
		kid[i]->kept = 0;
	if(nk > 1)
		qsort(kid, (size_t)nk, sizeof *kid, by_order);
	n->kid = kid;
	n->nkid = nk;
}

/* the folders open read again; those closed, when they are opened */
static void
reload(struct node *n)
{
	int i;

	if(!n->dir || !n->loaded)
		return;
	if(n->up && !n->open) {
		n->loaded = 0;
		return;
	}
	load(n);
	for(i = 0; i < n->nkid; i++)
		reload(n->kid[i]);
}

static void
add_row(struct node *n, int match)
{
	struct node **r;
	unsigned char *m;

	if(nrows == rowcap) {
		rowcap = rowcap ? rowcap * 2 : 256;
		r = realloc(rows, (size_t)rowcap * sizeof *rows);
		m = realloc(rmatch, (size_t)rowcap);
		if(r)
			rows = r;
		if(m)
			rmatch = m;
		if(r == 0 || m == 0) {
			rowcap = nrows;
			return;
		}
	}
	rmatch[nrows] = (unsigned char)match;
	rows[nrows++] = n;
}

/* the rows of what is in n (the search: what matches, and the folders it
 * is in): 1 if one */
static int
build(struct node *n, int searching)
{
	int i, at, m, any = 0;
	struct node *k;

	for(i = 0; i < n->nkid; i++) {
		k = n->kid[i];
		at = nrows;
		m = !searching || fz_score(k->name, (int)strlen(k->name)) >= 0;
		add_row(k, m);
		if(k->dir && k->open) {
			if(!k->loaded)
				load(k);
			if(build(k, searching))
				any = 1;	/* its rows stay, and it with them */
			else if(!m)
				nrows = at;
		} else if(!m)
			nrows = at;
		any |= m;
	}
	return any;
}

/* the row of path, -1 if none */
static int
row_of(const char *path)
{
	char p[PATH_MAX_ + 2];
	int r;

	for(r = 0; r < nrows; r++)
		if(rows[r]) {
			node_path(rows[r], p);
			if(strcmp(p, path) == 0)
				return r;
		}
	return -1;
}

static void
rebuild(void)
{
	int searching, r;

	query.s[query.len] = 0;
	searching = query.len > 0;
	nrows = 0;
	if(searching)
		fz_set(query.s);
	else if(root->name[rootlen(root->name)] != 0)	/* not at the root: .. */
		add_row(0, 1);
	if(!root->loaded)
		load(root);
	build(root, searching);
	stale = 0;
	if(keep[0]) {
		if((r = row_of(keep)) >= 0)
			sel = r;
		keep[0] = 0;
	} else if(searching)	/* the first that matches */
		for(sel = 0; sel < nrows && !rmatch[sel]; sel++)
			;
	if(sel >= nrows)
		sel = nrows - 1;
	if(sel < 0)
		sel = 0;
	hov = -1;
}

/* path: within dir, or dir itself */
static int
within(const char *path, const char *dir)
{
	size_t n = strlen(dir);

	while(n > rootlen(dir) && issep(dir[n - 1]))
		n--;
	if(strncmp(path, dir, n) != 0)
		return 0;
	return path[n] == 0 || issep(path[n]) || n == rootlen(dir);
}

/* the folders down to path opened (within the folder shown): it is seen */
static void
reveal(const char *path)
{
	char name[PATH_MAX_];
	const char *s, *e;
	struct node *n = root;
	int i;

	if(!within(path, root->name))
		return;
	s = path + strlen(root->name);
	for(;;) {
		while(issep(*s))
			s++;
		if(*s == 0)
			break;
		for(e = s; *e && !issep(*e); e++)
			;
		memcpy(name, s, (size_t)(e - s));
		name[e - s] = 0;
		if(!n->loaded)
			load(n);
		if((i = find_kid(n, name, 1)) < 0 && (i = find_kid(n, name, 0)) < 0)
			break;
		n = n->kid[i];
		if(*e && n->dir)
			n->open = 1;
		s = e;
	}
}

/* the folder shown is now dir */
static void
set_root(const char *dir)
{
	char path[PATH_MAX_ + 2];
	struct node *n;

	strcpy(path, dir);
	path_normalize(path);
	if(path[0] == 0)
		path[0] = path_sep, path[1] = 0;
	if(file_kind(path) != 2) {
		error("No such folder: ", path);
		return;
	}
	if((n = node_new(0, path, 1)) == 0) {
		error("Out of memory", 0);
		return;
	}
	node_free(root);
	nrows = 0;	/* until made again */
	hov = -1;
	root = n;
	root->open = 1;
	strcpy(last_root, path);
	edl_set(&query, "");
	sel = top = 0;
	pv_shown[0] = 0;
	stale = 1;
}

/* the folder above the one shown: it is the one chosen there */
static void
go_up(void)
{
	char was[PATH_MAX_ + 2], up[PATH_MAX_ + 2];
	size_t n;

	strcpy(was, root->name);
	if(was[rootlen(was)] == 0)
		return;		/* the root of all */
	strcpy(up, was);
	n = (size_t)(path_last(up) - up);
	up[n] = 0;
	set_root(up);
	strcpy(keep, was);
}

/* the folder shown or what is in it changed: read again, path (if any)
 * chosen and seen */
void
explorer_changed(void)
{
	if(root) {
		if(!keep[0] && sel < nrows && rows[sel])
			node_path(rows[sel], keep);
		nrows = 0;	/* their nodes may go: none until made again */
		hov = -1;
		reload(root);
		stale = 1;
	}
	pv_shown[0] = 0;
	preview_forget();
	ui_again = 1;
}

static void
changed_to(const char *path)
{
	explorer_changed();
	if(path) {
		reveal(path);
		strcpy(keep, path);
	}
}

void
explorer_open(void)
{
	char cwd[PATH_MAX_];

	st.explorer = 1;
	edl_set(&query, "");
	ask = ASK_NONE;
	cm = props = 0;
	if(root == 0) {
		if(last_root[0])
			set_root(last_root);
		if(root == 0 && st.pdir[0])
			set_root(st.pdir);
		if(root == 0) {
			plat_cwd(cwd, sizeof cwd);
			set_root(cwd);
		}
	} else
		explorer_changed();
	ui_steal(ID_P_NAME);
	ui_again = 1;
}

void
explorer_close(void)
{
	node_free(root);
	root = 0;
	free(rows);
	free(rmatch);
	rows = 0;
	rmatch = 0;
	nrows = rowcap = 0;
	preview_forget();
	pv_shown[0] = 0;
	ask = ASK_NONE;
	cm = props = 0;
	st.explorer = 0;
	rows_stale = 1;
	ui_again = 1;
}

/* --- what is done to them ------------------------------------------------- */

/* the path of row r (-1: the folder shown): 0 for .., or none */
static int
row_path(int r, char *out)
{
	if(r < 0) {
		strcpy(out, root->name);
		return 2;
	}
	if(r >= nrows || rows[r] == 0)
		return 0;
	node_path(rows[r], out);
	return 1;
}

/* the folder of row r: itself, or the one it is in */
static void
row_folder(int r, char *out)
{
	if(r >= 0 && r < nrows && rows[r] && rows[r]->dir)
		node_path(rows[r], out);
	else if(r >= 0 && r < nrows && rows[r] && rows[r]->up)
		node_path(rows[r]->up, out);
	else
		strcpy(out, root->name);
}

static void
parent_of(const char *path, char *out)
{
	size_t n;

	strcpy(out, path);
	n = (size_t)(path_last(out) - out);
	while(n > rootlen(out) && issep(out[n - 1]))
		n--;
	out[n] = 0;
}

/* the documents of what was moved: their new names */
static void
docs_moved(const char *from, const char *to)
{
	size_t n = strlen(from), k = strlen(to), r;
	struct doc *d;
	char *p;

	for(d = docs; d; d = d->next)
		if(d->path && within(d->path, from)) {
			r = strlen(d->path + n);
			if((p = mem_alloc(k + r + 1)) == 0)
				continue;
			memcpy(p, to, k);
			memcpy(p + k, d->path + n, r + 1);
			mem_free(d->path);
			d->path = p;
			strncpy(d->name, path_last(p), sizeof d->name - 1);
			d->name[sizeof d->name - 1] = 0;
			d->lang = (unsigned char)lang_detect(p);
			d->b.stamp = file_stamp(p);
			bar_stale = rows_stale = 1;
		}
}

/* what is not to be moved: said, 1 */
static int
move_refused(const char *path)
{
	char cwd[PATH_MAX_];
	const char *home = getenv("HOME");

	plat_cwd(cwd, sizeof cwd);
	if(within(cwd, path)) {
		error("The project's folder is in it: ", path);
		return 1;
	}
	if(home && within(home, path)) {
		error("The home folder is in it: ", path);
		return 1;
	}
	return 0;
}

static int
bad_name(const char *name)
{
	const char *p;

	for(p = name; *p; p++)
		if(issep(*p))
			break;
	if(*name == 0 || *p || strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		error("Not a name for a file or a folder: ", name);
		return 1;
	}
	return 0;
}

static void
why(const char *what, const char *reason, const char *path)
{
	char msg[200];

	strcpy(msg, what);
	strcat(msg, " (");
	strncat(msg, reason, 120);
	strcat(msg, "): ");
	error(msg, path);
}

/* a name free next to path: "name (copy).ext", "name (copy 2).ext", ... */
static int
free_copy_name(const char *was, char *out)
{
	char path[PATH_MAX_ + 2], num[16];
	const char *name, *dot;
	size_t stem;
	int i;

	strcpy(path, was);	/* out may be was */
	name = path_last(path);
	dot = strrchr(name, '.');
	if(dot == 0 || dot == name || file_kind(path) == 2)
		dot = name + strlen(name);
	stem = (size_t)(dot - path);
	for(i = 1; i < 1000; i++) {
		if(stem + strlen(dot) + 20 >= PATH_MAX_)
			return 0;
		memcpy(out, path, stem);
		strcpy(out + stem, " (copy");
		if(i > 1) {
			num[0] = ' ';
			udec(num + 1, (pos_t)i);
			strcat(out, num);
		}
		strcat(out, ")");
		strcat(out, dot);
		if(file_kind(out) == 0)
			return 1;
	}
	return 0;
}

/* from copied or moved to to (a name, not a folder to put it in); a copy
 * onto itself is given a free name */
static int
transfer(const char *from, const char *to, int move)
{
	char dst[PATH_MAX_ + 2];
	const char *r;

	strcpy(dst, to);
	if(strcmp(from, dst) == 0) {
		if(move)
			return 1;	/* there already */
		if(!free_copy_name(from, dst)) {
			error("No name is free for a copy of ", from);
			return 0;
		}
	}
	if(within(dst, from)) {
		error(move ? "A folder cannot be moved into itself: " :
		    "A folder cannot be copied into itself: ", from);
		return 0;
	}
	if(file_kind(dst) != 0) {
		error("There is one of that name already: ", dst);
		return 0;
	}
	if(move && move_refused(from))
		return 0;
	if((r = move ? plat_move(from, dst) : plat_copy(from, dst)) != 0) {
		why(move ? "Cannot move" : "Cannot copy", r, from);
		changed_to(0);
		return 0;
	}
	if(move)
		docs_moved(from, dst);
	changed_to(dst);
	return 1;
}

static void
do_paste(int r)
{
	char dir[PATH_MAX_ + 2], to[PATH_MAX_ + 2];

	if(xclip == 0) {
		error("Cut or copy a file or a folder first (Ctrl+X, Ctrl+C)", 0);
		return;
	}
	if(file_kind(xclip) == 0) {
		error("It is no longer there: ", xclip);
		mem_free(xclip);
		xclip = 0;
		return;
	}
	row_folder(r, dir);
	path_join(to, dir, path_last(xclip));
	path_normalize(to);
	if(xcut && strcmp(to, xclip) == 0)
		return;		/* where it is */
	if(!xcut && file_kind(to) != 0 && !free_copy_name(to, to)) {
		error("No name is free for a copy of ", xclip);
		return;
	}
	if(transfer(xclip, to, xcut) && xcut) {
		mem_free(xclip);	/* moved: no more to paste */
		xclip = 0;
	}
}

static void
do_clip(int r, int cut)
{
	char path[PATH_MAX_ + 2];

	if(row_path(r, path) != 1) {
		error("Choose a file or a folder in the tree", 0);
		return;
	}
	mem_free(xclip);
	xclip = mem_strdup(path);
	xcut = (char)cut;
}

static void
do_trash(int r)
{
	char path[PATH_MAX_ + 2];

	if(row_path(r, path) != 1) {
		error("Choose a folder or a file in the tree to move to the Trash", 0);
		return;
	}
	if(trash_refused(path))
		return;
	mem_free(st.trash);
	st.trash = mem_strdup(path);	/* dialogs.c asks, Yes: explorer_changed */
}

/* the field asks for a name: what for (k), of row r */
static void
ask_begin(int k, int r)
{
	char path[PATH_MAX_ + 2], dir[PATH_MAX_ + 2];
	size_t n;

	if(k == ASK_NEWDIR) {
		row_folder(r, ask_path);
		edl_set(&answer, "");
	} else {
		if(row_path(r, path) != 1) {
			error("Choose a file or a folder in the tree", 0);
			return;
		}
		strcpy(ask_path, path);
		if(k == ASK_RENAME)
			edl_set(&answer, path_last(path));
		else {	/* the folder it is in, to be changed */
			parent_of(path, dir);
			n = strlen(dir);
			if(n && !issep(dir[n - 1]))
				dir[n++] = path_sep, dir[n] = 0;
			edl_set(&answer, dir);
		}
	}
	ask = k;
	cm = 0;
	ui_steal(ID_P_NAME);
}

/* Enter in the field: done (0: the field asks again) */
static int
ask_done(void)
{
	char path[PATH_MAX_ + 2], dir[PATH_MAX_ + 2];
	const char *name, *r;

	answer.s[answer.len] = 0;
	name = answer.s;
	switch(ask) {
	case ASK_NEWDIR:
		if(*name == 0)
			return 1;
		if(bad_name(name))
			return 0;
		path_join(path, ask_path, name);
		path_normalize(path);
		if((r = dir_make(path)) != 0) {
			why("Cannot make the folder", r, path);
			return 0;
		}
		changed_to(path);
		return 1;
	case ASK_RENAME:
		if(*name == 0 || strcmp(name, path_last(ask_path)) == 0)
			return 1;
		if(bad_name(name))
			return 0;
		parent_of(ask_path, dir);
		path_join(path, dir, name);
		path_normalize(path);
		return transfer(ask_path, path, 1);
	case ASK_COPYTO:
	case ASK_MOVETO:
		if(*name == 0)
			return 1;
		/* ~/ the home; relative: to the folder shown */
		if(name[0] == '~' && (name[1] == 0 || issep(name[1])) && getenv("HOME"))
			path_join(path, getenv("HOME"), name[1] ? name + 2 : "");
		else
			path_join(path, root->name, name);
		path_normalize(path);
		if(file_kind(path) == 2) {	/* a folder: into it */
			strcpy(dir, path);
			path_join(path, dir, path_last(ask_path));
			path_normalize(path);
		} else {
			parent_of(path, dir);
			if(file_kind(dir) != 2) {
				error("No such folder: ", dir);
				return 0;
			}
		}
		return transfer(ask_path, path, ask == ASK_MOVETO);
	}
	return 1;
}

/* a count of bytes as one reads it: 1.5 MB (1,572,864 bytes) */
static void
size_text(pos_t n, char *out)
{
	static const char *const unit[] = { "KB", "MB", "GB", "TB" };
	char d[32];
	pos_t v = n, frac = 0;
	int u = -1, len, i, o = 0;

	while(v >= 1024 && u < 3) {
		frac = v % 1024 * 10 / 1024;
		v /= 1024;
		u++;
	}
	out[0] = 0;
	if(u >= 0) {
		udec(out, v);
		strcat(out, ".");
		udec(out + strlen(out), frac);
		strcat(out, " ");
		strcat(out, unit[u]);
		strcat(out, " (");
	}
	len = udec(d, n);
	o = (int)strlen(out);
	for(i = 0; i < len; i++) {	/* the thousands apart */
		if(i && (len - i) % 3 == 0)
			out[o++] = ',';
		out[o++] = d[i];
	}
	out[o] = 0;
	strcat(out, n == 1 ? " byte" : " bytes");
	if(u >= 0)
		strcat(out, ")");
}

static void
prop(const char *label, const char *value)
{
	size_t n = strlen(value);

	if(nprops >= PROPS_MAX)
		return;
	if(n >= sizeof pvalue[0])
		n = sizeof pvalue[0] - 1;
	strcpy(plabel[nprops], label);
	memcpy(pvalue[nprops], value, n);
	pvalue[nprops][n] = 0;
	nprops++;
}

static void
do_props(int r)
{
	char path[PATH_MAX_ + 2], dir[PATH_MAX_ + 2], v[PATH_MAX_ + 64];
	struct file_info fi;
	pos_t files, dirs, bytes;
	int whole;

	if(row_path(r, path) == 0)
		return;
	if(!file_info(path, &fi)) {
		error("Cannot read: ", path);
		return;
	}
	nprops = 0;
	prop("Name:", path[rootlen(path)] ? path_last(path) : path);
	strcpy(v, fi.kind == 2 ? "Folder" : fi.kind == 1 ? "File" : "Other (a device, a pipe, ...)");
	if(fi.link) {
		strcpy(v, fi.kind == 2 ? "Link to a folder" : fi.kind == 1 ? "Link to a file" :
		    "Link (to nothing)");
	}
	prop("Kind:", v);
	parent_of(path, dir);
	prop("Location:", dir);
	if(fi.link)
		prop("Points to:", fi.target);
	if(fi.kind == 2) {
		whole = dir_usage(path, &files, &dirs, &bytes, 2000);
		size_text(bytes, v);
		if(!whole)
			strcat(v, ", more: too many to count");
		prop("Size:", v);
		udec(v, files);
		strcat(v, files == 1 ? " file, " : " files, ");
		udec(v + strlen(v), dirs);
		strcat(v, dirs == 1 ? " folder" : " folders");
		prop("Contains:", v);
	} else if(fi.kind == 1) {
		size_text(fi.size, v);
		prop("Size:", v);
	}
	prop("Modified:", fi.modified);
	prop("Permissions:", fi.mode);
	prop("Owner:", fi.owner);
	props = 1;
}

/* a row taken (Enter, a double click, Open): .. up; a folder opened or
 * closed (into it: Open of the menu); a file opened in the editor */
static void
activate(int r, int into)
{
	char path[PATH_MAX_ + 2];
	struct node *n;

	if(r < 0 || r >= nrows)
		return;
	if((n = rows[r]) == 0) {
		go_up();
		return;
	}
	node_path(n, path);
	if(n->dir) {
		if(into)
			set_root(path);
		else {
			n->open = !n->open;
			stale = 1;
			strcpy(keep, path);
		}
		return;
	}
	if(doc_open(path)) {
		explorer_close();
		cursor_visible();
	}
}

/* --- the menu (the right button, Shift+F10) ------------------------------ */

enum { C_OPEN, C_NEWDIR, C_S1, C_CUT, C_COPY, C_PASTE, C_S2, C_COPYTO, C_MOVETO,
	C_RENAME, C_TRASH, C_S3, C_PROPS, C_COUNT };
static const struct {
	const char *text, *keys;
	char accel;
} cm_items[C_COUNT] = {
	{ "Open", "Enter", 'O' },
	{ "New Folder" DOTS, KEY_ALT "+N", 'N' },
	{ "-", "", 0 },
	{ "Cut", KEY_CTRL "+X", 'T' },
	{ "Copy", KEY_CTRL "+C", 'C' },
	{ "Paste", KEY_CTRL "+V", 'P' },
	{ "-", "", 0 },
	{ "Copy To" DOTS, "", 'Y' },
	{ "Move To" DOTS, "", 'M' },
	{ "Rename" DOTS, "F2", 'R' },
	{ "Move to Trash", "Del", 'H' },
	{ "-", "", 0 },
	{ "Properties", KEY_ALT "+Enter", 'I' },
};

static int
cm_usable(int i)
{
	int thing = cm_row >= 0 && cm_row < nrows && rows[cm_row];

	switch(i) {
	case C_S1: case C_S2: case C_S3:
		return 0;
	case C_NEWDIR:
		return 1;
	case C_PASTE:
		return xclip != 0;
	case C_PROPS:
		return cm_row < 0 || thing;
	}
	return thing;
}

static void
cm_open(int r, int x, int y)
{
	cm = 1;
	cm_row = r;
	cm_drag = 0;
	cmx = x;
	cmy = y;
	for(cmsel = 0; cmsel < C_COUNT && !cm_usable(cmsel); cmsel++)
		;
	ui_again = 1;
}

static void
cm_box(int *x, int *y, int *x2, int *y2, int *kx)
{
	int i, tw = 0, kw = 0, w;

	for(i = 0; i < C_COUNT; i++) {
		if(text_width(cm_items[i].text) > tw)
			tw = text_width(cm_items[i].text);
		if(text_width(cm_items[i].keys) > kw)
			kw = text_width(cm_items[i].keys);
	}
	w = 1 + 2 + tw + 3 + kw + 1 + 1;
	*x = cmx;
	*y = cmy;
	if(*x + w > scr_cols)
		*x = scr_cols - w;
	if(*y + C_COUNT + 2 > scr_rows)
		*y = cmy - C_COUNT - 1;
	if(*y + C_COUNT + 2 > scr_rows)
		*y = scr_rows - C_COUNT - 2;
	if(*x < 0)
		*x = 0;
	if(*y < 0)
		*y = 0;
	*x2 = *x + w - 1;
	*y2 = *y + C_COUNT + 1;
	*kx = *x + 1 + 2 + tw + 3;
}

static void
cm_do(int i)
{
	int r = cm_row;

	cm = 0;
	ui_again = 1;
	switch(i) {
	case C_OPEN: activate(r, 1); break;
	case C_NEWDIR: ask_begin(ASK_NEWDIR, r); break;
	case C_CUT: do_clip(r, 1); break;
	case C_COPY: do_clip(r, 0); break;
	case C_PASTE: do_paste(r); break;
	case C_COPYTO: ask_begin(ASK_COPYTO, r); break;
	case C_MOVETO: ask_begin(ASK_MOVETO, r); break;
	case C_RENAME: ask_begin(ASK_RENAME, r); break;
	case C_TRASH: do_trash(r); break;
	case C_PROPS: do_props(r); break;
	}
}

/* the keys and the mouse, while it is open: all of them its own */
static void
cm_input(void)
{
	int x, y, x2, y2, kx, i, d;
	kcode k = ui_key;
	struct mouse *ms = &term_mouse;

	if(k == 0)
		return;
	ui_key = 0;
	ui_again = 1;
	cm_box(&x, &y, &x2, &y2, &kx);
	if(k == K_MOUSE) {
		i = ms->x > x && ms->x < x2 && ms->y > y && ms->y < y2 ? ms->y - y - 1 : -1;
		if(ms->kind == M_MOVE || ms->kind == M_DRAG) {
			cm_drag |= ms->kind == M_DRAG;
			if(i >= 0 && cm_usable(i))
				cmsel = i;
		} else if(ms->kind == M_PRESS) {
			if(i >= 0 && cm_usable(i))
				cm_do(i);
			else if(i < 0 && !(ms->x >= x && ms->x <= x2 && ms->y >= y && ms->y <= y2))
				cm = 0;	/* elsewhere: it closes */
		} else if(ms->kind == M_RELEASE && cm_drag && i >= 0 && cm_usable(i))
			cm_do(i);	/* the button that opened it, dragged onto one */
		return;
	}
	if(k == 27 || k == (K_SHIFT | K_F10)) {
		cm = 0;
		return;
	}
	if(k == K_UP || k == K_DOWN) {
		d = k == K_UP ? -1 : 1;
		for(i = 0; i < C_COUNT; i++) {
			cmsel = (cmsel + d + C_COUNT) % C_COUNT;
			if(cm_usable(cmsel))
				break;
		}
		return;
	}
	if((k == 13 || k == ' ') && cm_usable(cmsel)) {
		cm_do(cmsel);
		return;
	}
	for(i = 0; i < C_COUNT; i++)
		if(cm_usable(i) && cm_items[i].accel && !(k & (K_CTRL | K_ALT)) &&
		    (K_CODE(k) & ~0x20) == cm_items[i].accel) {
			cm_do(i);
			return;
		}
}

static void
cm_draw(void)
{
	int x, y, x2, y2, kx, i, a;

	cm_box(&x, &y, &x2, &y2, &kx);
	scr_fill(x, y, x2, y2, ' ', S_FLOAT);
	scr_frame(x, y, x2, y2, S_FLOAT);
	for(i = 0; i < C_COUNT; i++) {
		if(cm_items[i].text[0] == '-') {
			scr_put(x, y + 1 + i, G_SEPL, S_FLOAT);
			scr_fill(x + 1, y + 1 + i, x2 - 1, y + 1 + i, G_H, S_FLOAT);
			scr_put(x2, y + 1 + i, G_SEPR, S_FLOAT);
			continue;
		}
		a = !cm_usable(i) ? S_FLOAT | F_DIM : i == cmsel ? S_FOCUS : S_FLOAT;
		scr_fill(x + 1, y + 1 + i, x2 - 1, y + 1 + i, ' ', a);
		ui_accel(x + 3, y + 1 + i, cm_items[i].text, cm_items[i].accel, a, kx - 1);
		scr_text(kx, y + 1 + i, cm_items[i].keys, a, x2);
	}
}

/* --- the dialog ---------------------------------------------------------- */

static void
draw_props(void)
{
	static const char *const ok[] = { "Ok" };
	static const int okid[] = { ID_OK };
	int w = 30, lw = 0, i, x, y, h = nprops + 5, r;

	for(i = 0; i < nprops; i++)
		if(text_width(plabel[i]) > lw)
			lw = text_width(plabel[i]);
	for(i = 0; i < nprops; i++)
		if(lw + 1 + text_width(pvalue[i]) + 6 > w)
			w = lw + 1 + text_width(pvalue[i]) + 6;
	w = dialog_width(w);
	ui_modal_begin("Properties", w, h, S_FLOAT, &x, &y);
	for(i = 0; i < nprops; i++) {
		scr_text(x + 3, y + 2 + i, plabel[i], S_FLOAT | F_DIM, x + w - 3);
		ui_trunc(x + 3 + lw + 1, y + 2 + i, pvalue[i], S_FLOAT, x + w - 3);
	}
	r = buttons(x + 1, y + h - 2, w - 2, ok, 0, okid, 1, S_FLOAT);
	if(ui_modal_end() || r >= 0) {
		props = 0;
		ui_steal(ID_P_LIST);
		ui_again = 1;
	}
}

/* the preview of row r, from (x1, y1) to (x2, y2): a file's text, a
 * folder's entries */
static void
draw_view(int r, int x1, int y1, int x2, int y2)
{
	char path[PATH_MAX_ + 2], count[64];
	struct node *n;
	int i, h = y2 - y1 + 1, nd = 0, at;

	if(r < 0 || r >= nrows || (n = rows[r]) == 0)
		return;
	node_path(n, path);
	if(strcmp(path, pv_shown) != 0) {	/* another: from its start */
		strcpy(pv_shown, path);
		pv_scroll = 0;
	}
	if(!n->dir) {
		preview_file(path, x1, y1, x2, y2, &pv_scroll);
		return;
	}
	if(!n->loaded)
		load(n);
	for(i = 0; i < n->nkid; i++)
		nd += n->kid[i]->dir;
	udec(count, (pos_t)nd);
	strcat(count, nd == 1 ? " folder, " : " folders, ");
	udec(count + strlen(count), (pos_t)(n->nkid - nd));
	strcat(count, n->nkid - nd == 1 ? " file" : " files");
	scr_text(x1, y1, count, S_COMMENT, x2 + 1);
	if(pv_scroll > n->nkid - (h - 2))
		pv_scroll = n->nkid - (h - 2);
	if(pv_scroll < 0)
		pv_scroll = 0;
	for(i = 0; i < h - 2 && pv_scroll + i < n->nkid; i++) {
		struct node *k = n->kid[pv_scroll + i];

		at = k->dir ? S_KEYWORD : S_TEXT;
		if(k->dir)
			scr_put(x1, y1 + 2 + i, G_SCROLLRIGHT, S_COMMENT);
		scr_put(ui_trunc(x1 + 2, y1 + 2 + i, k->name, at, x2), y1 + 2 + i,
		    k->dir ? path_sep : ' ', at);
	}
}

static void
draw_row(int r, int x1, int y, int x2, int at)
{
	char path[PATH_MAX_ + 2];
	struct node *n = rows[r];
	int x;

	if(n == 0) {
		scr_text(x1 + 2, y, "..", at, x2 + 1);
		return;
	}
	if(!rmatch[r])	/* the search: only a folder of what matches */
		at |= F_DIM;
	if(xclip && xcut) {	/* cut: greyed until pasted */
		node_path(n, path);
		if(strcmp(path, xclip) == 0)
			at |= F_DIM;
	}
	x = x1 + n->depth * 2;
	if(x > x2)
		return;
	if(n->dir)
		scr_put(x, y, n->open ? G_SCROLLDOWN : G_SCROLLRIGHT, at);
	x = ui_trunc(x + 2, y, n->name, at, x2);
	if(n->dir && x <= x2)
		scr_put(x, y, path_sep, at);
}

void
draw_explorer(void)
{
	static const char *const hint =
	    "Enter: open   Right/Left: open/close a folder   F2: rename   Del: Trash   "
	    "right button: menu";
	static const char *const ask_label[] = { "Search:", "New folder:", "New name:",
		"Copy to:", "Move to:" };
	static const char *const ask_hint[] = { "", "Enter: make the folder   Esc: cancel",
		"Enter: rename   Esc: cancel", "Enter: copy (a folder: into it)   Esc: cancel",
		"Enter: move (a folder: into it)   Esc: cancel" };
	int mx = scr_cols >= 60 ? scr_cols / 20 : 0, my = scr_rows >= 20 ? scr_rows / 20 : 0;
	int w = scr_cols - 2 * mx, h = scr_rows - 2 * my;
	int x, y, lw = 12, lh, tx2, px1 = 0, i, r, at, lf, inlist, nav, act = -1, hw, cx, bx;
	int over, view, empty;
	struct mouse *ms = &term_mouse;
	struct edl *field = ask ? &answer : &query;
	kcode k;

	if(root == 0) {
		explorer_close();
		return;
	}
	if(h < 10)
		h = scr_rows < 10 ? scr_rows : 10;
	if(stale)
		rebuild();
	over = st.trash || st.nerr || props;	/* asked over it, or an error */
	ui_nograb = over;
	ui_modal_begin("Explorer", w, h, S_FLOAT, &x, &y);
	lh = h - 6;
	tx2 = x + w - 2;	/* the tree from x + 1; the preview after it */
	if(w >= PREVIEW_MIN) {
		tx2 = x + w * 2 / 5;
		px1 = tx2 + 2;
	}
	k = over ? 0 : ui_key;
	if(cm && !over) {
		cm_input();
		k = 0;
		if(!st.explorer) {	/* a file opened */
			ui_modal_end();
			return;
		}
	}
	/* the list has the focus and a character is typed: to the field */
	if(ui_focus == ID_P_LIST && k >= ' ' && k < 0x110000 && k != 127)
		ui_steal(ID_P_NAME);
	lf = ui_focus == ID_P_LIST;
	inlist = lf || ui_focus == ID_P_NAME;
	empty = lf || (ui_focus == ID_P_NAME && !ask && query.len == 0);
	nav = inlist && !ask;
	if(ask && (k == 27 || (k == 13 && ui_focus == ID_P_NAME))) {
		if(k == 27 || ask_done()) {
			ask = ASK_NONE;
			ui_steal(ID_P_LIST);
		}
		ui_key = 0;
		k = 0;
	}
	if(k == 0)
		;
	else if(k == (K_CTRL | 'E')) {	/* the shortcut again: closed */
		explorer_close();
		ui_modal_end();
		return;
	} else if(k == (K_ALT | 'N') || k == (K_ALT | 'n'))
		ask_begin(ASK_NEWDIR, sel);
	else if(k == (K_CTRL | 'H') || k == (K_ALT | 'H')) {
		st.phidden = !st.phidden;
		settings_find_changed();
		explorer_changed();
	} else if(k == K_F5)
		explorer_changed();
	else if(!ask && k == (K_ALT | 13))
		do_props(sel);
	else if(!ask && k == (K_SHIFT | K_F10) && nrows) {
		cm_open(sel, x + 4 + (rows[sel] ? rows[sel]->depth * 2 : 0),
		    y + 3 + sel - top + 1);
	} else if(!ask && k == K_F2 && inlist)
		ask_begin(ASK_RENAME, sel);
	else if(empty && !ask && k == K_DEL)
		do_trash(sel);
	else if(empty && !ask && k == (K_CTRL | 'C'))
		do_clip(sel, 0);
	else if(empty && !ask && k == (K_CTRL | 'X'))
		do_clip(sel, 1);
	else if(empty && !ask && k == (K_CTRL | 'V'))
		do_paste(sel);
	else if(nav && (k == K_UP || k == (K_CTRL | 'P') || k == K_DOWN || k == (K_CTRL | 'N') ||
	    k == K_PGUP || k == K_PGDN)) {
		sel += k == K_UP || k == (K_CTRL | 'P') ? -1 : k == K_DOWN || k == (K_CTRL | 'N') ?
		    1 : k == K_PGUP ? -(lh - 1) : lh - 1;
		hov = -1;
	} else if(nav && empty && (k == K_HOME || k == K_END))
		sel = k == K_HOME ? 0 : nrows - 1;
	else if(nav && empty && k == K_RIGHT && sel < nrows && rows[sel] && rows[sel]->dir) {
		if(!rows[sel]->open)
			activate(sel, 0);
		else if(sel + 1 < nrows && rows[sel + 1] && rows[sel + 1]->up == rows[sel])
			sel++;
	} else if(nav && empty && k == K_LEFT && sel < nrows && rows[sel]) {
		if(rows[sel]->dir && rows[sel]->open)
			activate(sel, 0);
		else if(rows[sel]->up != root)	/* to the folder it is in */
			for(r = sel - 1; r >= 0; r--)
				if(rows[r] == rows[sel]->up) {
					sel = r;
					break;
				}
	} else if(nav && k == 13)
		act = sel;
	else if(nav && (k == (K_ALT | K_UP) || (k == 8 && empty)))
		go_up();
	else
		k = 0;
	if(k)
		ui_key = 0;
	if(stale)
		rebuild();

	scr_text(x + 2, y + 1, "Folder:", S_FLOAT, x + 2 + lw);
	ui_trunc(x + 2 + lw, y + 1, root->name, S_FLOAT, x + w - 2);
	scr_text(x + 2, y + 2, ask_label[ask], S_FLOAT, x + 2 + lw);
	field = ask ? &answer : &query;
	if(ui_edit(ID_P_NAME, x + 2 + lw, y + 2, w - 4 - lw, field, S_FLOAT) && !ask) {
		stale = 1;
		rebuild();
	}

	/* the tree: the one chosen in sight */
	if(sel >= nrows)
		sel = nrows - 1;
	if(sel < 0)
		sel = 0;
	if(sel < top)
		top = sel;
	if(sel >= top + lh)
		top = sel - lh + 1;
	if(top > 0 && top > nrows - lh)
		top = nrows - lh > 0 ? nrows - lh : 0;
	lf = ui_focusable(ID_P_LIST, W_LIST, x + 1, y + 3, tx2);
	if(ui_key == K_MOUSE && ui_mouse_ok() && !cm) {
		int in = ms->x > x && ms->x <= tx2 && ms->y >= y + 3 && ms->y < y + 3 + lh;

		i = in ? top + ms->y - (y + 3) : -1;
		if(i >= nrows)
			i = -1;
		if(ms->kind == M_PRESS)
			hov = i;
		if(ms->kind == M_MOVE || ms->kind == M_DRAG) {
			if(hov != i)
				ui_again = 1;
			hov = i;	/* the preview: of the one under the mouse */
		} else if(in && (ms->kind == M_WHEELUP || ms->kind == M_WHEELDOWN)) {
			sel += ms->kind == M_WHEELUP ? -WHEEL : WHEEL;
			ui_key = 0;
		} else if(px1 && ms->x >= px1 && ms->x < x + w - 1 && ms->y >= y + 3 &&
		    ms->y < y + 3 + lh && (ms->kind == M_WHEELUP || ms->kind == M_WHEELDOWN)) {
			pv_scroll += ms->kind == M_WHEELUP ? -WHEEL : WHEEL;
			ui_key = 0;
		} else if(in && ms->kind == M_PRESS && ms->button == MB_RIGHT) {
			if(i >= 0)
				sel = i;
			ui_steal(ID_P_LIST);
			cm_open(i, ms->x, ms->y);	/* on no row: the folder shown */
			ui_key = 0;
		} else if(in && ms->kind == M_PRESS && ms->button == MB_LEFT && i >= 0) {
			sel = i;
			ui_steal(ID_P_LIST);
			if(ms->clicks == 2)
				act = i;
			else if(rows[i] && rows[i]->dir &&
			    ms->x == x + 2 + rows[i]->depth * 2)	/* its arrow */
				activate(i, 0);
			ui_key = 0;
		}
		ui_again = 1;
	}
	if(stale)
		rebuild();
	for(i = 0; i < lh; i++) {
		r = top + i;
		at = r == sel && (lf || ui_focus == ID_P_NAME) ? S_FOCUS : r == sel ?
		    S_SELECTION : S_LIST;
		scr_fill(x + 1, y + 3 + i, tx2, y + 3 + i, ' ', at);
		if(r < nrows)
			draw_row(r, x + 2, y + 3 + i, tx2, at);
	}
	if(nrows == 0)
		scr_text(x + 3, y + 3, query.len ? "Nothing matches in the folders open" :
		    "The folder is empty", S_LIST | F_DIM, tx2);
	if(px1) {		/* the preview */
		for(i = 0; i < lh; i++)
			scr_put(tx2 + 1, y + 3 + i, G_V, S_FLOAT);
		scr_fill(px1, y + 3, x + w - 2, y + 3 + lh - 1, ' ', S_TEXT);
		view = hov >= 0 && hov < nrows && rows[hov] ? hov : sel;
		draw_view(view, px1 + 1, y + 3, x + w - 3, y + 3 + lh - 1);
	}
	scr_text(x + 2, y + h - 3, ask ? ask_hint[ask] : hint, S_FLOAT | F_DIM, x + w - 2);
	/* under it, as in Open File: Hidden, [New Folder] [Trash (Del)]; at
	   the right [Open] [Close] */
	hw = text_width("Hidden (Alt+H)") + 5;
	cx = x + w - 3 - text_width("Close") - 1;
	bx = cx - 3 - text_width("Open") - 1;
	if(ui_check(ID_P_HIDDEN, x + 2, y + h - 2, "Hidden (Alt+H)", &st.phidden, S_FLOAT)) {
		settings_find_changed();
		explorer_changed();
	}
	if(x + 2 + hw + 2 + text_width("New Folder") + 4 + text_width("Trash (Del)") + 2 < bx - 2) {
		if(ui_button(ID_P_NEWDIR, x + 2 + hw + 2, y + h - 2, "New Folder", 'N', S_FLOAT))
			ask_begin(ASK_NEWDIR, sel);
		if(ui_button(ID_P_TRASH, x + 2 + hw + 2 + text_width("New Folder") + 4, y + h - 2,
		    "Trash (Del)", 0, S_FLOAT))
			do_trash(sel);
	}
	if(bx > x + 2 + hw && ui_button(ID_P_OPEN, bx, y + h - 2, "Open", 0, S_FLOAT) && !ask)
		act = sel;
	if(cm && !over)
		cm_draw();
	if((cx > x + 2 + hw && ui_button(ID_P_CLOSE, cx, y + h - 2, "Close", 0, S_FLOAT)) |
	    ui_modal_end()) {
		explorer_close();
		return;
	}
	if(props)
		draw_props();
	if(act >= 0)
		activate(act, 0);
	if(stale)
		ui_again = 1;
}
