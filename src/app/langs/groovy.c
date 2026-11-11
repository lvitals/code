/* Groovy */
#include "../lang_def.h"

LANGUAGE(groovy) {
	.name = "groovy",
	.title = "Groovy",
	.flags = F_ATTR,
	.extensions = "groovy gvy",
	.filenames = "Jenkinsfile",

	.keywords = WORDS(
		"abstract", "as", "assert", "break", "case", "catch", "continue", "def",
		"default", "do", "else", "extends", "false", "final", "finally", "for",
		"if", "implements", "import", "in", "instanceof", "it", "mixin",
		"native", "new", "null", "package", "private", "property", "protected",
		"public", "return", "static", "strictfp", "super", "switch",
		"synchronized", "test", "this", "throw", "throws", "transient", "true",
		"try", "using", "volatile", "while"),

	.types = WORDS(
		"boolean", "byte", "char", "class", "double", "float", "int",
		"interface", "long", "short", "void"),

	.functions = WORDS(
		"abs", "any", "append", "asList", "asWritable", "call", "collect",
		"compareTo", "count", "div", "dump", "each", "eachByte", "eachFile",
		"eachLine", "every", "find", "findAll", "flatten", "getAt", "getErr",
		"getIn", "getOut", "getText", "grep", "immutable", "inject", "inspect",
		"intersect", "invokeMethods", "isCase", "join", "leftShift", "minus",
		"multiply", "newInputStream", "newOutputStream", "newPrintWriter",
		"newReader", "newWriter", "next", "plus", "pop", "power", "previous",
		"print", "println", "push", "putAt", "read", "readBytes", "readLines",
		"reverse", "reverseEach", "round", "size", "sort", "splitEachLine",
		"step", "subMap", "times", "toInteger", "toList", "tokenize", "upto",
		"waitForOrKill", "withPrintWriter", "withReader", "withStream",
		"withWriter", "withWriterAppend", "write", "writeLine"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.long_strings = { { "'''", "'''" }, { "\"\"\"", "\"\"\"" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
