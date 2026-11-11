/* Context */
#include "../lang_def.h"

LANGUAGE(context) {
	.name = "context",
	.title = "Context",

	.line_comments = { "%" },

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
