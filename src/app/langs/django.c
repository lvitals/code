/* Django */
#include "../lang_def.h"

LANGUAGE(django) {
	.name = "django",
	.title = "Django",

	.keywords = WORDS(
		"and", "as", "autoescape", "block", "blocktranslate", "comment",
		"csrf_token", "cycle", "debug", "elif", "else", "empty",
		"endautoescape", "endblock", "endblocktranslate", "endcomment",
		"endfilter", "endfor", "endif", "endifchanged", "endspaceless",
		"endverbatim", "endwith", "extends", "filter", "firstof", "for",
		"get_available_languages", "get_current_language",
		"get_current_language_bidi", "get_language_info",
		"get_language_info_list", "get_media_prefix", "get_static_prefix", "if",
		"ifchanged", "in", "include", "is", "language", "load", "lorem", "not",
		"now", "or", "regroup", "resetcycle", "spaceless", "templatetag",
		"translate", "url", "verbatim", "widthratio", "with"),

	.functions = WORDS(
		"add", "addslashes", "capfirst", "center", "cut", "date", "default",
		"default_if_none", "dictsort", "dictsortreversed", "divisibleby",
		"escape", "escapejs", "filesizeformat", "first", "floatformat",
		"force_escape", "get_digit", "iriencode", "join", "json_script",
		"language_bidi", "language_name", "language_name_local",
		"language_name_translated", "last", "length", "length_is", "linebreaks",
		"linebreaksbr", "linenumbers", "ljust", "lower", "make_list",
		"phone2numeric", "pluralize", "pprint", "random", "rjust", "safe",
		"safeseq", "slice", "slugify", "stringformat", "striptags", "time",
		"timesince", "timeuntil", "title", "truncatechars_html",
		"truncatewords", "truncatewords_html", "unordered_list", "upper",
		"urlencode", "urlize", "urlizetrunc", "wordcount", "wordwrap", "yesno"),

	.block_comments = { { "{#", "#}" } },
	.quotes = "\"",

	.blocks = BLOCKS(
		PRE_BLOCK("{{", "}}"),
		PRE_BLOCK("{%", "%}")),
};
