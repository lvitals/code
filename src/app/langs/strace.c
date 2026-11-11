/* strace */
#include "../lang_def.h"

LANGUAGE(strace) {
	.name = "strace",
	.title = "strace",

	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",
};
