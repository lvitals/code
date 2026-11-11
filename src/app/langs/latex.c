/* LaTeX */
#include "../lang_def.h"

LANGUAGE(latex) {
	.name = "latex",
	.title = "LaTeX",
	.extensions = "bbl dtx ins ltx tex sty",

	.line_comments = { "%" },
	.block_comments = { { "\\begin", " " } },

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
