/* JSON */
#include "../lang_def.h"

LANGUAGE(json) {
	.name = "json",
	.title = "JSON",
	.extensions = "json jsonc json5",

	.keywords = WORDS(
		"false", "null", "true"),

	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
