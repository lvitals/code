/* Ruby */
#include "../lang_def.h"

LANGUAGE(ruby) {
	.name = "ruby",
	.title = "Ruby",
	.flags = F_VARS,
	.extensions = "rake rb rbw",
	.filenames = "Vagrantfile Rakefile Gemfile",
	.interpreters = "ruby",

	.keywords = WORDS(
		"BEGIN", "END", "__FILE__", "__LINE__", "alias", "and", "begin",
		"break", "case", "class", "def", "defined?", "do", "else", "elsif",
		"end", "ensure", "false", "for", "if", "in", "module", "next", "nil",
		"not", "or", "redo", "rescue", "retry", "return", "self", "super",
		"then", "true", "undef", "unless", "until", "when", "while", "yield"),

	.functions = WORDS(
		"at_exit", "autoload", "binding", "caller", "catch", "chomp", "chomp!",
		"chop", "chop!", "eval", "exec", "exit", "exit!", "extend", "fail",
		"fork", "format", "gets", "global_variables", "gsub", "gsub!",
		"include", "iterator?", "lambda", "load", "local_variables", "loop",
		"module_function", "open", "p", "print", "printf", "proc", "putc",
		"puts", "raise", "rand", "readline", "readlines", "require",
		"require_relative", "select", "sleep", "split", "sprintf", "srand",
		"sub", "sub!", "syscall", "system", "test", "trace_var", "trap",
		"untrace_var"),

	.line_comments = { "#" },
	.block_comments = { { "=begin", "=end" } },
	.quotes = "`'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("begin", "end"),
		WORD_BLOCK("class", "end"),
		WORD_BLOCK("def", "end"),
		WORD_BLOCK("do", "end"),
		WORD_BLOCK("for", "end"),
		WORD_BLOCK("module", "end"),
		WORD_BLOCK("case", "end"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
