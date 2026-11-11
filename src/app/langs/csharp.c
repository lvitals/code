/* C# */
#include "../lang_def.h"

LANGUAGE(csharp) {
	.name = "csharp",
	.title = "C#",
	.flags = F_HASHPRE | F_ATTR,
	.extensions = "cs",

	.keywords = WORDS(
		"abstract", "add", "as", "assembly", "base", "break", "case", "catch",
		"checked", "class", "const", "continue", "default", "delegate", "do",
		"else", "enum", "event", "explicit", "extern", "false", "finally",
		"fixed", "for", "foreach", "get", "goto", "if", "implicit", "in",
		"interface", "internal", "is", "lock", "namespace", "new", "null",
		"out", "override", "params", "partial", "private", "protected",
		"public", "readonly", "ref", "remove", "return", "sealed", "set",
		"sizeof", "stackalloc", "static", "struct", "super", "switch", "this",
		"throw", "true", "try", "typeof", "unchecked", "unsafe", "using",
		"value", "var", "virtual", "void", "volatile", "while", "yield"),

	.types = WORDS(
		"bool", "byte", "char", "decimal", "double", "float", "int", "long",
		"object", "operator", "sbyte", "short", "string", "uint", "ulong",
		"ushort"),

	.preproc = WORDS(
		"define", "elif", "else", "endif", "endregion", "error", "if", "line",
		"region", "undef", "warning"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		PRE_BLOCK("if", "endif"),
		PRE_BLOCK("ifdef", "endif"),
		PRE_BLOCK("ifndef", "endif"),
		PRE_BLOCK("region", "endregion"),
		OP_BLOCK("{", "}")),
};
