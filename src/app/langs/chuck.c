/* ChucK */
#include "../lang_def.h"

LANGUAGE(chuck) {
	.name = "chuck",
	.title = "ChucK",
	.extensions = "ck",

	.keywords = WORDS(
		"break", "const", "continue", "else", "for", "fun", "function", "if",
		"new", "repeat", "return", "spork", "switch", "until", "while"),

	.types = WORDS(
		"class", "dur", "extends", "float", "implements", "int", "interface",
		"private", "protected", "public", "pure", "same", "static", "super",
		"this", "time", "void"),

	.constants = WORDS(
		"E", "I", "J", "NULL", "PI", "RANDOM_MAX", "TWO_PI", "adc", "blackhole",
		"dac", "day", "e", "false", "hour", "i", "j", "maybe", "me", "minute",
		"ms", "now", "null", "pi", "samp", "second", "true", "week"),

	.functions = WORDS(
		"abs", "acos", "add", "asin", "atan", "atan2", "atof", "atoi", "ceil",
		"cos", "cosh", "crash", "dbtopow", "dbtorms", "exp", "fabs", "floor",
		"fmod", "ftom", "getenv", "hypot", "isinf", "isnan", "log", "log10",
		"log2", "max", "min", "mtof", "nextpow2", "pow", "powtodb", "random",
		"random2", "random2f", "randomf", "remainder", "remove", "replace",
		"rmstodb", "round", "setenv", "sgn", "sin", "sinh", "spork", "sqrt",
		"srandom", "status", "system", "tan", "tanh", "trunc"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",
};
