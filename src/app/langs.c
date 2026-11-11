/* The registry of the languages: each one of src/app/langs/all.h (its
 * file src/app/langs/NAME.c), after plain text.  Nothing to change here:
 * a language is added by its file and its line in all.h. */
#include "syntax.h"

#define LANG(name)	extern const struct syn lang_##name;
#include "langs/all.h"
#undef LANG

static const struct syn plain = {
	.name = "text",
	.title = "Plain Text",
	.kind = K_PLAIN,
};

const struct syn *const syns[] = {
	&plain,
#define LANG(name)	&lang_##name,
#include "langs/all.h"
#undef LANG
};
const int nsyns = (int)(sizeof syns / sizeof syns[0]);
