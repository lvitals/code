/* Julia */
#include "../lang_def.h"

LANGUAGE(julia) {
	.name = "julia",
	.title = "Julia",
	.flags = F_BACKTICK | F_ATTR,
	.extensions = "jl",
	.interpreters = "julia",

	.keywords = WORDS(
		"baremodule", "begin", "break", "catch", "const", "continue", "do",
		"else", "elseif", "end", "export", "finally", "for", "function",
		"global", "if", "import", "in", "isa", "let", "local", "macro",
		"module", "quote", "return", "struct", "try", "using", "where", "while"),

	.line_comments = { "#" },
	.block_comments = { { "#=", "=#" } },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "\"",
};
