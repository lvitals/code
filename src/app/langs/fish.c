/* Fish */
#include "../lang_def.h"

LANGUAGE(fish) {
	.name = "fish",
	.title = "Fish",
	.flags = F_VARS,
	.extensions = "fish",
	.interpreters = "fish",

	.keywords = WORDS(
		"alias", "and", "begin", "bg", "bind", "block", "break", "breakpoint",
		"builtin", "case", "cd", "command", "commandline", "complete",
		"contains", "continue", "count", "dirh", "dirs", "echo", "else", "emit",
		"end", "eval", "exec", "exit", "fg", "fish", "fish_config",
		"fish_indent", "fish_pager", "fish_prompt", "fish_right_prompt",
		"fish_update_completions", "fishd", "for", "funced", "funcsave",
		"function", "functions", "help", "history", "if", "in", "isatty",
		"jobs", "math", "mimedb", "nextd", "not", "open", "or", "popd", "prevd",
		"psub", "pushd", "pwd", "random", "read", "return", "set", "set_color",
		"source", "status", "switch", "test", "trap", "type", "ulimit", "umask",
		"vared", "while"),

	.line_comments = { "#!/", "#" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("begin", "end"),
		WORD_BLOCK("for", "end"),
		WORD_BLOCK("function", "end"),
		WORD_BLOCK("if", "end"),
		WORD_BLOCK("switch", "end"),
		WORD_BLOCK("while", "end")),
};
