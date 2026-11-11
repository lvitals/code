/* TeX */
#include "../lang_def.h"

LANGUAGE(tex) {
	.name = "tex",
	.title = "TeX",

	.line_comments = { "%" },

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
