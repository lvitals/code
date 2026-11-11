/* What a file of a language (src/app/langs/NAME.c) is written with: C99,
 * a struct syn (syntax.h) filled by the names of its fields.
 *
 *   #include "../lang_def.h"
 *
 *   LANGUAGE(foo) {
 *   	.name = "foo", .title = "Foo",
 *   	.extensions = "foo fooc",
 *   	.keywords = WORDS("if", "else", "while", "func", "end"),
 *   	.types = WORDS("int", "string"),
 *   	.line_comments = { "//" },
 *   	.block_comments = { { "/" "*", "*" "/" } },
 *   	.quotes = "'\"",
 *   	.blocks = BLOCKS(WORD_BLOCK("func", "end"), OP_BLOCK("{", "}")),
 *   };
 *
 * and a line LANG(foo) in src/app/langs/all.h.  A field left out has
 * none.  The guide: docs/LANGUAGES.md. */
#ifndef LANG_DEF_H
#define LANG_DEF_H

#include "syntax.h"

/* the definition of language name: lang_name, that all.h lists */
#define LANGUAGE(name)	const struct syn lang_##name =

/* a list of words: WORDS("if", "else") (ended by 0 for you) */
#define WORDS(...)	((const char *const []){ __VA_ARGS__, 0 })

/* a comment by a character in a column: COLUMN(7, "*") is a * in
 * column 7 (COBOL), COLUMN(1, "Cc*") a C, c or * in column 1 (Fortran) */
#define COLUMN(col, chars)	{ (col), (chars) }

/* where blocks start and end, for Highlight Matching Brackets and Go to
 * Matching Bracket: BLOCKS(WORD_BLOCK("if", "fi"), OP_BLOCK("{", "}")) */
#define BLOCKS(...)		((const char *const []){ __VA_ARGS__, 0 })
#define WORD_BLOCK(open, close)	"k", (open), (close)	/* keywords */
#define OP_BLOCK(open, close)	"o", (open), (close)	/* operators */
#define PRE_BLOCK(open, close)	"p", (open), (close)	/* the preprocessor's */

#endif
