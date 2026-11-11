/* BibTeX */
#include "../lang_def.h"

LANGUAGE(bibtex) {
	.name = "bibtex",
	.title = "BibTeX",
	.extensions = "bib",

	.functions = WORDS(
		"address", "affiliation", "article", "author", "book", "booklet",
		"booktitle", "chapter", "conference", "edition", "editor",
		"howpublished", "inbook", "incollection", "inproceedings",
		"institution", "isbn", "issn", "issue", "journal", "key", "keyword",
		"lambda", "manual", "mastersthesis", "misc", "month", "note", "number",
		"organization", "pages", "phdthesis", "proceedings", "publisher",
		"school", "series", "string", "techreport", "title", "type",
		"unpublished", "url", "volume", "year"),

	.preproc = WORDS(
		"entry"),

	.quotes = "\"",
};
