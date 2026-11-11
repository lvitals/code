/* Gherkin */
#include "../lang_def.h"

LANGUAGE(gherkin) {
	.name = "gherkin",
	.title = "Gherkin",
	.extensions = "feature",

	.keywords = WORDS(
		"And", "Background", "But", "Examples", "Feature", "Given", "Outline",
		"Scenario", "Scenarios", "Then", "When"),

	.line_comments = { "#" },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "\"",
};
