/* gettext */
#include "../lang_def.h"

LANGUAGE(gettext) {
	.name = "gettext",
	.title = "gettext",
	.flags = F_NOCASE,
	.extensions = "po pot",

	.keywords = WORDS(
		"c-format", "fuzzy", "msgid", "msgid_plural", "msgstr", "no-c-format"),

	.line_comments = { "#", ": .~" },
	.quotes = "\"",
};
