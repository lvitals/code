/* reStructuredText */
#include "../lang_def.h"

LANGUAGE(rest) {
	.name = "rest",
	.title = "reStructuredText",
	.extensions = "rst",

	.keywords = WORDS(
		"admonition", "attention", "caution", "centered", "citations", "class",
		"code", "compound", "container", "contents", "csv-table", "danger",
		"date", "default-role", "deprecated", "epigraph", "error", "figure",
		"footer", "footnotes", "glossary", "header", "highlight", "highlights",
		"hint", "hlist", "image", "important", "include", "index", "line-block",
		"list-table", "literalinclude", "math", "meta", "note", "only",
		"parsed-literal", "productionlist", "pull-quote", "raw", "replace",
		"restructuredtext-test-directive", "role", "rubric",
		"section-autonumbering", "sectionauthor", "sectnum", "seealso",
		"sidebar", "table", "tabularcolumns", "target-notes", "tip", "title",
		"toctree", "topic", "unicode", "versionadded", "versionchanged",
		"warning"),
};
