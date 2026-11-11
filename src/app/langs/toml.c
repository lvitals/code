/* TOML */
#include "../lang_def.h"

LANGUAGE(toml) {
	.name = "toml",
	.title = "TOML",
	.kind = K_KEYVAL,
	.extensions = "toml",
	.filenames = "Cargo.lock",

	.keywords = WORDS(
		"false", "true"),

	.line_comments = { "#" },
	.quotes = "'\"",
};
