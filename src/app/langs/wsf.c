/* WSF */
#include "../lang_def.h"

LANGUAGE(wsf) {
	.name = "wsf",
	.title = "WSF",
	.kind = K_XML,
	.extensions = "wsf",

	.constants = WORDS(
		"amp", "apos", "gt", "lt", "quot"),

	.block_comments = { { "<!--", "-->" } },
	.quotes = "'\"",
};
