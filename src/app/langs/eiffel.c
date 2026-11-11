/* Eiffel */
#include "../lang_def.h"

LANGUAGE(eiffel) {
	.name = "eiffel",
	.title = "Eiffel",
	.extensions = "e eif",

	.keywords = WORDS(
		"alias", "all", "and", "as", "check", "class", "creation", "current",
		"debug", "deferred", "do", "else", "elseif", "end", "ensure",
		"expanded", "export", "external", "false", "feature", "from", "frozen",
		"if", "implies", "indexing", "infix", "inherit", "inspect", "invariant",
		"is", "like", "local", "loop", "not", "obsolete", "old", "once", "or",
		"precursor", "prefix", "redefine", "rename", "require", "rescue",
		"result", "retry", "select", "separate", "strip", "then", "true",
		"undefine", "unique", "until", "variant", "void", "when", "xor"),

	.types = WORDS(
		"any", "bit", "boolean", "character", "integer", "none", "real",
		"string"),

	.line_comments = { "--" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("check", "end"),
		WORD_BLOCK("debug", "end"),
		WORD_BLOCK("do", "end"),
		WORD_BLOCK("from", "end"),
		WORD_BLOCK("if", "end"),
		WORD_BLOCK("inspect", "end"),
		WORD_BLOCK("once", "end")),
};
