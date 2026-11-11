/* Git Rebase Message */
#include "../lang_def.h"

LANGUAGE(git_rebase) {
	.name = "git-rebase",
	.title = "Git Rebase Message",
	.filenames = "git-rebase-todo",
	.paths = "rebase-merge/done",

	.keywords = WORDS(
		"b", "break", "d", "drop", "e", "edit", "exec", "f", "fixup", "l",
		"label", "m", "merge", "p", "pick", "r", "reset", "reword", "s",
		"squash", "t", "x"),

	.column_comments = { COLUMN(1, "#") },
};
