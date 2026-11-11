/* XML */
#include "../lang_def.h"

LANGUAGE(xml) {
	.name = "xml",
	.title = "XML",
	.kind = K_XML,
	.extensions = "glif plist dtd svg xml xsd xsl xslt xul",

	.constants = WORDS(
		"amp", "apos", "gt", "lt", "quot"),

	.block_comments = { { "<!--", "-->" } },
	.quotes = "'\"",
};
