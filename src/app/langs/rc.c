/* rc */
#include "../lang_def.h"

LANGUAGE(rc) {
	.name = "rc",
	.title = "rc",
	.flags = F_VARS,
	.extensions = "es rc",
	.interpreters = "rc es",

	.keywords = WORDS(
		".", "builtin", "case", "cd", "eval", "exec", "exit", "flag", "fn",
		"for", "if", "in", "not", "rfork", "shift", "switch", "ulimit", "umask",
		"wait", "whatis", "while", "~"),

	.line_comments = { "#" },
	.quotes = "'",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
