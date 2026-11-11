/* Makefile */
#include "../lang_def.h"

LANGUAGE(makefile) {
	.name = "makefile",
	.title = "Makefile",
	.flags = F_VARS,
	.extensions = "mk iface mak makefile",
	.filenames = "GNUmakefile Makefile",
	.interpreters = "make",

	.keywords = WORDS(
		"define", "else", "elseif", "elseifdef", "elseifndef", "endef", "endif",
		"export", "if", "ifdef", "ifeq", "ifndef", "ifneq", "include", "load",
		"override", "private", "undefine", "unexport", "vpath"),

	.constants = WORDS(
		"special_targets"),

	.functions = WORDS(
		".DEFAULT_GOAL", ".FEATURES", ".INCLUDE_DIRS", ".LIBPATTERNS",
		".LOADED", ".RECIPEPREFIX", ".SHELLFLAGS", ".SHELLSTATUS", ".VARIABLES",
		"AR", "ARFLAGS", "AS", "ASFLAGS", "CC", "CFLAGS", "CO", "COFLAGS",
		"COMSPEC", "CPP", "CPPFLAGS", "CTANGLE", "CURDIR", "CWEAVE", "CXX",
		"CXXFLAGS", "DEFAULT", "DELETE_ON_ERROR", "DESTDIR",
		"EXPORT_ALL_VARIABLES", "FC", "FFLAGS", "GET", "GFLAGS", "GPATH",
		"IGNORE", "INTERMEDIATE", "LDFLAGS", "LDLIBS", "LEX", "LFLAGS", "LINT",
		"LINTFLAGS", "LOW_RESOLUTION_TIME", "M2C", "MAKE", "MAKECMDGOALS",
		"MAKEFILES", "MAKEFILE_LIST", "MAKEFLAGS", "MAKEINFO", "MAKELEVEL",
		"MAKEOVERRIDES", "MAKESHELL", "MAKE_RESTARTS", "MAKE_TERMERR",
		"MAKE_TERMOUT", "MFLAGS", "NOTPARALLEL", "ONESHELL", "OUTPUT_OPTION",
		"PC", "PFLAGS", "PHONY", "POSIX", "PRECIOUS", "RFLAGS", "RM",
		"SECONDARY", "SECONDEXPANSION", "SHELL", "SILENT", "SUFFIXES", "TANGLE",
		"TEX", "TEXI2DVI", "VPATH", "WEAVE", "YACC", "YFLAGS", "abspath",
		"addprefix", "addsuffix", "and", "basename", "bindir", "call", "dir",
		"error", "eval", "exec_prefix", "file", "filter", "filter-out",
		"findstring", "firstword", "flavor", "foreach", "if", "info", "join",
		"lastword", "libexecdir", "notdir", "or", "origin", "patsubst",
		"prefix", "realpath", "sbindir", "shell", "sort", "strip", "subst",
		"suffix", "value", "warning", "wildcard", "word", "wordlist", "words"),

	.line_comments = { "#" },
	.quotes = "'\"",
};
