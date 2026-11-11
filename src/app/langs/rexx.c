/* Rexx */
#include "../lang_def.h"

LANGUAGE(rexx) {
	.name = "rexx",
	.title = "Rexx",
	.flags = F_HASHPRE | F_NOCASE,
	.extensions = "orx rex",

	.keywords = WORDS(
		"address", "arg", "by", "call", "class", "do", "drop", "else", "end",
		"exit", "expose", "forever", "forward", "guard", "if", "interpret",
		"iterate", "leave", "method", "nop", "numeric", "otherwise", "parse",
		"procedure", "pull", "push", "queue", "raise", "rc", "reply",
		"requires", "result", "return", "routine", "say", "select", "self",
		"sigl", "signal", "super", "then", "to", "trace", "until", "use",
		"when", "while"),

	.functions = WORDS(
		"abbrev", "abs", "address", "arg", "b2x", "beep", "bitand", "bitor",
		"bitxor", "c2d", "c2x", "center", "changestr", "charin", "charout",
		"chars", "compare", "consition", "copies", "countstr", "d2c", "d2x",
		"datatype", "date", "delstr", "delword", "digits", "directory",
		"errortext", "filespec", "form", "format", "fuzz", "insert", "lastpos",
		"left", "length", "linein", "lineout", "lines", "max", "min", "overlay",
		"pos", "queued", "random", "reverse", "right", "rxfuncadd",
		"rxfuncdrop", "rxfuncquery", "rxmessagebox", "rxwinexec", "sign",
		"sourceline", "space", "stream", "strip", "substr", "subword", "symbol",
		"sysaddrexxmacro", "sysbootdrive", "sysclearrexxmacrospace",
		"syscloseeventsem", "sysclosemutexsem", "syscls", "syscreateeventsem",
		"syscreatemutexsem", "syscurpos", "syscurstate", "sysdriveinfo",
		"sysdrivemap", "sysdropfuncs", "sysdroprexxmacro", "sysdumpvariables",
		"sysfiledelete", "sysfilesearch", "sysfilesystemtype", "sysfiletree",
		"sysfromunicode", "sysgeterrortext", "sysgetfiledatetime", "sysgetkey",
		"sysini", "sysloadfuncs", "sysloadrexxmacrospace", "sysmkdir",
		"sysopeneventsem", "sysopenmutexsem", "sysposteventsem",
		"syspulseeventsem", "sysqueryprocess", "sysqueryrexxmacro",
		"sysreleasemutexsem", "sysreorderrexxmacro", "sysrequestmutexsem",
		"sysreseteventsem", "sysrmdir", "syssaverexxmacrospace",
		"syssearchpath", "syssetfiledatetime", "syssetpriority", "syssleep",
		"sysstemcopy", "sysstemdelete", "syssteminsert", "sysstemsort",
		"sysswitchsession", "syssystemdirectory", "systempfilename",
		"systextscreenread", "systextscreensize", "systounicode",
		"sysutilversion", "sysversion", "sysvolumelabel", "syswaiteventsem",
		"syswaitnamedpipe", "syswindecryptfile", "syswinencryptfile",
		"syswinver", "time", "trace", "translate", "trunc", "value", "var",
		"verify", "word", "wordindex", "wordlength", "wordpos", "words", "x2b",
		"x2c", "x2d", "xrange"),

	.line_comments = { "--" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("do", "end"),
		WORD_BLOCK("select", "return")),
};
