/* The editor: the documents and the state of the screen, and what each
 * part of the screen draws. */
#ifndef APP_H
#define APP_H

#include "../core/buf.h"
#include "../tui/ui.h"

#define PATH_MAX_	1024
#define ERR_MAX		10
#define DOTS		"\xe2\x80\xa6"	/* … */
#define KEY_CTRL	"Ctrl"
#define KEY_ALT		"Alt"
/* a text this small has its lines counted at once */
#define SMALL_TEXT	((pos_t)4 << 20)

/* what the menus and the shortcuts do */
enum {
	A_NONE, A_NEW, A_OPEN, A_SAVE, A_SAVEAS, A_CLOSE, A_EXIT,
	A_UNDO, A_REDO, A_CUT, A_COPY, A_PASTE, A_FIND, A_REPLACE, A_SELALL,
	A_DELHEAD, A_DELTAIL, A_MARK, A_JUMPMARK,
	A_PREFS, A_STATUS, A_GOTOFILE, A_GOTO, A_WRAP, A_AUTOINDENT, A_SHOWTABS, A_SHOWEOL, A_SHOWEOF,
	A_LINENUMS, A_RELNUMS, A_CURLINE, A_SYNTAX,
	A_ZOOMIN, A_ZOOMOUT, A_ZOOMRESET, A_MATCHBR, A_BSINDENT, A_GUIDES,
	A_ABOUT, A_FINDNEXT, A_COMPLETE, A_GOMATCH, A_EXPAND, A_SHRINK,
	A_BACK, A_FORWARD, A_FILTER, A_MCNEXT, A_MCUP, A_MCDOWN, A_ENCODING,
	A_FINDFILE, A_FINDTEXT, A_OPENFOLDER
};

/* a cursor besides the document's own (Alt+Click, Ctrl+D) */
struct mcur {
	pos_t cur, anchor;
	int pref;
	char prim;	/* while all are moved together: the document's own */
};

struct doc {
	struct doc *next;
	struct buf b;
	char *path;		/* the full name; 0: untitled */
	char name[40];		/* the last part of it, as the status bar shows */
	int counter;		/* of an untitled one: Untitled-N.txt */
	pos_t cur, anchor;	/* anchor POS_NONE: no selection */
	pos_t mark;		/* the saved cursor position (Alt+M, Alt+J) */
	pos_t top;		/* the start of the first row shown */
	int left, pref;		/* the first column shown; the column of Up and Down */
	char wrap, overtype, autoindent, showtabs, showeol, showeof;
	char linenums;		/* the line numbers shown, and counted */
	char relnums;		/* relative to the line of the cursor */
	char curline;		/* the line of the cursor highlighted */
	char syntax;		/* syntax highlighting */
	char matchbr;		/* the bracket at the cursor and its match shown */
	char bsindent;		/* Backspace in the indentation: a whole level */
	char guides;		/* indent guides */
	char line_copy;
	unsigned char lang;	/* what the status bar names; 0 is plain text */
	pos_t warned;		/* the file as it was when told it changed */
	struct mcur *mc;	/* the other cursors, in the order of the text */
	int nmc, mccap;
};

/* the widgets that take the focus */
enum {
	ID_NONE, ID_EDITOR, ID_MENU,
	ID_FIND, ID_REPLACE, ID_CASE, ID_WORD, ID_REGEX, ID_REPLALL, ID_CLOSE,
	ID_ST_NL, ID_ST_ENC, ID_ST_INDENT, ID_ST_PROJECT, ID_ST_FILE, ID_ST_OVR,
	ID_IND_TYPE, ID_IND_WIDTH, ID_ENC_LIST, ID_ENC_SAVE, ID_ENC_REOPEN,
	ID_OK, ID_YES, ID_NO, ID_CANCEL, ID_GOTO, ID_LIST,
	ID_P_NAME, ID_P_LIST, ID_P_SUGGEST, ID_P_HIDDEN, ID_P_FOLDER, ID_FILTER, ID_FINDER
};

enum { SEARCH_HIDDEN, SEARCH_FIND, SEARCH_REPLACE };
enum { PICK_NONE, PICK_OPEN, PICK_SAVEAS, PICK_FOLDER };
enum { FINDER_NONE, FINDER_FILES, FINDER_TEXT };	/* st.finder */

struct state {
	int search, search_focus;
	struct edl needle, repl;
	char mcase, wword, regex, sfail;
	char phidden;	/* the file picker shows the hidden files (.name) */

	int picker;
	char pdir[PATH_MAX_];
	struct edl pname;
	char *overwrite;	/* the file the warning is about */

	int mx, my;	/* where the context menu opens */
	char mdrag;	/* a button held since the menu opened */
	int menu, mitem;	/* the open menu (-1 none), its focused item */
	int mtop;		/* its first item shown, when not all fit */

	char wants_save, wants_close, wants_exit, wants_goto, wants_about;
	char wants_gotofile, wants_indent, wants_status, wants_filter;
	char wants_enc;		/* the encoding picker is open */
	char finder;		/* the project finder: FINDER_FILES, FINDER_TEXT */
	char wants_reload;	/* the file changed on the disk: read it again? */
	struct edl gototarget;
	char goto_invalid;

	char *errors[ERR_MAX];
	int nerr;
	char exit;
};

extern struct state st;
extern int ndocs;
/* D, the active document; docs, it or 0 when there is none */
extern struct doc *docs;	/* the active one first */
#define D		docs
#define DBUF(d)		(&(d)->b)	/* the text of a document */
extern char *clip;		/* the clipboard */
extern pos_t cliplen;
extern char clip_line;		/* a whole line, copied without a selection */

/* settings.c */
void settings_load(void);
void settings_apply(struct doc *d);
int settings_toggle(int a);
void settings_find_changed(void);
int settings_font_size(void);	/* of a window: 0, its default */
void settings_use_font(void);	/* the font of the settings, to a window */
int settings_ruler(void);	/* the column marked: 0, none */
void settings_set_font_size(int px);
/* File > Preferences: the file of the settings, open as a document (made
 * first, with the values, if there is none) */
void settings_open(void);
/* a document saved at path: the settings again, if it is their file */
void settings_saved(const char *path);

/* main.c */
void app_action(int a);
/* start.c */
int app_start(int argc, char **argv, int *cols, int *rows);
void error(const char *msg, const char *name);
void doc_new(void);
int doc_open(const char *path);
/* the document of path, if open and not changed: read again */
void doc_reload(const char *path);	/* 0: an error was shown */
/* the folder of the project: the one worked in (Open and Save As start
 * there; Alt+P and Alt+/ look in the project found from it): 0 if it
 * cannot be (an error was shown) */
int project_set(const char *dir);
/* the project: the repository of the project's folder (the first folder
 * above it with a .git), or that folder; ended by a separator */
void project_root(char *root, int max);
void project_update(void);	/* the project's folder may have changed */
extern char project_name[64];	/* the last part of project_root */
extern unsigned project_gen;	/* one more each time it changes */
int doc_reopen(int enc);		/* the active one, read again in enc */
void doc_close(void);			/* the active one */
void doc_activate(struct doc *d);
int doc_save(const char *path);	/* the active one */
const char *path_last(const char *p);
int issep(int c);
size_t rootlen(const char *p);

/* paths.c */
void path_join(char *out, const char *dir, const char *name);
void path_normalize(char *p);

/* editor.c */
extern char drawing;	/* the pass shows the text */
extern char rows_stale;	/* all the rows of the text are drawn again */
extern pos_t edit_low, edit_high, edit_delta;	/* the edit since the text was shown */
extern unsigned edit_gen;	/* one more for each change of the text */
extern char no_line_numbers;	/* -n: the documents open without them */
int bar_lines(void);
extern int edit_lines;		/* the lines it added (< 0: took) */
void draw_editor(int y, int h);
void editor_key(void);		/* the keys of the editor, if it has the focus */
void cmd_undo(int redo);
void cmd_cut(int cut);
void cmd_paste(void);
void cmd_selectall(void);
void cmd_delete_linepart(int tail);
void cmd_mark(int jump);
void cmd_goto(pos_t line, pos_t col);
void cmd_gomatch(void);		/* to the other end of a bracket, a block */
void cmd_expand(int grow);	/* the selection grown a step, or shrunk */
/* more cursors: at the next match of the selection (Ctrl+D), a line
 * above or below (Shift+Alt+Up, Down); one again */
void mc_next(void);
void mc_vertical(int dir);
void mc_clear(void);
void cursor_visible(void);
pos_t sel_begin(void);
pos_t sel_end(void);
int has_sel(void);
char *sel_text(void);	/* the selection, 0 if none */
extern pos_t hit;	/* the start of the last match found */
int edit(pos_t off, pos_t ndel, const char *s, pos_t n, int kind);
void write_text(const char *s, pos_t n, int typed);

/* search.c */
void draw_search(int y);
enum { SEARCH, REPLACE, REPLACE_ALL };
void search_execute(int action);

/* complete.c: Complete Word (Ctrl+Space) and its list */
void complete_start(void);
int complete_open(void);
void complete_key(void);	/* before the editor: the keys of the list */
void draw_complete(void);	/* after it: the list, at the cursor */

/* filter.c: Filter Through Command (Alt+|) */
void draw_filter(void);

/* finder.c: the files of the project (Alt+P), the lines of their text
 * (Alt+/), with a preview */
void finder_open(int mode);
void finder_close(void);
void draw_finder(void);
int finder_busy(void);		/* work to do while idle */
void finder_work(void);		/* some of it, until a key waits */

/* jumps.c: the places the cursor jumped from (Alt+Left, Alt+Right) */
void jump_push(void);		/* before a jump: where the cursor is */
void jump_go(int dir);		/* back (-1) or forward (1) */
void jump_adjust(pos_t off, pos_t ndel, pos_t n);	/* an edit of D */
void jump_forget(struct doc *d);	/* it closes */

/* menu.c */
#define NMENUS		4
#define CTX_MENU	NMENUS	/* the context menu (the right button) */
extern int menu_x[NMENUS];	/* where each title is */
void draw_menubar(void);
int menu_shortcut(void);	/* Alt+letter and F10 */
int menu_shown(int m);
void open_menu(int m);
void close_menu(void);

/* menus.c */
void draw_menus(void);
int menu_checked(int a);	/* an option of the View menu: on */

/* syntax.c: the classes of the bytes of a line, from the state at its
 * start (0 at the start of the text); the state at its end */
enum { HL_TEXT, HL_KEYWORD, HL_TYPE, HL_STRING, HL_COMMENT, HL_NUMBER, HL_PREPROC,
	HL_DELETED, HL_COUNT };
int syn_line(int lang, const char *p, int n, int state, unsigned char *cls);
/* the style of a class of the syntax (HL_*), on the line of the cursor or not */
int hl_style(int cls, int curline);
int syn_has(int lang);	/* a language with highlighting */

/* lang.c: the language, from the name, then a shebang or the first
 * lines.  No language server. */
int lang_detect(const char *path);
int lang_find(const char *name);	/* by its name ("c"): 0 if none */
const char *lang_name(int id);

/* status.c */
void draw_statusbar(int y);
extern char bar_stale;
extern pos_t cur_line, cur_lstart;
void draw_indent_picker(int y);
void draw_enc_picker(int y);

/* dialogs.c */
int wrap_text(int x, int y, int w, const char *s, int at);
int buttons(int x0, int y, int w, const char *const *text, const char *accel,
    const int *ids, int n, int at);
int dialog_width(int want);
/* a modal of lines and a set of buttons: the one pressed (the last for
 * ESC), -1 none */
enum { MB_OK, MB_SAVE, MB_YESNO };
int msgbox(const char *title, const char *const *line, int n, int set, int at);
void draw_about(void);
void draw_goto(void);
void draw_unsaved(void);
void draw_errors(void);
void draw_overwrite(void);
void draw_gotofile(void);
void draw_exit(void);
void draw_save(void);
void draw_reload(void);

/* picker.c */
void draw_picker(void);
/* pickfile.c */
#define ENT_MAX		8192
#define NAMES_MAX	262144
extern char *pk_names;
extern unsigned *pk_ents;
extern int pk_n, pk_loaded, pk_sel, pk_top;
void picker_open(int kind);
void close_picker(void);
void picker_finish(const char *path);

#endif
