/* R */
#include "../lang_def.h"

LANGUAGE(r) {
	.name = "r",
	.title = "R",
	.extensions = "R Rout Rhistory Rt",
	.filenames = "Rout.save Rout.fail",
	.interpreters = "Rscript",

	.keywords = WORDS(
		"%%", "%*%", "%/%", "%in%", "%o%", "%x%", "F", "FALSE", "Inf", "NA",
		"NULL", "NaN", "T", "TRUE", "break", "else", "for", "if", "in", "next",
		"repeat", "return", "switch", "try", "while", "|>"),

	.types = WORDS(
		"array", "character", "closure", "complex", "data.frame", "double",
		"environment", "expression", "externalptr", "factor", "function",
		"integer", "list", "logical", "matrix", "numeric", "pairlist",
		"promise", "raw", "symbol", "vector"),

	.line_comments = { "#" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
