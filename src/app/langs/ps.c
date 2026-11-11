/* PostScript */
#include "../lang_def.h"

LANGUAGE(ps) {
	.name = "ps",
	.title = "PostScript",
	.extensions = "eps ps",

	.keywords = WORDS(
		"NULL", "clear", "cleartomark", "copy", "count", "countexecstack",
		"counttomark", "dup", "exch", "exec", "execstack", "exit", "false",
		"for", "if", "ifelse", "loop", "mark", "pop", "quit", "repeat", "roll",
		"start", "stop", "stopped", "true"),

	.functions = WORDS(
		"abs", "add", "atan", "ceiling", "cos", "div", "exp", "floor", "idiv",
		"ln", "log", "mod", "mul", "ned", "rand", "round", "rrand", "sin",
		"sqrt", "srand", "sub", "truncate"),

	.line_comments = { "%" },
};
