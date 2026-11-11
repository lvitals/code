/* Smalltalk */
#include "../lang_def.h"

LANGUAGE(smalltalk) {
	.name = "smalltalk",
	.title = "Smalltalk",
	.extensions = "changes st sources",

	.keywords = WORDS(
		"Smalltalk", "Transcript", "false", "isNil", "nil", "not", "self",
		"super", "true"),

	.types = WORDS(
		"Array", "Boolean", "Character", "Date", "False", "Integer", "Object",
		"String", "Symbol", "Time", "True"),

	.quotes = "'",

	.blocks = BLOCKS(
		OP_BLOCK("[", "]")),
};
