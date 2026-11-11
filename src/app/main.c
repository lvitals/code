/* Code Editor (cedit in a terminal, xcedit in a window), in portable C99.
 * Here: the
 * documents, what the menus and the shortcuts do, and the passes over the
 * screen. */
#include <string.h>
#include "app.h"
#include "../core/mem.h"

struct state st;
int ndocs;
char no_line_numbers;

#define COUNT_STEP	((pos_t)1 << 20)	/* the bytes counted between keys */

void
error(const char *msg, const char *name)
{
	size_t n = strlen(msg), k = name ? strlen(name) : 0;
	char *s;
	int i;

	if((s = mem_alloc(n + k + 1)) == 0)
		return;
	memcpy(s, msg, n);
	if(k)
		memcpy(s + n, name, k);
	s[n + k] = 0;
	if(st.nerr == ERR_MAX) {	/* the oldest goes */
		mem_free(st.errors[0]);
		for(i = 1; i < ERR_MAX; i++)
			st.errors[i - 1] = st.errors[i];
		st.nerr--;
	}
	st.errors[st.nerr++] = s;
	ui_again = 1;
}

/* --- the paths ----------------------------------------------------------- */

int
issep(int c)
{
	return c == path_sep || (path_sep == '\\' && c == '/');
}

/* the root of p: "/", or "A:\", "A:", "\" */
size_t
rootlen(const char *p)
{
	if(path_sep == '/')
		return p[0] == '/';
	if(p[0] && p[1] == ':')
		return issep(p[2]) ? 3 : 2;
	return issep(p[0]);
}

const char *
path_last(const char *p)
{
	const char *s = p + rootlen(p), *q;

	for(q = s; *q; q++)
		if(issep(*q))
			s = q + 1;
	return s;
}

/* --- the documents ------------------------------------------------------- */

struct doc *docs;

static void
set_path(const char *path)
{
	char *p = mem_strdup(path);

	if(p == 0)
		return;
	mem_free(D->path);
	D->path = p;
	strncpy(D->name, path_last(p), sizeof D->name - 1);
	D->name[sizeof D->name - 1] = 0;
}

/* once, when the file is opened or saved: not while typing. */
static void
doc_language(void)
{
	D->lang = (unsigned char)lang_detect(D->path ? D->path : D->name);
	bar_stale = 1;
}

/* a new document, active, with an empty text: 0 without memory */
static int
doc_add(void)
{
	rows_stale = 1;
	struct doc *d = mem_alloc(sizeof *d);

	if(d == 0) {
		error("Out of memory", 0);
		return 0;
	}
	memset(d, 0, sizeof *d);
	d->next = docs;
	docs = d;
	B = &d->b;
	ndocs++;
	buf_init();
	D->anchor = POS_NONE;
	D->mark = POS_NONE;
	D->pref = -1;
	settings_apply(D);
	ui_again = 1;
	return 1;
}

void
doc_new(void)
{
	int n = 0;
	char *p;
	struct doc *o;

	for(o = docs; o; o = o->next)
		if(o->counter > n)
			n = o->counter;
	if(!doc_add())
		return;
	D->counter = n + 1;
	strcpy(D->name, "Untitled-");
	p = D->name + strlen(D->name);
	if(D->counter >= 10)
		*p++ = (char)('0' + D->counter / 10 % 10);
	*p++ = (char)('0' + D->counter % 10);
	strcpy(p, ".txt");
	doc_language();
}

int
doc_open(const char *path)
{
	int r, kind = file_kind(path);
	struct doc *d;

	for(d = docs; d; d = d->next)
		if(d->path && strcmp(d->path, path) == 0) {
			doc_activate(d);
			return 1;
		}
	if(kind == 2) {
		error("Is a directory: ", path);
		return 0;
	}
	/* a pristine Untitled one gives its place */
	if(docs && !D->path && !buf_dirty(B) && buf_len() == 0)
		doc_close();
	jump_push();	/* where it was in the one it leaves */
	if(!doc_add())
		return 0;
	set_path(path);
	if(kind == 1) {
		r = buf_load(path, ENC_AUTO);
		if(r == B_NOFILE) {
			error("Cannot open ", path);
			doc_close();
			return 0;
		}
		if(r == B_NOMEM)
			error("Out of memory, the file was cut: ", path);
	}
	doc_language();
	return 1;
}

void
doc_close(void)
{
	rows_stale = 1;
	if(docs == 0)
		return;
	jump_forget(D);
	buf_free();
	mem_free(D->path);
	mem_free(D->mc);
	ndocs--;
	{
		struct doc *d = docs;

		docs = d->next;
		mem_free(d);
		B = docs ? &docs->b : 0;
	}
	ui_again = 1;
}

void
doc_activate(struct doc *d)
{
	rows_stale = 1;
	struct doc **pp;

	for(pp = &docs; *pp && *pp != d; pp = &(*pp)->next)
		;
	if(*pp == 0 || d == docs)
		return;
	jump_push();	/* where it was in the one it leaves */
	*pp = d->next;
	d->next = docs;
	docs = d;
	B = &d->b;
	ui_again = 1;
}

int
doc_save(const char *path)
{
	int r = buf_save(path);

	if(r == B_ENCODE) {	/* nothing written: the character shown */
		char msg[120];
		pos_t at = buf_bad;

		strcpy(msg, "Line ");
		udec(msg + strlen(msg), buf_lineno(at) + 1);
		strcat(msg, " has a character not in ");
		strcat(msg, enc_names[(int)B->enc]);
		strcat(msg, " (selected), not saved: ");
		error(msg, path);
		jump_push();
		mc_clear();
		D->anchor = at;
		D->cur = buf_next(at);
		D->pref = -1;
		cursor_visible();
		return 0;
	}
	if(r != B_OK) {
		error("Cannot write ", path);
		return 0;
	}
	if(D->path == 0 || strcmp(D->path, path))
		set_path(path);
	doc_language();
	ui_again = 1;
	settings_saved(path);	/* the preferences: taken at once */
	return 1;
}

void
doc_reload(const char *path)
{
	struct doc *d;
	struct buf *keep = B;
	int r;

	for(d = docs; d; d = d->next)
		if(d->path && strcmp(d->path, path) == 0 && !buf_dirty(&d->b)) {
			int was = d->b.enc;

			B = &d->b;
			r = buf_load(path, ENC_AUTO);
			/* an 8-bit one chosen stays: they cannot be told apart */
			if(r == B_OK && ENC_8BIT(was) && ENC_8BIT(B->enc) && B->enc != was)
				r = buf_load(path, was);
			if(r == B_OK) {
				if(d->cur > buf_len())
					d->cur = buf_len();
				d->anchor = POS_NONE;
				if(d->top > buf_len())
					d->top = 0;
				d->warned = 0;
			}
			B = keep;
			rows_stale = 1;
			edit_gen++;
		}
}

/* the repository the project's folder (the current one) is in: .git
 * above it; or that folder itself: ended by a '/' */
void
project_root(char *root, int max)
{
	char probe[PATH_MAX_ + 8];
	int n;

	plat_cwd(root, max);
	for(n = (int)strlen(root); n > 1; ) {
		memcpy(probe, root, (size_t)n);
		strcpy(probe + n, ".git");
		if(file_kind(probe) != 0) {
			root[n] = 0;
			return;
		}
		for(n--; n > 0 && root[n - 1] != '/'; n--)	/* the folder above */
			;
	}
	plat_cwd(root, max);
}

char project_name[64];
unsigned project_gen;

void
project_update(void)
{
	char root[PATH_MAX_];
	const char *b;
	size_t n;

	project_root(root, sizeof root);
	n = strlen(root);
	while(n > 1 && root[n - 1] == path_sep)
		n--;
	for(b = root + n; b > root && b[-1] != path_sep; b--)
		;
	if(n == 1)	/* the root of all: / */
		b = root;
	n = (size_t)(root + n - b);
	if(n >= sizeof project_name)
		n = sizeof project_name - 1;
	memcpy(project_name, b, n);
	project_name[n] = 0;
	project_gen++;
	bar_stale = 1;
}

int
project_set(const char *dir)
{
	char path[PATH_MAX_];
	size_t n = strlen(dir);

	if(n + 2 > sizeof path || !plat_chdir(dir)) {
		error("Cannot work in the folder ", dir);
		return 0;
	}
	memcpy(path, dir, n + 1);
	if(n == 0 || path[n - 1] != path_sep) {	/* ended by its separator */
		path[n] = path_sep;
		path[n + 1] = 0;
	}
	strcpy(st.pdir, path);
	project_update();
	return 1;
}

/* the active document read again from its file, in enc: 0 if it could
 * not be (an error was shown) */
int
doc_reopen(int enc)
{
	int r;

	if(D->path == 0) {
		error("Not saved yet: ", D->name);
		return 0;
	}
	if(buf_dirty(B)) {
		error("Save or undo the changes before reopening: ", D->path);
		return 0;
	}
	r = buf_load(D->path, enc);
	if(r == B_NOFILE) {
		error("Cannot open ", D->path);
		return 0;
	}
	if(r == B_NOMEM)
		error("Out of memory, the file was cut: ", D->path);
	mc_clear();
	jump_forget(D);
	if(D->cur > buf_len())
		D->cur = buf_len();
	D->cur = buf_linestart(D->cur);
	D->anchor = POS_NONE;
	D->mark = POS_NONE;
	D->top = buf_linestart(D->top > buf_len() ? 0 : D->top);
	D->warned = 0;
	rows_stale = 1;
	edit_gen++;
	ui_again = 1;
	return 1;
}

/* --- Save and Exit ------------------------------------------------------- */

void
draw_save(void)
{
	st.wants_save = 0;
	if(docs == 0)
		return;
	if(docs->path)
		doc_save(docs->path);
	else
		picker_open(PICK_SAVEAS);	/* no name: Save As */
	ui_again = 1;
}

void
draw_exit(void)
{
	while(docs) {
		if(buf_dirty(B)) {
			st.wants_close = 1;
			return;
		}
		doc_close();
	}
	st.exit = 1;
}

/* --- the commands -------------------------------------------------------- */

void
app_action(int a)
{
	ui_again = 1;
	switch(a) {
	case A_NEW:
	case A_OPEN:
		if(a == A_NEW)
			doc_new();
		else
			picker_open(PICK_OPEN);
		return;
	case A_FINDFILE:
	case A_FINDTEXT:
		finder_open(a == A_FINDFILE ? FINDER_FILES : FINDER_TEXT);
		return;
	case A_OPENFOLDER:
		picker_open(PICK_FOLDER);
		return;
	case A_EXIT:
		st.wants_exit = 1;
		return;
	case A_ABOUT:
		st.wants_about = 1;
		return;
	case A_PREFS:	/* opens the settings as a document: none needed before */
		settings_open();
		return;
	case A_ZOOMIN: case A_ZOOMOUT: case A_ZOOMRESET:	/* the window's text */
		if(term_zoom(a == A_ZOOMIN ? 1 : a == A_ZOOMOUT ? -1 : 0))
			settings_set_font_size(term_font_size());
		return;
	}
	if(docs == 0)
		return;
	switch(a) {
	case A_SAVE:
		st.wants_save = 1;
		break;
	case A_SAVEAS:
		picker_open(PICK_SAVEAS);
		break;
	case A_ENCODING:
		st.wants_indent = 0;
		st.wants_enc = 1;
		break;
	case A_CLOSE:
		st.wants_close = 1;
		break;
	case A_UNDO:
		cmd_undo(0);
		break;
	case A_REDO:
		cmd_undo(1);
		break;
	case A_CUT:
		cmd_cut(1);
		break;
	case A_COPY:
		cmd_cut(0);
		break;
	case A_PASTE:
		cmd_paste();
		break;
	case A_SELALL:
		cmd_selectall();
		break;
	case A_DELHEAD:
		cmd_delete_linepart(0);
		break;
	case A_DELTAIL:
		cmd_delete_linepart(1);
		break;
	case A_MARK:
		cmd_mark(0);
		break;
	case A_JUMPMARK:
		cmd_mark(1);
		break;
	case A_FIND:
	case A_REPLACE:
		st.search = a == A_FIND ? SEARCH_FIND : SEARCH_REPLACE;
		st.search_focus = 1;
		break;
	case A_FINDNEXT:
		search_execute(SEARCH);
		break;
	case A_COMPLETE:
		complete_start();
		break;
	case A_GOMATCH:
		cmd_gomatch();
		break;
	case A_FILTER:
		st.wants_filter = 1;
		break;
	case A_MCNEXT:
		mc_next();
		break;
	case A_MCUP:
	case A_MCDOWN:
		mc_vertical(a == A_MCUP ? -1 : 1);
		break;
	case A_BACK:
	case A_FORWARD:
		jump_go(a == A_BACK ? -1 : 1);
		break;
	case A_EXPAND:
	case A_SHRINK:
		cmd_expand(a == A_EXPAND);
		break;
	case A_STATUS:
		st.wants_status = 1;
		break;
	case A_GOTOFILE:
		st.wants_gotofile = 1;
		break;
	case A_GOTO:
		st.wants_goto = 1;
		break;
	case A_WRAP: case A_AUTOINDENT: case A_SHOWTABS: case A_SHOWEOL:
	case A_SHOWEOF: case A_LINENUMS: case A_RELNUMS: case A_CURLINE: case A_SYNTAX:
	case A_MATCHBR: case A_BSINDENT: case A_GUIDES:
		settings_toggle(a);	/* on or off, and kept */
		rows_stale = 1;
		bar_stale = 1;
		break;
	}
	if(a >= A_UNDO && a <= A_SELALL)
		cursor_visible();
}

static int
shortcut(kcode k)
{
	switch(k) {
	case K_CTRL | 'N': return A_NEW;
	case K_CTRL | 'O': return A_OPEN;
	case K_CTRL | 'S': return A_SAVE;
	case K_CTRL | K_SHIFT | 'S': return A_SAVEAS;
	case K_CTRL | 'W': return A_CLOSE;
	case K_CTRL | 'P': return A_GOTOFILE;
	case K_CTRL | 'Q': return A_EXIT;
	case K_CTRL | 'G': return A_GOTO;
	case K_CTRL | 'F': return A_FIND;
	case K_CTRL | 'R': return A_REPLACE;
	case K_CTRL | K_SHIFT | 'H': return A_DELHEAD;
	case K_CTRL | K_SHIFT | 'G': return A_DELTAIL;
	case K_ALT | 'M': return A_MARK;
	case K_ALT | 'N': return A_LINENUMS;
	case K_ALT | 'R': return A_RELNUMS;
	case K_CTRL | '=': case K_CTRL | '+': case K_CTRL | K_SHIFT | '=':
	case K_CTRL | K_SHIFT | '+': return A_ZOOMIN;
	case K_CTRL | '-': return A_ZOOMOUT;
	case K_CTRL | '0': return A_ZOOMRESET;
	case K_ALT | 'J': return A_JUMPMARK;
	case K_ALT | 'O': return A_OPENFOLDER;
	case K_ALT | 'P': return A_FINDFILE;
	case K_ALT | '/': return A_FINDTEXT;
	case K_ALT | '|': case K_ALT | K_SHIFT | '|': case K_ALT | K_SHIFT | '\\':
		return A_FILTER;
	case K_ALT | K_LEFT: return A_BACK;
	case K_ALT | K_RIGHT: return A_FORWARD;
	case K_F3: return A_FINDNEXT;
	}
	return A_NONE;
}

/* --- a pass over the screen ---------------------------------------------- */

static int
modal_open(void)
{
	return st.wants_close || st.wants_goto || st.picker || st.wants_gotofile ||
	    st.wants_about || st.wants_filter || st.wants_reload || st.finder || st.nerr;
}


static int
draw(kcode key)
{
	int sh = st.search == SEARCH_REPLACE ? 3 : st.search == SEARCH_FIND ? 2 : 0, a;
	kcode menukey;

	static char over;	/* a menu or a modal was over the text */

	ui_pass(key);
	scr_lower(0, -1, -1);	/* the picker asks for it again */
	if(!modal_open())
		menu_shortcut();
	if(drawing) {
		if(over)
			rows_stale = 1;
		over = modal_open() || st.menu >= 0 || st.wants_indent || st.wants_enc || complete_open();
		if(over)
			rows_stale = 1;
		if(rows_stale)
			bar_stale = 1;
	}
	/* the bar: drawn, or a menu open, or the mouse on it */
	if(drawing || (st.menu >= 0 && ui_key) ||
	   (ui_key == K_MOUSE && term_mouse.y == 0 && !modal_open()))
		draw_menubar();
	/* a menu open has the mouse first (it is over the rest, drawn after
	   it): what is under it does not see the event */
	menukey = 0;
	if(st.menu >= 0 && ui_key == K_MOUSE && term_mouse.y > 0) {
		menukey = ui_key;
		ui_key = 0;
	}
	if(sh)
		draw_search(1);
	if(st.search == SEARCH_HIDDEN)
		sh = 0;
	/* Intermediate focus-settling passes carry no key and draw nothing:
	   the editor is not traversed for them. */
	if(docs && !modal_open() && st.menu < 0)
		complete_key();		/* the list of words has its keys first */
	if(drawing || ui_key)
		draw_editor(1 + sh, scr_rows - 2 - sh);
	if(drawing && docs)
		draw_complete();
	/* the bar: drawn, or its focus, or the mouse on it (its buttons) */
	if(drawing || (ui_focus >= ID_ST_NL && ui_focus <= ID_ENC_REOPEN) ||
	   (ui_key == K_MOUSE && term_mouse.y == scr_rows - 1))
		draw_statusbar(scr_rows - 1);	/* else it shows nothing new */
	draw_indent_picker(scr_rows - 1);
	draw_enc_picker(scr_rows - 1);
	if(menukey)
		ui_key = menukey;
	if(drawing || ui_key)
		draw_menus();
	else if(st.menu >= 0)
		ui_steal(ID_MENU);	/* keep focus without laying the menu out again */
	if(st.wants_close)
		draw_unsaved();
	if(st.wants_exit)
		draw_exit();
	if(st.wants_goto)
		draw_goto();
	if(st.wants_filter)
		draw_filter();
	if(st.finder)
		draw_finder();
	if(st.wants_reload)
		draw_reload();
	if(st.picker)
		draw_picker();
	if(st.overwrite)
		draw_overwrite();
	if(st.trash)
		draw_trash();
	if(st.wants_save)
		draw_save();
	if(st.wants_gotofile)
		draw_gotofile();
	if(st.wants_about)
		draw_about();
	if(st.nerr)
		draw_errors();
	if(ui_key && ui_key != K_MOUSE && ui_key != K_NONE && !modal_open() &&
	   (a = shortcut(ui_key)) != A_NONE) {
		ui_key = 0;
		app_action(a);
	}
	return ui_end(docs ? ID_EDITOR : ID_NONE);
}

int
main(int argc, char **argv)
{
	int cols, rows, i, r = app_start(argc, argv, &cols, &rows);
	pos_t stamp;
	kcode key = K_NONE;

	if(r >= 0)
		return r;
	if(!scr_init(cols, rows)) {
		term_close();
		plat_print(plat_name);
		plat_print(": out of memory\r\n");
		return 1;
	}
	ui_focus = ID_EDITOR;
	if(docs)
		cursor_visible();
	while(!st.exit) {
		drawing = 0;	/* the text is shown in the last pass only */
		draw(key);
		for(i = 0; i < 4 && ui_again && !st.exit; i++)
			draw(K_NONE);
		if(st.exit)
			break;
		drawing = 1;
		draw(K_NONE);
		/* Flush every incremental update before waiting for a key, so
		   a held arrow never outruns the visible cursor. */
		scr_flush();
		term_cursor(ui_cx, ui_cy, ui_cx >= 0);
		if(docs)
			term_title(docs->name, buf_dirty(B));
		file_idle();
		/* another program changed the file, or cut it while mapped */
		if(file_lost()) {
			error("Part of a file was lost: another program cut it", 0);
			key = K_NONE;
			continue;
		}
		/* another program changed the file: read again if asked (when
		   nothing here is unsaved); else said, the changes here kept */
		if(docs && D->path && B->stamp && !modal_open() &&
		   (stamp = file_stamp(D->path)) != B->stamp && stamp != D->warned) {
			D->warned = stamp;	/* once for each change */
			if(stamp == 0)
				error("Deleted by another program (save to keep it): ", D->path);
			else if(!buf_dirty(B))
				st.wants_reload = 1;
			else
				error("Changed on the disk by another program; the unsaved "
				    "changes here are kept (save to overwrite it): ", D->path);
			key = K_NONE;
			continue;
		}
		/* idle: the finder walks and searches a slice at a time */
		if(finder_busy()) {
			finder_work();
			if(!term_keyready()) {
				key = K_NONE;
				continue;
			}
		}
		/* idle: the lines counted a slice at a time; drawn again every
		   so often (the numbers, the progress), and when all are */
		if(docs && D->linenums && !buf_lines_known()) {
			for(i = 0; i < 32 && !term_keyready() && buf_count_step(COUNT_STEP); i++)
				;
			if(!term_keyready()) {
				key = K_NONE;
				continue;
			}
		}
		do	/* a move of the mouse matters only to an open menu, and
			   onto a button or off one (its hover) */
			key = term_key();
		while(key == K_MOUSE && term_mouse.kind == M_MOVE && st.menu < 0 &&
		    !ui_hover_moved(term_mouse.x, term_mouse.y));
		if(key == K_RESIZE) {	/* a new size: all of it drawn again */
			if(term_size(&cols, &rows) && !scr_init(cols, rows))
				break;
			rows_stale = 1;
			bar_stale = 1;
			ui_scrollbar_again();
			ui_hscrollbar_again();
		}
	}
	term_close();
	return 0;
}
