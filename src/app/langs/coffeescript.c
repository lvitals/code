/* CoffeeScript */
#include "../lang_def.h"

LANGUAGE(coffeescript) {
	.name = "coffeescript",
	.title = "CoffeeScript",
	.flags = F_BACKTICK,
	.extensions = "coffee",

	.keywords = WORDS(
		"all", "and", "bind", "break", "by", "case", "catch", "class", "const",
		"continue", "default", "delete", "do", "each", "else", "enum", "export",
		"extends", "false", "finally", "for", "function", "if", "import", "in",
		"instanceof", "is", "isnt", "let", "loop", "native", "new", "no", "not",
		"of", "off", "on", "or", "return", "super", "switch", "then", "this",
		"throw", "true", "try", "typeof", "unless", "until", "var", "void",
		"when", "while", "with", "yes"),

	.line_comments = { "#" },
	.quotes = "'\"",
};
