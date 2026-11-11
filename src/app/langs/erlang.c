/* Erlang */
#include "../lang_def.h"

LANGUAGE(erlang) {
	.name = "erlang",
	.title = "Erlang",
	.extensions = "erl hrl",

	.keywords = WORDS(
		"after", "and", "andalso", "badarg", "band", "begin", "bnot", "bor",
		"bsl", "bsr", "bxor", "case", "catch", "cond", "div", "end", "false",
		"fun", "if", "let", "nocookie", "not", "of", "or", "orelse", "query",
		"receive", "rem", "true", "try", "when", "xor"),

	.functions = WORDS(
		"abs", "acos", "alive", "any", "apply", "asin", "atan", "atan2", "atom",
		"atom_to_list", "binary", "binary_to_list", "binary_to_term",
		"bitstring", "byte", "check_process_code", "concat_binary", "constant",
		"cos", "cosh", "date", "delete_module", "disconnect_node", "element",
		"erase", "exit", "exp", "float", "float_to_list", "function", "get",
		"get_cookie", "get_keys", "group_leader", "halt", "hash", "hd",
		"integer", "integer_to_list", "is_alive", "is_record", "length", "link",
		"list", "list_to_atom", "list_to_binary", "list_to_float",
		"list_to_integer", "list_to_pid", "list_to_tuple", "load_module", "log",
		"log10", "make_ref", "map", "math", "max", "mfa", "min",
		"module_loaded", "monitor_node", "node", "nodes", "non_neg_integer",
		"now", "number", "open_port", "pi", "pid", "pid_to_list", "port_close",
		"port_info", "ports", "pos_integer", "pow", "power", "preloaded",
		"process", "process_flag", "process_info", "processes", "purge_module",
		"put", "record", "reference", "register", "registered", "round", "self",
		"set_cookie", "set_node", "setelement", "sin", "sinh", "size", "spawn",
		"spawn_link", "split_binary", "sqrt", "statistics", "tan", "tanh",
		"term_to_binary", "throw", "time", "tl", "trunc", "tuple_to_list",
		"unlink", "unregister", "whereis"),

	.preproc = WORDS(
		"author", "behavior", "behaviour", "compile", "copyright", "define",
		"doc", "else", "endif", "export", "file", "ifdef", "ifndef", "import",
		"include", "include_lib", "module", "record", "spec", "type", "undef"),

	.line_comments = { "%" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("case", "end"),
		WORD_BLOCK("fun", "end"),
		WORD_BLOCK("if", "end"),
		WORD_BLOCK("query", "end"),
		WORD_BLOCK("receive", "end"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
