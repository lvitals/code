/* Pascal */
#include "../lang_def.h"

LANGUAGE(pascal) {
	.name = "pascal",
	.title = "Pascal",
	.flags = F_NOCASE,
	.extensions = "dpk dpr p pas pp",

	.keywords = WORDS(
		"absolute", "abstract", "and", "array", "as", "asm", "assembler", "at",
		"automated", "begin", "case", "cdecl", "class", "const", "constructor",
		"contains", "default", "deprecated", "destructor", "dispid",
		"dispinterface", "div", "do", "downto", "dynamic", "else", "end",
		"except", "export", "exports", "external", "false", "far", "file",
		"final", "finalization", "finally", "for", "forward", "function",
		"goto", "if", "implementation", "implements", "in", "index",
		"inherited", "initialization", "inline", "interface", "is", "label",
		"library", "local", "message", "mod", "name", "namespaces", "near",
		"nil", "nodefault", "not", "object", "of", "on", "or", "out",
		"overload", "override", "package", "packed", "pascal", "platform",
		"private", "procedure", "program", "property", "protected", "public",
		"published", "raise", "read", "readonly", "record", "register",
		"reintroduce", "repeat", "requires", "resident", "resourcestring",
		"safecall", "sealed", "self", "set", "shl", "shr", "static", "stdcall",
		"stored", "string", "then", "threadvar", "to", "true", "try", "type",
		"unit", "unsafe", "until", "uses", "var", "varargs", "virtual", "while",
		"with", "write", "writeonly", "xor"),

	.types = WORDS(
		"boolean", "byte", "bytebool", "cardinal", "char", "comp", "currency",
		"double", "extended", "integer", "longbool", "longint", "pointer",
		"real", "shortint", "single", "smallint", "word", "wordbool"),

	.functions = WORDS(
		"abs", "arctan", "chr", "cos", "eof", "eoln", "exp", "ln", "odd", "ord",
		"pred", "round", "sin", "sqr", "sqrt", "succ", "trunc"),

	.line_comments = { "//" },
	.block_comments = { { "{", "}" }, { "(*", "*)" } },
	.quotes = "'",
};
