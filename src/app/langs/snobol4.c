/* Snobol4 */
#include "../lang_def.h"

LANGUAGE(snobol4) {
	.name = "snobol4",
	.title = "Snobol4",
	.flags = F_NOCASE,
	.extensions = "sno SNO",

	.keywords = WORDS(
		"ABORT", "ARRAY", "CONTINUE", "DEFINE", "END", "FRETURN", "INPUT",
		"NRETURN", "OUTPUT", "PUNCH", "RETURN", "SCONTINUE", "TABLE"),

	.types = WORDS(
		"ABORT", "ANY", "ARB", "ARBNO", "BAL", "BREAK", "BREAKX", "FAIL",
		"FENCE", "LEN", "NOTANY", "POS", "REM", "RPOS", "RTAB", "SPAN",
		"SUCCEED", "TAB"),

	.functions = WORDS(
		"APPLY", "ARRAY", "CHAR", "CONVERT", "COPY", "DATA", "DATE", "DIFFER",
		"DUPL", "EQ", "EVAL", "FILE_ABSPATH", "FILE_ISDIR", "FREEZE",
		"FUNCTION", "GE", "GT", "HOST", "IDENT", "INTEGER", "IO_FINDUNIT",
		"ITEM", "LABEL", "LE", "LGT", "LOAD", "LPAD", "LT", "NE", "OPSYN",
		"ORD", "PROTOTYPE", "REMDR", "REPLACE", "REVERSE", "RPAD", "RSORT",
		"SERV_LISTEN", "SET", "SETEXIT", "SIZE", "SORT", "SQRT", "SSET",
		"SUBSTR", "TABLE", "THAW", "TIME", "TRACE", "TRIM", "UNLOAD", "VALUE",
		"VDIFFER"),

	.line_comments = { "*#|;!" },
	.quotes = "\"'",
};
