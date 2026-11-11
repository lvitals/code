/* Routeros */
#include "../lang_def.h"

LANGUAGE(routeros) {
	.name = "routeros",
	.title = "Routeros",
	.extensions = "rsc",

	.keywords = WORDS(
		":beep", ":blink", ":delay", ":do", ":environment", ":error",
		":execute", ":find", ":for", ":foreach", ":global", ":if", ":len",
		":local", ":log", ":parse", ":pick", ":ping", ":put", ":resolve",
		":return", ":set", ":terminal", ":time", ":toarray", ":tobool", ":toid",
		":toip", ":toip6", ":tonum", ":tostr", ":totime", ":typeof", ":while",
		"add", "alert", "and", "append", "as-value", "brief", "count-only",
		"critical", "debug", "detail", "disable", "do", "edit", "else",
		"emergency", "enable", "error", "export", "false", "file", "find",
		"follow", "follow-only", "from", "get", "in", "info", "interval",
		"monitor", "no", "notice", "on-error", "or", "print", "remove", "set",
		"step", "terse", "to", "true", "value-list", "warning", "where",
		"while", "without-paging", "yes"),

	.line_comments = { "#" },
	.quotes = "\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
