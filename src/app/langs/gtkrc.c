/* Gtkrc */
#include "../lang_def.h"

LANGUAGE(gtkrc) {
	.name = "gtkrc",
	.title = "Gtkrc",
	.kind = K_KEYVAL,
	.extensions = "gtkrc",

	.keywords = WORDS(
		"binding", "class", "im_module_file", "include", "module_path",
		"pixmap_path", "style", "widget", "widget_class"),

	.constants = WORDS(
		"ACTIVE", "FALSE", "INSENSITIVE", "NORMAL", "PRELIGHT", "SELECTED",
		"TRUE"),

	.functions = WORDS(
		"base", "bg", "bg_pixmap", "color", "darker", "engine", "fg", "font",
		"font_name", "fontset", "lighter", "mix", "shade", "stock", "text",
		"xthickness", "ythickness"),

	.line_comments = { "#" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
