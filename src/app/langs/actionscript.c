/* ActionScript */
#include "../lang_def.h"

LANGUAGE(actionscript) {
	.name = "actionscript",
	.title = "ActionScript",
	.extensions = "as asc",

	.keywords = WORDS(
		"Infinity", "NaN", "Void", "abstract", "break", "case", "catch",
		"class", "const", "continue", "debugger", "default", "delete", "do",
		"dynamic", "else", "export", "extends", "false", "final", "finally",
		"for", "function", "goto", "if", "implements", "import", "in",
		"instanceof", "interface", "native", "new", "null", "on", "package",
		"private", "protected", "public", "return", "static", "super", "switch",
		"synchonized", "this", "throw", "throws", "transient", "true", "try",
		"typeof", "undefined", "var", "void", "volatile", "while", "with"),

	.types = WORDS(
		"Array", "Boolean", "Color", "Date", "Function", "Key", "Math", "Mouse",
		"MovieClip", "Number", "Object", "Selection", "Sound", "String", "XML",
		"XMLNode", "XMLSocket", "boolean", "byte", "char", "double", "enum",
		"float", "int", "long", "short"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
