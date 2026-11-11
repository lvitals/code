/* Pony */
#include "../lang_def.h"

LANGUAGE(pony) {
	.name = "pony",
	.title = "Pony",
	.extensions = "pony",

	.keywords = WORDS(
		"actor", "as", "be", "break", "class", "compile_error",
		"compile_intrinsic", "consume", "continue", "do", "else", "elseif",
		"embed", "end", "error", "for", "fun", "if", "ifdef", "iftype", "in",
		"interface", "is", "isnt", "lambda", "let", "match", "new", "object",
		"primitive", "recover", "repeat", "return", "struct", "then", "this",
		"trait", "try", "type", "until", "use", "var", "where", "while", "with"),

	.constants = WORDS(
		"false", "true"),

	.functions = WORDS(
		"_deserialise", "_final", "_serialise", "_serialise_space", "add",
		"add_unsafe", "apply", "create", "dispose", "div", "div_unsafe", "eq",
		"eq_unsafe", "ge", "ge_unsafe", "gt", "gt_unsafe", "has_next", "le",
		"le_unsafe", "lt", "lt_unsafe", "mod", "mod_unsafe", "mul",
		"mul_unsafe", "ne", "ne_unsafe", "neg", "neg_unsafe", "next", "op_and",
		"op_not", "op_or", "op_xor", "shl", "shl_unsafe", "shr", "shr_unsafe",
		"sub", "sub_unsafe", "update"),

	.preproc = WORDS(
		"alias", "any", "box", "iso", "read", "ref", "send", "share", "tag",
		"trn", "val"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "'\"",
};
