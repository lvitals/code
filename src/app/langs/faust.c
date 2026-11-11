/* Faust */
#include "../lang_def.h"

LANGUAGE(faust) {
	.name = "faust",
	.title = "Faust",
	.extensions = "dsp",

	.keywords = WORDS(
		"case", "component", "declare", "dependencies", "distributed",
		"environment", "fconstant", "ffunction", "float", "fvariable", "h:",
		"import", "inputs", "int", "library", "mdoctags", "outputs", "par",
		"prod", "seq", "sum", "t:", "v:", "waveform", "with", "xor"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "\"",
};
