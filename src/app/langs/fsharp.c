/* F# */
#include "../lang_def.h"

LANGUAGE(fsharp) {
	.name = "fsharp",
	.title = "F#",
	.flags = F_HASHPRE,
	.extensions = "fs",

	.keywords = WORDS(
		"abstract", "and", "as", "asr", "assert", "async", "atomic", "begin",
		"break", "checked", "class", "component", "const", "constructor",
		"continue", "default", "delegate", "do", "done", "downcast", "downto",
		"eager", "else", "end", "enum", "event", "exception", "external",
		"false", "finaly", "fixed", "for", "fun", "function", "functor", "if",
		"iherit", "in", "include", "inline", "interface", "land", "lazy", "let",
		"lor", "lsl", "lsr", "lxor", "match", "member", "method", "mixin",
		"mod", "module", "mutable", "namespace", "new", "null", "of", "open",
		"or", "override", "process", "property", "protected", "public", "pure",
		"readonly", "return", "sealed", "sig", "static", "struct", "switch",
		"then", "to", "true", "try", "type", "upcast", "val", "virtual", "void",
		"volatile", "when", "where", "while", "with"),

	.types = WORDS(
		"bool", "byte", "char", "decimal", "double", "float", "float32", "int",
		"int16", "int64", "nativeint", "sbyte", "single", "string", "uint16",
		"uint32", "uint64", "unativeint", "unit", "void"),

	.preproc = WORDS(
		"else", "endif", "endregion", "if", "ifdef", "ifndef", "light", "region"),

	.line_comments = { "//" },
	.block_comments = { { "(*", "*)" } },
	.quotes = "'\"",
};
