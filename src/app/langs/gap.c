/* GAP */
#include "../lang_def.h"

LANGUAGE(gap) {
	.name = "gap",
	.title = "GAP",
	.extensions = "gd gi gap",

	.keywords = WORDS(
		"and", "break", "continue", "do", "elif", "else", "end", "fail",
		"false", "fi", "for", "function", "if", "in", "infinity", "local",
		"not", "od", "or", "rec", "repeat", "return", "then", "true", "until",
		"while"),

	.line_comments = { "#" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("function", "end"),
		WORD_BLOCK("do", "od"),
		WORD_BLOCK("if", "fi"),
		WORD_BLOCK("repeat", "until")),
};
