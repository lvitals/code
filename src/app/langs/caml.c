/* OCaml */
#include "../lang_def.h"

LANGUAGE(caml) {
	.name = "caml",
	.title = "OCaml",
	.extensions = "caml ml mli mll mly",

	.keywords = WORDS(
		"and", "as", "asr", "begin", "class", "closed", "constraint", "do",
		"done", "downto", "else", "end", "exception", "external", "failwith",
		"false", "flush", "for", "fun", "function", "functor", "if", "in",
		"include", "incr", "inherit", "land", "let", "load", "los", "lsl",
		"lsr", "lxor", "match", "method", "mod", "module", "mutable", "new",
		"not", "of", "open", "option", "or", "parser", "private", "raise",
		"rec", "ref", "regexp", "sig", "stderr", "stdin", "stdout", "struct",
		"then", "to", "true", "try", "type", "val", "virtual", "when", "while",
		"with"),

	.types = WORDS(
		"bool", "char", "float", "int", "string", "unit"),

	.functions = WORDS(
		"abs", "abs_float", "acos", "asin", "at_exit", "atan", "atan2",
		"bool_of_string", "ceil", "char_of_int", "classify_float", "close_in",
		"close_in_noerr", "close_out", "close_out_noerr", "compare", "cos",
		"cosh", "decr", "epsilon_float", "exit", "exp", "failwith", "float",
		"float_of_int", "float_of_string", "floor", "flush", "flush_all",
		"format_of_string", "frexp", "fst", "ignore", "in_channel_length",
		"incr", "infinity", "input", "input_binary_int", "input_byte",
		"input_char", "input_line", "input_value", "int_of_char",
		"int_of_float", "int_of_string", "invalid_arg", "ldexp", "log", "log10",
		"max", "max_float", "max_int", "min", "min_float", "min_int", "mod",
		"mod_float", "modf", "nan", "open_in", "open_in_bin", "open_in_gen",
		"open_out", "open_out_bin", "open_out_gen", "out_channel_length",
		"output", "output_binary_int", "output_byte", "output_char",
		"output_string", "output_value", "pos_in", "pos_out", "pred",
		"prerr_char", "prerr_endline", "prerr_float", "prerr_int",
		"prerr_newline", "prerr_string", "print_char", "print_endline",
		"print_float", "print_int", "print_newline", "print_string", "raise",
		"read_float", "read_int", "read_line", "really_input", "seek_in",
		"seek_out", "set_binary_mode_in", "set_binary_mode_out", "sin", "sinh",
		"snd", "sqrt", "stderr", "stdin", "stdout", "string_of_bool",
		"string_of_float", "string_of_format", "string_of_int", "succ", "tan",
		"tanh", "truncate"),

	.block_comments = { { "(*", "*)" } },
	.quotes = "'\"",
};
