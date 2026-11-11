/* Scala */
#include "../lang_def.h"

LANGUAGE(scala) {
	.name = "scala",
	.title = "Scala",
	.flags = F_BACKTICK | F_ATTR,
	.extensions = "scala",

	.keywords = WORDS(
		"abstract", "case", "catch", "class", "def", "do", "else", "extends",
		"false", "final", "finally", "for", "forSome", "if", "implicit",
		"import", "lazy", "match", "new", "null", "object", "override",
		"package", "private", "protected", "return", "sealed", "super", "this",
		"throw", "trait", "true", "try", "type", "val", "var", "while", "with",
		"yield"),

	.types = WORDS(
		"Array", "Boolean", "Buffer", "Byte", "Char", "Collection", "Double",
		"Float", "Int", "Iterator", "LinkedList", "List", "Long", "Map", "None",
		"Option", "Set", "Short", "SortedMap", "SortedSet", "String", "TreeMap",
		"TreeSet"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
