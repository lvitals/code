/* The languages the editor highlights.  Each one is a file of its own,
 * src/app/langs/NAME.c, that fills a struct syn (with the macros of
 * lang_def.h); src/app/langs/all.h lists them; one lexer of lines in C
 * (syntax.c) reads them.  Language 0 is plain text.
 *
 * How to write one: docs/LANGUAGES.md, and docs/lang_template.c. */
#ifndef SYNTAX_H
#define SYNTAX_H

/* how a language is lexed: K_CODE (0, what a field left out is) by the
 * table; the others by lexers of their own in syntax.c */
enum { K_CODE, K_PLAIN, K_DIFF, K_MD, K_XML, K_KEYVAL };

/* what its code has (.flags, joined with |) */
#define F_HASHPRE	0x01	/* # at the start of a line: the preprocessor (C) */
#define F_LUALONG	0x02	/* [[ ]] strings, --[[ ]] comments (Lua) */
#define F_BACKTICK	0x04	/* `` strings of many lines (Go, JavaScript) */
#define F_NOCASE	0x08	/* words in any case (COBOL, Ada, SQL) */
#define F_VARS		0x10	/* $name and ${name} (shells, Perl, PHP) */
#define F_ATTR		0x20	/* @name, #[...]: the preprocessor's colour */
#define F_NOQ1		0x40	/* ' is no quote (Rust lifetimes...) */
#define F_DASH		0x80	/* - is in names: WORKING-STORAGE, let-values */

/* a comment by a character in a column: Fortran's C in column 1 */
struct column_comment {
	int column;		/* from 1; 0: none */
	const char *chars;	/* any one of them */
};

struct syn {
	const char *name;	/* its name: "c", "python" (lowercase) */
	const char *title;	/* as the status bar shows it */
	unsigned char kind, flags;

	/* the words, each list ended by 0 (WORDS(...) ends it): keywords,
	   types, constants, functions (and variables) built in, the
	   preprocessor's */
	const char *const *keywords, *const *types, *const *constants;
	const char *const *functions, *const *preproc;

	const char *line_comments[5];		/* to the end of the line: "//" */
	struct column_comment column_comments[3];
	const char *block_comments[4][2];	/* what opens, what closes */
	const char *long_strings[5][2];		/* of many lines: """ """ */
	const char *quotes;			/* each quotes a string: "'\"" */
	/* where a block starts and ends: triples of a kind ("k" a keyword,
	   "o" an operator, "p" the preprocessor's), what opens and what
	   closes, ended by 0 (BLOCKS(WORD_BLOCK("if", "end"), ...)) */
	const char *const *blocks;

	/* how a file is known for it: words apart by spaces */
	const char *extensions;		/* "c h": x.c, x.h (any case) */
	const char *filenames;		/* "Makefile GNUmakefile": the whole name */
	const char *paths;		/* ".git/config": how the path ends */
	const char *interpreters;	/* "python python3": of a #! line */
};

/* the languages: syns[0] is plain text */
extern const struct syn *const syns[];
extern const int nsyns;

#endif
