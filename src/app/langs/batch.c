/* Batch */
#include "../lang_def.h"

LANGUAGE(batch) {
	.name = "batch",
	.title = "Batch",
	.flags = F_NOCASE,
	.extensions = "bat cmd",

	.keywords = WORDS(
		"AUX", "NUL", "PRN", "call", "cd", "chdir", "cls", "copy", "defined",
		"del", "do", "echo", "echo.", "else", "endlocal", "errorlevel", "exist",
		"exit", "for", "goto", "if", "in", "md", "mkdir", "move", "not",
		"pause", "popd", "pushd", "ren", "set", "setlocal", "shift"),

	.functions = WORDS(
		"APPEND", "ATTRIB", "CHKDSK", "CHOICE", "DEBUG", "DEFRAG", "DELTREE",
		"DISKCOMP", "DISKCOPY", "DOSKEY", "DRVSPACE", "EMM386", "EXPAND",
		"FASTOPEN", "FC", "FDISK", "FIND", "FORMAT", "GRAPHICS", "KEYB",
		"LABEL", "LOADFIX", "MEM", "MODE", "MORE", "MOVE", "MSCDEX", "NLSFUNC",
		"POWER", "PRINT", "RD", "REPLACE", "RESTORE", "SETVER", "SHARE", "SORT",
		"SUBST", "SYS", "TREE", "UNDELETE", "UNFORMAT", "VSAFE", "XCOPY"),

	.line_comments = { "::" },
	.quotes = "\"",

	.blocks = BLOCKS(
		WORD_BLOCK("setlocal", "endlocal")),
};
