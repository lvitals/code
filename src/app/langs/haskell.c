/* Haskell */
#include "../lang_def.h"

LANGUAGE(haskell) {
	.name = "haskell",
	.title = "Haskell",
	.extensions = "hs",

	.keywords = WORDS(
		"_", "as", "case", "class", "data", "default", "deriving", "do", "else",
		"hiding", "if", "import", "in", "infix", "infixl", "infixr", "instance",
		"let", "module", "newtype", "of", "qualified", "then", "type", "where"),

	.line_comments = { "--" },
	.block_comments = { { "{-", "-}" } },
	.quotes = "'\"",
};
