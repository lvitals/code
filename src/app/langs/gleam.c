/* Gleam */
#include "../lang_def.h"

LANGUAGE(gleam) {
	.name = "gleam",
	.title = "Gleam",
	.extensions = "gleam",

	.keywords = WORDS(
		"big", "binary", "bit_string", "bits", "bytes", "float", "int",
		"little", "native", "signed", "size", "unit", "unsigned", "utf16",
		"utf16_codepoint", "utf32", "utf32_codepoint", "utf8", "utf8_codepoint"),

	.line_comments = { "//" },
	.quotes = "\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("(", ")")),
};
