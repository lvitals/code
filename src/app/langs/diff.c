/* Diff */
#include "../lang_def.h"

LANGUAGE(diff) {
	.name = "diff",
	.title = "Diff",
	.kind = K_DIFF,
	.extensions = "rej diff patch",

	.line_comments = { "Index: ", "---", "****", "=" },
};
