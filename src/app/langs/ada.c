/* Ada */
#include "../lang_def.h"

LANGUAGE(ada) {
	.name = "ada",
	.title = "Ada",
	.flags = F_NOCASE,
	.extensions = "adb ads",

	.keywords = WORDS(
		"abort", "abs", "abstract", "accept", "access", "aliased", "all", "and",
		"array", "at", "begin", "body", "case", "constant", "declare", "delay",
		"delta", "digits", "do", "else", "elsif", "end", "entry", "exception",
		"exit", "false", "for", "function", "generic", "goto", "if", "in",
		"interface", "is", "limited", "loop", "mod", "new", "not", "null", "of",
		"or", "others", "out", "overriding", "package", "parallel", "pragma",
		"private", "procedure", "protected", "raise", "range", "record", "rem",
		"renames", "requeue", "return", "reverse", "select", "separate", "some",
		"subtype", "synchronized", "tagged", "task", "terminate", "then",
		"true", "type", "until", "use", "when", "while", "with", "xor"),

	.types = WORDS(
		"boolean", "character", "count", "duration", "float", "integer",
		"long_float", "long_integer", "priority", "short_float",
		"short_integer", "string"),

	.line_comments = { "--" },
	.quotes = "\"",
};
