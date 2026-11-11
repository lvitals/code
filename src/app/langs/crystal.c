/* Crystal */
#include "../lang_def.h"

LANGUAGE(crystal) {
	.name = "crystal",
	.title = "Crystal",
	.flags = F_VARS | F_ATTR,
	.extensions = "cr",

	.keywords = WORDS(
		"__FILE__", "__LINE__", "alias", "begin", "break", "case", "class",
		"def", "defined?", "do", "else", "elsif", "end", "ensure", "false",
		"for", "if", "in", "module", "next", "nil", "not", "redo", "rescue",
		"retry", "return", "self", "super", "then", "true", "undef", "unless",
		"until", "when", "while", "yield"),

	.functions = WORDS(
		"abort", "assert_responds_to", "at_exit", "caller", "debugger", "delay",
		"exit", "fork", "future", "get_stack_top", "gets", "lazy", "loop",
		"main", "p", "parallel", "pp", "print", "printf", "puts", "raise",
		"rand", "read_line", "record", "redefine_main", "require", "sleep",
		"spawn", "sprintf", "system", "with_color"),

	.line_comments = { "#" },
	.quotes = "`'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("begin", "end"),
		WORD_BLOCK("case", "end"),
		WORD_BLOCK("class", "end"),
		WORD_BLOCK("def", "end"),
		WORD_BLOCK("do", "end"),
		WORD_BLOCK("for", "end"),
		WORD_BLOCK("module", "end"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
