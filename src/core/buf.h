/* buf: the text of a document, a piece table, with the undo and redo of
 * what was done to it.
 *
 * The text is a sequence of pieces, each a reference to bytes that never
 * change: the file as it was read (or mapped, for a large one: nothing is
 * copied), and an append-only buffer with what was typed or pasted.  The
 * pieces are the nodes of a balanced tree (a treap) by their position, so
 * finding a byte, an edit and an undo are O(log n) whatever the size of
 * the text.  An undo keeps the pieces it took out, not the bytes.
 *
 * Each node also counts the newlines of its piece and of its subtree,
 * and the bytes of the subtree not counted yet.  They are counted lazily
 * (eight bytes at a time), when a line number is asked for, and then
 * kept, so a large file opens without being read through; the editor
 * counts the rest when it is idle, and asks with a budget (the _try
 * functions) so that drawing never waits for a count.
 *
 * The newlines in the text are LF: crlf says what a save writes.  The
 * characters are UTF-8, 1 to 4 bytes (buf_next and buf_prev step over
 * them): a file in another encoding (enc.h) is decoded when it is read,
 * and encoded again when it is written; enc says in which.
 *
 * The functions work on one text, B: the one of the active document. */
#ifndef BUF_H
#define BUF_H

#include "../platform/platform.h"
#include "enc.h"

/* what an edit was: typed characters are merged into one undo */
enum { U_TYPE = 1, U_OTHER };

struct piece;
struct pblock;

struct undo {
	struct undo *next;	/* the one before */
	pos_t off, ndel, nins;	/* at off, ndel bytes became nins */
	pos_t cur;		/* the cursor before */
	struct piece *other;	/* the bytes not in the text now: the ndel
				   taken (on the undo list) or the nins
				   (on the redo list) */
	unsigned seq;		/* the edits of one command */
	char kind;
};

struct buf {
	struct piece *root;	/* the pieces */
	struct pblock *blocks;	/* the bytes they refer to */
	pos_t len;
	const char *cp;		/* the piece of the last byte read: its */
	pos_t cs, cl;		/* bytes, where it starts, its length */
	pos_t ls_lo, ls_hi;	/* a line start, and how far no newline
				   follows it (ls_hi POS_NONE: none known) */
	pos_t ln_off, ln_no;	/* a known line: its start, its number */
	long dlines;		/* the newlines the last edit added; DL_MANY:
				   too many to count */
	struct undo *undo, *redo, *saved;	/* saved: the top at the save, or
					   the buf when no undo reaches it */
	unsigned seq;
	char merge;		/* the next typed character may merge */
	char crlf, tabs, tabsize;	/* tabs: indent with them */
	char detected;		/* tabs, tabsize: as the file is indented */
	char mapped;		/* the file is mapped, not copied */
	char enc;		/* the encoding of the file (ENC_UTF8 ...) */
	char nobom;		/* UTF-16 read without its BOM: written so */
	pos_t stamp;		/* the file as it was loaded or saved */
};

extern struct buf *B;

#define DL_MANY		(-2147483647L - 1)

/* the results of buf_load and buf_save */
enum { B_OK, B_NOFILE, B_NOMEM, B_WRITE, B_ENCODE };
/* B_ENCODE: the character at buf_bad is not in the encoding (or is not
 * UTF-8); nothing was written */
extern pos_t buf_bad;

#define buf_len()	(B->len)
/* the byte at i (i < buf_len()): from the piece read last, or looked for */
#define BCH(i)		((pos_t)(i) - B->cs < B->cl ? \
			    (unsigned char)B->cp[(pos_t)(i) - B->cs] : buf_at(i))
int buf_at(pos_t i);
#define buf_dirty(b)	((b)->undo != (b)->saved)

void buf_init(void);
void buf_free(void);
/* the file in enc, or ENC_AUTO: guessed (enc.h) */
int buf_load(const char *name, int enc);
/* the file to be looked at, not edited (a preview): as buf_load (its
 * encoding guessed), but a large one in UTF-8 is mapped even with CR LF
 * newlines (kept: the lines are the same), and with no snapshot */
int buf_load_search(const char *name);
int buf_save(const char *name);

/* the next command starts: its edits are undone together */
void buf_begin(void);
/* changed without an undo (the newlines): dirty until saved */
void buf_touch(void);
/* ndel bytes at off become the n of s; cur is the cursor before.
 * 0 without memory */
int buf_replace(pos_t off, pos_t ndel, const char *s, pos_t n, pos_t cur, int kind);
/* undo (or redo) the last command: the cursor after it, or POS_NONE */
pos_t buf_undo(int redo);
void buf_copy(pos_t off, pos_t n, char *dst);

pos_t buf_next(pos_t off);	/* the next character */
pos_t buf_prev(pos_t off);
rune buf_char(pos_t off);	/* the code point at off */
pos_t buf_word(pos_t off, int dir);	/* Ctrl+Left/Right */

pos_t buf_linestart(pos_t off);
pos_t buf_lineend(pos_t off);	/* at its newline */
/* the newlines of [a, b), up to max: what is read is only that range */
pos_t buf_count(pos_t a, pos_t b, pos_t max);
/* The line numbers: counted (once) from the start of the text */
pos_t buf_lineno(pos_t off);	/* 0-based */
pos_t buf_lineoff(pos_t line);	/* the start of a line */
pos_t buf_lines(void);		/* the newlines, plus one: all counted */
int buf_lines_known(void);	/* buf_lines would read nothing */
pos_t buf_counted(void);	/* the bytes whose newlines are counted */
/* count up to budget bytes not counted yet: 1 if some remain */
int buf_count_step(pos_t budget);
/* the line of off, or the newlines of [a, b), if no more than budget
 * bytes not counted yet are to be counted: 0 if more would be */
int buf_lineno_try(pos_t off, pos_t budget, pos_t *n);
int buf_count_try(pos_t a, pos_t b, pos_t budget, pos_t *n);

/* the characters (UTF-8) of [a, b) */
pos_t buf_chars(pos_t a, pos_t b);
/* the bytes from a (before b) that are printable ASCII: a column each */
pos_t buf_plain(pos_t a, pos_t b);

enum { FIND_CASE = 1, FIND_WORD = 2 };
/* the first match of s from off on, past the end from the start; 0 if
 * none */
int buf_find(pos_t off, const char *s, int flags, pos_t *at);
/* the first match of s that starts in [from, to): 0 if none */
int buf_find_range(pos_t from, pos_t to, const char *s, int flags, pos_t *at);
int buf_match(pos_t off, const char *s, int flags);

int isword(int c);

#endif
