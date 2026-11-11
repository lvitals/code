/* Elixir */
#include "../lang_def.h"

LANGUAGE(elixir) {
	.name = "elixir",
	.title = "Elixir",
	.flags = F_BACKTICK,
	.extensions = "ex exs",

	.keywords = WORDS(
		"after", "alias", "case", "catch", "cond", "do", "else", "end", "exit",
		"fn", "for", "if", "import", "is_atom", "is_binary", "is_bitstring",
		"is_boolean", "is_exception", "is_float", "is_function", "is_integer",
		"is_list", "is_map", "is_number", "is_pid", "is_port", "is_record",
		"is_reference", "is_tuple", "quote", "raise", "receive", "require",
		"rescue", "self", "send", "super", "throw", "try", "unless", "unquote",
		"use", "when", "with"),

	.constants = WORDS(
		"false", "nil", "true"),

	.functions = WORDS(
		"def", "defcallback", "defdelegate", "defexception", "defimpl",
		"defmacro", "defmacrop", "defmodule", "defoverridable", "defp",
		"defprotocol", "defrecord", "defrecordp", "defstruct"),

	.line_comments = { "#" },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "\"",
};
