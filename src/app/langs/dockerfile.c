/* Dockerfile */
#include "../lang_def.h"

LANGUAGE(dockerfile) {
	.name = "dockerfile",
	.title = "Dockerfile",
	.flags = F_VARS,
	.filenames = "Dockerfile",

	.keywords = WORDS(
		"ADD", "ARG", "CMD", "COPY", "ENTRYPOINT", "ENV", "EXPOSE", "FROM",
		"LABEL", "MAINTAINER", "ONBUILD", "RUN", "STOPSIGNAL", "USER", "VOLUME",
		"WORKDIR"),

	.line_comments = { "#" },
	.quotes = "'\"",
};
