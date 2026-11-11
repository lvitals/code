/* XS */
#include "../lang_def.h"

LANGUAGE(xs) {
	.name = "xs",
	.title = "XS",
	.extensions = "xs xsin xsrc",

	.keywords = WORDS(
		".", "...", ":eq", ":ge", ":gt", ":le", ":lt", ":ne", "access", "alias",
		"catch", "cd", "dirs", "echo", "else", "escape", "eval", "exec", "exit",
		"false", "fn", "fn-", "for", "forever", "fork", "history", "if", "jobs",
		"let", "limit", "local", "map", "omap", "popd", "printf", "pushd",
		"read", "result", "set-", "switch", "throw", "time", "true", "umask",
		"until", "unwind-protect", "var", "vars", "wait", "whats", "while", "~",
		"~~"),

	.line_comments = { "#" },
	.quotes = "'",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
