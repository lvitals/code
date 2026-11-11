/* Icon */
#include "../lang_def.h"

LANGUAGE(icon) {
	.name = "icon",
	.title = "Icon",
	.extensions = "icn",

	.keywords = WORDS(
		"allocated", "ascii", "break", "by", "case", "clock", "collections",
		"create", "cset", "current", "date", "dateline", "default", "digits",
		"do", "dump", "e", "else", "end", "error", "errornumber", "errortext",
		"errorvalue", "errout", "every", "fail", "features", "file", "global",
		"host", "if", "initial", "input", "invocable", "lcase", "letters",
		"level", "line", "link", "local", "main", "next", "not", "null", "of",
		"output", "phi", "pi", "pos", "procedure", "progname", "random",
		"record", "regions", "repeat", "return", "source", "static", "storage",
		"subject", "suspend", "then", "time", "to", "trace", "ucase", "until",
		"version", "while"),

	.preproc = WORDS(
		"define", "else", "endif", "error", "ifdef", "ifndef", "include",
		"line", "undef"),

	.line_comments = { "#" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		PRE_BLOCK("ifdef", "endif"),
		PRE_BLOCK("ifndef", "endif"),
		WORD_BLOCK("procedure", "end")),
};
