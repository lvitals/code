/* Tcl */
#include "../lang_def.h"

LANGUAGE(tcl) {
	.name = "tcl",
	.title = "Tcl",
	.flags = F_VARS,
	.extensions = "tcl tk",
	.interpreters = "tclsh jimsh wish",

	.line_comments = { "#", "^[ \t]", "^[r\n;]" },

	.blocks = BLOCKS(
		WORD_BLOCK("{", "}")),
};
