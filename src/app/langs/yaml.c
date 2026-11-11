/* YAML */
#include "../lang_def.h"

LANGUAGE(yaml) {
	.name = "yaml",
	.title = "YAML",
	.kind = K_KEYVAL,
	.extensions = "yaml yml",

	.constants = WORDS(
		"false", "null", "true"),

	.line_comments = { "#" },
	.quotes = "'\"",
};
