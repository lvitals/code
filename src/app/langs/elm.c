/* Elm */
#include "../lang_def.h"

LANGUAGE(elm) {
	.name = "elm",
	.title = "Elm",
	.extensions = "elm",

	.keywords = WORDS(
		"alias", "as", "case", "else", "exposing", "if", "import", "in", "let",
		"module", "of", "port", "then", "type"),

	.line_comments = { "--" },
	.block_comments = { { "{-", "-}" } },
	.quotes = "\"'",
};
