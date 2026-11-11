/* vCard */
#include "../lang_def.h"

LANGUAGE(vcard) {
	.name = "vcard",
	.title = "vCard",
	.flags = F_NOCASE,
	.extensions = "vcf vcard",

	.keywords = WORDS(
		"BEGIN", "END", "FN", "N", "VERSION"),

	.types = WORDS(
		"ADR", "AGENT", "ANNIVERSARY", "BDAY", "CALADRURI", "CALURI",
		"CATEGORIES", "CLASS", "CLIENTPIDMAP", "EMAIL", "END", "FBURL",
		"GENDER", "GEO", "IMPP", "KEY", "KIND", "LABEL", "LANG", "LOGO",
		"MAILER", "MEMBER", "NAME", "NICKNAME", "NOTE", "ORG", "PHOTO",
		"PRODID", "PROFILE", "RELATED", "REV", "ROLE", "SORT-STRING", "SOUND",
		"SOURCE", "TEL", "TITLE", "TZ", "UID", "URL", "XML"),

	.blocks = BLOCKS(
		WORD_BLOCK("BEGIN", "END")),
};
