/* DOT */
#include "../lang_def.h"

LANGUAGE(dot) {
	.name = "dot",
	.title = "DOT",
	.extensions = "dot",

	.keywords = WORDS(
		"arrowhead", "arrowsize", "arrowtail", "color", "comment", "constraint",
		"decorate", "digraph", "dir", "edge", "fontname", "fontsize", "graph",
		"headURL", "headlabel", "headport", "label", "labelangle",
		"labeldistance", "labelfloat", "labelfontcolor", "labelfontname",
		"labelfontsize", "layer", "lhead", "ltail", "minlen", "node", "rankdir",
		"samehead", "sametail", "shape", "style", "subgraph", "tailURL",
		"taillabel", "tailport", "weight"),

	.types = WORDS(
		"Mcircle", "Mdiamond", "Msquare", "box", "box3d", "circle", "diamond",
		"doublecircle", "doubleoctagon", "egg", "ellipse", "folder", "hexagon",
		"house", "invhouse", "invtrapezium", "invtriangle", "none", "note",
		"octagon", "parallelogram", "pentagon", "plaintext", "point", "polygon",
		"record", "rect", "rectangle", "septagon", "tab", "trapezium",
		"triangle", "tripleoctagon"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
