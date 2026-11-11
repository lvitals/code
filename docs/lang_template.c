/* A model of a language: copy it to src/app/langs/NAME.c, fill it in,
 * and add LANG(NAME) to src/app/langs/all.h (in the order of the names).
 * Then: make, and  build/cedit --lex some.file  to see what it does.
 *
 * Every field is optional but .name and .title: leave out what the
 * language has not.  The guide: docs/LANGUAGES.md.  (Here, in docs/, the
 * include below is not found: it is, once the file is in langs/.) */
#include "../lang_def.h"

LANGUAGE(foo) {
	/* its name (lowercase, unique; - becomes _ in LANGUAGE() and in the
	   name of the file), and what the status bar shows */
	.name = "foo",
	.title = "Foo",

	/* how its code is: joined with | (syntax.h has them all)
	     F_NOCASE    the words in any case (IF, If, if)
	     F_DASH      - is in names (WORKING-STORAGE, let-values)
	     F_HASHPRE   # at the start of a line is the preprocessor (C)
	     F_VARS      $name and ${name} are variables (shells)
	     F_ATTR      @name and #[...] in the preprocessor's colour
	     F_BACKTICK  `...` is a string of many lines
	     F_LUALONG   [[ ]] strings and --[[ ]] comments (Lua)
	     F_NOQ1      ' is no quote (Rust's lifetimes) */
	.flags = F_NOCASE,

	/* how a file is known for it: words apart by spaces */
	.extensions = "foo fooh",		/* x.foo, x.fooh (in any case) */
	.filenames = "Foofile",			/* the whole name of the file */
	.paths = ".config/foo/config",		/* how its path ends */
	.interpreters = "foo foorun",		/* #!/usr/bin/env foo */

	/* the words, in colours of their own (any order) */
	.keywords = WORDS("if", "then", "else", "end", "while", "do", "func", "return"),
	.types = WORDS("int", "real", "string", "bool"),
	.constants = WORDS("true", "false", "nil"),
	.functions = WORDS("print", "len"),
	.preproc = WORDS("#use"),

	/* comments: to the end of the line (up to 4); by a character in a
	   column, for the whole line (up to 3: COLUMN(7, "*") a * in column
	   7); of blocks, what opens and what closes (up to 3) */
	.line_comments = { "//", "--" },
	.column_comments = { COLUMN(1, "*") },
	.block_comments = { { "(*", "*)" } },

	/* strings: the characters that quote one (on one line), and those
	   of many lines, what opens and what closes (up to 4) */
	.quotes = "'\"",
	.long_strings = { { "[[", "]]" } },

	/* where blocks start and end: Highlight Matching Brackets and Go to
	   Matching Bracket show and go from one to the other.  ( ) [ ] { }
	   are there for every language */
	.blocks = BLOCKS(
		WORD_BLOCK("func", "end"),	/* keywords */
		WORD_BLOCK("if", "end"),
		OP_BLOCK("begin(", ")end"),	/* operators */
		PRE_BLOCK("#if", "#endif")),	/* the preprocessor's */
};
