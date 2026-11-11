/* AppleScript */
#include "../lang_def.h"

LANGUAGE(applescript) {
	.name = "applescript",
	.title = "AppleScript",
	.flags = F_NOCASE,
	.extensions = "applescript",

	.keywords = WORDS(
		"after", "and", "as", "back", "before", "beginning", "close",
		"considering", "contains", "continue", "copy", "count", "delete", "div",
		"duplicate", "each", "eighth", "else", "end", "equal", "equals",
		"error", "every", "exists", "exit", "fifth", "first", "fourth", "from",
		"front", "get", "given", "global", "id", "if", "ignoring", "in",
		"index", "into", "is", "isn't", "last", "launch", "local", "make",
		"middle", "mod", "move", "my", "named", "nd", "ninth", "not", "of",
		"on", "open", "or", "print", "prop", "property", "put", "quit", "rd",
		"reopen", "repeat", "return", "run", "save", "saving", "script",
		"second", "set", "seventh", "sixth", "some", "st", "tell", "tenth",
		"th", "the", "then", "third", "through", "thru", "timeout", "times",
		"to", "transaction", "try", "until", "where", "while", "whose", "with",
		"without"),

	.constants = WORDS(
		"anything", "apr", "april", "ask", "aug", "august", "bold", "case",
		"condensed", "days", "dec", "december", "diacriticals", "expanded",
		"expansion", "false", "feb", "february", "fri", "friday", "hidden",
		"hours", "hyphens", "it", "italic", "jan", "january", "jul", "july",
		"jun", "june", "mar", "march", "may", "me", "minutes", "mon", "monday",
		"month", "no", "nov", "november", "oct", "october", "outline", "pi",
		"plain", "punctuation", "result", "sat", "saturday", "sep", "september",
		"shadow", "space", "strikethrough", "subscript", "sun", "sunday",
		"superscript", "tab", "thu", "thursday", "true", "tue", "tuesday",
		"underline", "version", "wed", "wednesday", "weekday", "weeks", "yes"),

	.line_comments = { "--" },
	.block_comments = { { "(*", "*)" } },
	.quotes = "\"",
};
