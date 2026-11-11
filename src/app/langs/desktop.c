/* Desktop */
#include "../lang_def.h"

LANGUAGE(desktop) {
	.name = "desktop",
	.title = "Desktop",
	.kind = K_KEYVAL,
	.extensions = "desktop",

	.constants = WORDS(
		"false", "true"),

	.functions = WORDS(
		"Categories", "Comment", "Exec", "GenericName", "Hidden", "Icon",
		"MimeType", "Name", "NoDisplay", "NotShowIn", "OnlyShowIn", "Path",
		"StartupNotify", "StartupWMClass", "Terminal", "TryExec", "Type", "URL",
		"Version"),

	.line_comments = { "#" },
	.quotes = "\"",
};
