/* AWK */
#include "../lang_def.h"

LANGUAGE(awk) {
	.name = "awk",
	.title = "AWK",
	.flags = F_VARS,
	.extensions = "awk",
	.interpreters = "awk mawk nawk gawk goawk",

	.keywords = WORDS(
		"BEGIN", "END", "break", "close", "continue", "delete", "do", "else",
		"exit", "fflush", "for", "function", "getline", "if", "in", "next",
		"nextfile", "print", "printf", "return", "system", "while"),

	.functions = WORDS(
		"ARGC", "ARGIND", "ARGV", "BINMODE", "CONVFMT", "ENVIRON", "ERRNO",
		"FIELDWIDTHS", "FILENAME", "FNR", "FPAT", "FS", "FUNCTAB", "IGNORECASE",
		"LINT", "NF", "NR", "OFMT", "OFS", "ORS", "PREC", "PROCINFO", "RLENGTH",
		"ROUNDMODE", "RS", "RSTART", "RT", "SUBSEP", "SYMTAB", "TEXTDOMAIN",
		"atan2", "cos", "exp", "gsub", "index", "int", "length", "log", "match",
		"mktime", "rand", "sin", "split", "sprintf", "sqrt", "srand",
		"strftime", "sub", "substr", "systime", "tolower", "toupper"),

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
