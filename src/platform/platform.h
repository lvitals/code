/* What a platform gives the editor: the terminal (a grid of cells, the
 * cursor, the keys), the files and directories, the memory.  posix/ is a
 * POSIX terminal (truecolor, UTF-8, the mouse). */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>

typedef unsigned int cell_t;	/* a Unicode code point */
typedef unsigned long pos_t;
#define CELL_WIDE	0x110000u	/* the right half of a wide character */
#define POS_NONE	((pos_t)-1)

/* The glyphs the interface draws, as cells below 20h (a control
 * character of the text is shown otherwise) */
enum {
	G_TL = 1, G_TR, G_BL, G_BR, G_H, G_V,	/* the frames */
	G_THUMB,		/* the scrollbar */
	G_CHECKED, G_UNCHECKED,	/* ✓, blank */
	G_WRAPDOT,		/* ∙ the margin of a wrapped line */
	G_ELLIPSIS,		/* … */
	G_CONTROL,		/* a control character of the text */
	G_TABVIEW, G_EOLVIEW, G_EOFVIEW,	/* the visible tab, line end, end of file */
	G_SCROLLUP, G_SCROLLDOWN,
	G_BOXON, G_BOXOFF,	/* ☑ ☐: a check box */
	G_SCROLLLEFT, G_SCROLLRIGHT, G_HTHUMB,	/* ◀ ▶ and a band: the horizontal bar */
	G_HBANDR, G_HBANDL,	/* half the band: a gap next to an arrow */
	G_TABLINE, G_TABEND,	/* Show Tabs: the cells of a tab after its first,
				   its last (a window draws a line to an arrow) */
	G_GUIDE,		/* │ an indent guide */
	G_SEPL, G_SEPR,		/* ├ ┤: a line across a frame (a menu's groups) */
	G_LAST
};

/* The looks of a cell: a style (the low 5 bits) and flags */
enum {
	S_TEXT, S_MARGIN, S_DOTS, S_SELECTION, S_SELBLUR, S_BAR, S_FLOAT,
	S_FOCUS, S_PANEL, S_FAIL, S_ALERT, S_LIST, S_TRACK, S_THUMB,
	S_CURLINE, S_CURNUM,	/* the line of the cursor: its text, its number */
	/* the syntax (HL_KEYWORD on): on another line, on that of the cursor */
	S_KEYWORD, S_TYPE, S_STRING, S_COMMENT, S_NUMBER, S_PREPROC, S_DELETED,
	S_CL_KEYWORD, S_CL_TYPE, S_CL_STRING, S_CL_COMMENT, S_CL_NUMBER, S_CL_PREPROC,
	S_CL_DELETED,
	S_HTRACK, S_HTHUMB,	/* the horizontal scrollbar: a band across its row */
	S_LAST			/* at most 32: STYLE() takes 5 bits */
};
#define F_UNDER		0x20	/* the accelerator of a menu */
#define F_REV		0x40	/* reversed: a focused button */
#define F_DIM		0x80	/* behind a modal; on a float, its text greyed */
#define F_HOVER		0x100	/* the mouse over it: a button, focused or not */
#define STYLE(a)	((a) & 0x1F)
typedef unsigned short attr_t;	/* the looks of a cell: a style and flags */

/* The keys: a character, or one of these, with the modifiers.  8 BS,
 * 9 TAB, 13 ENTER, 27 ESC; Ctrl and a letter is K_CTRL | 'A', Alt and a
 * letter K_ALT | 'A'.  They take more than 16 bits: a code point and the
 * modifiers. */
typedef long kcode;
typedef long rune;
#define K_FIRST		0x110000L
#define K_SHIFT		0x1000000L
#define K_CTRL		0x2000000L
#define K_ALT		0x4000000L
#define K_CODE(k)	((k) & 0xFFFFFFL)
#define K_UP		(K_FIRST + 0)
#define K_DOWN		(K_FIRST + 1)
#define K_LEFT		(K_FIRST + 2)
#define K_RIGHT		(K_FIRST + 3)
#define K_HOME		(K_FIRST + 4)
#define K_END		(K_FIRST + 5)
#define K_PGUP		(K_FIRST + 6)
#define K_PGDN		(K_FIRST + 7)
#define K_INS		(K_FIRST + 8)
#define K_DEL		(K_FIRST + 9)
#define K_F3		(K_FIRST + 10)
#define K_F10		(K_FIRST + 11)
#define K_RESIZE	(K_FIRST + 12)
#define K_MOUSE		(K_FIRST + 13)	/* see term_mouse */
#define K_PASTE		(K_FIRST + 14)	/* see term_paste */
#define K_NONE		(K_FIRST + 15)	/* nothing: a redraw */
#define K_F1		(K_FIRST + 16)
#define K_F2		(K_FIRST + 17)
#define K_F4		(K_FIRST + 18)
#define K_F5		(K_FIRST + 19)
#define K_MODS		(K_SHIFT | K_CTRL | K_ALT)

/* the mouse of the last K_MOUSE */
enum { M_PRESS, M_DRAG, M_RELEASE, M_WHEELUP, M_WHEELDOWN, M_MOVE, M_WHEELLEFT, M_WHEELRIGHT };
enum { MB_LEFT, MB_MIDDLE, MB_RIGHT };
struct mouse {
	int x, y, kind;
	int clicks;	/* 2 for a double click, 3 for a triple one */
	int button;	/* of a press, a drag or a release */
	kcode mods;
};
extern struct mouse term_mouse;
/* Shift is held now: a window knows it (a press or a release of Shift
 * alone brings a K_NONE, to be drawn again); a terminal tells it only
 * with the mouse (its last event) */
int term_shift(void);

/* the terminal: its size; 0 if it cannot be used */
int term_open(int *cols, int *rows);
void term_close(void);
int term_size(int *cols, int *rows);	/* after K_RESIZE */
/* n cells of row y from column x: their characters and looks */
void term_write(int x, int y, const cell_t *ch, const attr_t *at, int n);
void term_cursor(int x, int y, int visible);
void term_flush(void);
kcode term_key(void);		/* waits for one; K_NONE after a second
				   without any (the editor looks at its files) */
int term_keyready(void);	/* one waits: the screen may wait */
void term_beep(void);
void term_title(const char *name, int dirty);
/* cells x1 to x2 of row y, in a window, drawn a third of a cell lower:
 * a row above the bottom of a frame (its line runs through the middle
 * of its cell), nearer the middle; y < 0: none (a terminal: nothing) */
void term_lower(int x1, int x2, int y);
/* The size of the text, in a window (a terminal has its own: 0, and
 * term_zoom does nothing).  term_zoom: 1 larger, -1 smaller, 0 as it
 * was; 1 if it changed (a K_RESIZE follows). */
int term_font_size(void);
void term_set_font_size(int px);	/* before term_open; 0: the default */
/* before term_open: a window started from a terminal keeps it until it is
 * closed (else it gives it back once open); nothing in a terminal */
void term_stay(int on);
/* the font of a window (before term_open, or after: drawn again): a file
 * (.ttf .otf .ttc, ~/ for the home), else a family in the font folders
 * ("Name" or "Name:Style", read by the editor itself); "" the one in the
 * program.  One that fails leaves the font
 * as it was.  A terminal has its own: FONT_OK, nothing done */
enum { FONT_OK, FONT_NOTFOUND, FONT_BAD };
int term_set_font(const char *spec);
int term_zoom(int how);
/* the text of the last K_PASTE */
const char *term_paste(size_t *n);
/* the clipboard of the terminal (OSC 52): what was copied */
void term_clipboard(const char *s, size_t n);

/* the files: a handle, -1 when it fails.  write: 1 to write it, 0 to
 * read it, FILE_SCAN to read it for a search (on any thread): a regular
 * file only, opened without waiting (a pipe, a device: -1) */
#define FILE_SCAN	2
int file_open(const char *name, int write);
int file_read(int h, char *buf, int n);
int file_write(int h, const char *buf, int n);
int file_close(int h);		/* 0, or -1 when the data was not written */
int file_kind(const char *name);	/* 0 none, 1 file, 2 directory */
extern const int file_crlf;		/* the newline of a new file */
/* A large file is mapped, not read: the size of an open one (POS_NONE
 * if unknown), all of it mapped read-only (0 if it cannot be).  The bytes
 * stay as they were while mapped: a save writes a new file and renames it
 * over the old one, never into a mapped one.  snapshot: the text of a
 * document, kept from what another program writes into the file (a copy
 * that shares its blocks, where the file system makes one); 0: only read
 * a while (a preview). */
pos_t file_size(int h);
char *file_map(int h, pos_t size, int snapshot);
void file_unmap(char *p, pos_t size);
/* Another program may change a file: what it is now (its identity, size
 * and time; 0 if none), to see that it changed.  One that cuts a mapped
 * file makes its lost bytes read as zeros: file_lost says it happened
 * (once) instead of the editor being killed. */
pos_t file_stamp(const char *name);
int file_lost(void);
void file_idle(void);	/* now and then: a file mapped made safe from others */

/* the directories: the entries of one, a name and whether it is one */
int dir_open(const char *path);
int dir_next(char *name, int max, int *isdir);	/* 0 at the end */
void dir_close(void);
/* the next entry, for a walk of a tree: its kind (0 at the end).  A link
 * to a file is a file; a link to a folder is DK_OTHER (not followed: a
 * walk cannot loop) */
enum { DK_END, DK_FILE, DK_DIR, DK_OTHER };
int dir_entry(char *name, int max);
/* the file of the settings (its folder made): 0 if there is none */
int plat_config(char *buf, int max);
/* sh -c cmd in dir (0: here), the n bytes of in as its input: its status
 * (-1 not run, -2 stopped, too long), what it wrote to *out and to *err
 * (each freed with free) */
int plat_run(const char *cmd, const char *dir, const char *in, size_t n,
    char **out, size_t *outn, char **err, size_t *errn);
void plat_cwd(char *buf, int max);	/* the current directory, ended
					   by its separator */
int plat_chdir(const char *path);	/* it made the current one: 0 if not */
/* a new folder: 0, or why it was not made */
const char *dir_make(const char *path);
/* a file or a folder moved to the Trash of the desktop (the freedesktop.org
 * Trash: the one the file managers show, and restore from): 0, or why it
 * was not moved */
const char *plat_trash(const char *path);
/* a file or a folder (all that is in it) copied to to, a name that is
 * free; a link copied as a link: 0, or why not (what was made of to is
 * taken away) */
const char *plat_copy(const char *from, const char *to);
/* moved (or renamed) to to, a name that is free; to another file system,
 * copied and then removed: 0, or why not */
const char *plat_move(const char *from, const char *to);
/* a file, or a folder and all that is in it, deleted for good (not to the
 * Trash; a link: the link, not what it points to): 0, or why not all of it */
const char *plat_delete(const char *path);
/* what File > Explorer > Properties shows of a file or a folder */
struct file_info {
	int kind;		/* 1 file, 2 folder, 3 another kind (a device ...) */
	int link;		/* a symbolic link: the rest is of what it points to */
	pos_t size;
	char mode[12];		/* as ls has it: drwxr-xr-x */
	char owner[40];		/* user:group */
	char modified[32];	/* 2026-11-12 10:30:00, local time */
	char target[1024];	/* of a link */
};
int file_info(const char *path, struct file_info *fi);	/* 0 if none */
/* what is in a folder, all the way down (links not followed): 0 when it
 * stopped after ms milliseconds, the counts so far */
int dir_usage(const char *path, pos_t *files, pos_t *dirs, pos_t *bytes, int ms);
extern const char path_sep;		/* '/' or '\\' */
/* the drives (bit 0 is A:), when the root has a parent: the list of them */
unsigned plat_drives(void);

extern const char plat_encoding[];	/* "UTF-8" */
/* n in decimal at s: its length */
int udec(char *s, pos_t n);


/* threads: the search of a project's text, on each core.  A lock is
 * also where threads wait for each other (a monitor). */
struct plat_thread;
struct plat_lock;
int plat_cpus(void);		/* the cores, at least 1 */
long plat_ms(void);		/* a clock in milliseconds, of the wall, steady */
/* fn(arg) on a thread of its own: 0 if it cannot be made */
struct plat_thread *plat_thread_start(void (*fn)(void *), void *arg);
void plat_thread_join(struct plat_thread *t);	/* its end waited for */
struct plat_lock *plat_lock_new(void);	/* 0 without memory */
void plat_lock_free(struct plat_lock *l);
void plat_lock(struct plat_lock *l);
void plat_unlock(struct plat_lock *l);
/* the lock held: let go until plat_wake, or ms milliseconds (ms < 0: no
 * limit), and held again */
void plat_wait(struct plat_lock *l, int ms);
void plat_wake(struct plat_lock *l);	/* each thread waiting */

/* the name of the program: cedit (a terminal) or xcedit (a window) */
extern const char plat_name[];

/* a message out of the editor (--help, an error before the screen) */
void plat_print(const char *s);

#endif
