/* INI */
#include "../lang_def.h"

LANGUAGE(ini) {
	.name = "ini",
	.title = "INI",
	.kind = K_KEYVAL,
	.extensions = "conf cfg cnf inf ini reg",
	.filenames = ".editorconfig .gitconfig .gitmodules .gitattributes",
	.paths = ".git/config .config/git/config",

	.keywords = WORDS(
		"false", "no", "off", "on", "true", "yes"),

	.column_comments = { COLUMN(1, ";#") },
	.quotes = "'\"",
};
