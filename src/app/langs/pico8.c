/* PICO-8 */
#include "../lang_def.h"

LANGUAGE(pico8) {
	.name = "pico8",
	.title = "PICO-8",
	.flags = F_LUALONG,
	.extensions = "p8",

	.keywords = WORDS(
		"__gff__", "__gfx__", "__lua__", "__map__", "__music__", "__sfx__"),

	.line_comments = { "//" },
};
