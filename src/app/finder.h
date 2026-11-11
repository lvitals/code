/* The parts of the project finder (finder.c): the files of a tree
 * (walk.c), a fuzzy match of their names (fuzzy.c), and a search of
 * their text (grep.c).  None draws: finder.c does. */
#ifndef FINDER_H
#define FINDER_H

#include "app.h"

/* --- walk.c: the files under a folder, a step at a time ------------------
 *
 * Depth first, each folder in the order of its names (as bytes).  Left
 * out: the hidden names (.git, .cache, .x), what .gitignore and .ignore
 * files (and .git/info/exclude) leave out, and links to folders (a walk
 * never loops).  The paths are relative to the root. */
#define WALK_MAX	1000000		/* the most files kept */

void walk_start(const char *root);	/* root: ended by a '/' */
int walk_step(int budget);		/* up to budget entries: 1 if more */
int walk_done(void);
void walk_stop(void);			/* all of it freed */
int walk_count(void);
const char *walk_path(int i);
const char *walk_root(void);

/* the rules of a .gitignore, for walk.c, and alone for a test: a glob of
 * git (* ? [..] **) over a path, 1 if it matches */
int glob_match(const char *pat, const char *s);

/* --- fuzzy.c: a query matched against a path ---------------------------
 *
 * The query: words apart by blanks, each matched in turn (all of them
 * must be); the letters of a word in that order, not one next to the
 * other; any case unless the word has a capital (smart case).  A match
 * scores more for letters together, at the start of a word or of a name,
 * and in the name more than in its folders. */
int fz_set(const char *query);		/* its words: 0, everything matches */
int fz_score(const char *s, int n);	/* -1: no match */
/* the bytes of s a match takes: mark[i] = 1 (n bytes of mark) */
void fz_marks(const char *s, int n, unsigned char *mark);

/* --- scan.c: the lines of the files that match, on threads ---------------
 *
 * A pattern is a regular expression of the editor's (regex.h), or a
 * text when it has none of their signs; in any case unless it has a
 * capital.  The files are searched as the editor would open them (their
 * encoding guessed), on a thread for each core, a block at a time; a
 * binary one is left when a zero byte is seen (not UTF-16), as grep does.  All
 * of these are called from the editor's thread. */
int scan_start(const char *pat, const char **err);	/* 0: none (*err: why) */
void scan_cancel(void);		/* the search dropped: no pattern */
int scan_feed(int file, const char *path);	/* a file to search: 0 if
						   too many wait already */
/* the lines found (to hit), waiting for some up to ms if none yet: 1 if
 * there were */
int scan_take(void (*hit)(int file, pos_t line, pos_t col), int ms);
int scan_busy(void);		/* files wait, are searched, or found lines */
void scan_stop(void);		/* the threads ended */

#endif
