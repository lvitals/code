/* Forth */
#include "../lang_def.h"

LANGUAGE(forth) {
	.name = "forth",
	.title = "Forth",
	.flags = F_NOCASE,
	.extensions = "fth forth frt",

	.keywords = WORDS(
		"#>", "#s", "*" "/", "*" "/mod", "+loop", ",", ".", ".r", "/mod", "0<", "0<>",
		"0=", "0>", "1+", "1-", "2!", "2*", "2/", "2>r", "2@", "2drop", "2dup",
		"2over", "2r>", "2r@", "2swap", ":noname", "<#", "<>", ">body", ">in",
		">number", ">r", "?do", "?dup", "@", "[']", "[char]", "[compile]",
		"abort", "abs", "accept", "action-of", "again", "align", "aligned",
		"allot", "and", "base", "begin", "bl", "buffer:", "c!", "c,", "c@",
		"case", "cell+", "cells", "char", "char+", "chars", "compile,",
		"constant,", "count", "cr", "create", "decimal", "defer", "defer!",
		"defer@", "depth", "do", "does>", "drop", "dup", "else", "emit",
		"endcase", "endof", "environment?", "erase", "evaluate", "execute",
		"exit", "false", "fill", "find", "fm/mod", "here", "hex", "hold",
		"holds", "i", "if", "immediate", "invert", "is", "j", "key", "leave",
		"literal", "loop", "lshift", "m*", "marker", "max", "min", "mod",
		"move", "negate", "nip", "of", "or", "over", "pad", "parse",
		"parse-name", "pick", "postpone", "quit", "r>", "r@", "recurse",
		"refill", "restore-input", "roll", "rot", "rshift", "s>d", "save-input",
		"sign", "sm/rem", "source", "source-id", "space", "spaces", "state",
		"swap", "then", "to", "true", "tuck", "type", "u.", "u.r", "u<", "u>",
		"um*", "um/mod", "unloop", "until", "unused", "value", "variable",
		"while", "within", "word", "xor"),

	.line_comments = { "|\\" },
	.block_comments = { { "(", ")" } },
	.quotes = "\"",
};
