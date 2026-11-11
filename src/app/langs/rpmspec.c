/* RPM Spec */
#include "../lang_def.h"

LANGUAGE(rpmspec) {
	.name = "rpmspec",
	.title = "RPM Spec",
	.flags = F_VARS,
	.extensions = "spec",

	.keywords = WORDS(
		"AutoProv", "AutoReq", "AutoReqProv", "AutoRequires", "BuildArch",
		"BuildArchitectures", "BuildConflicts", "BuildPreReq", "BuildRequires",
		"BuildRoot", "Conflicts", "Distribution", "Enhances", "Epoch",
		"ExcludeArch", "ExclusiveArch", "ExclusiveOS", "Group", "Icon",
		"License", "Name", "Obsoletes", "Packager", "Patch", "Prefix", "Prereq",
		"Provides", "Recommends", "Release", "Requires", "Source", "Suggests",
		"Summary", "Supplements", "URL", "Vendor", "Version"),

	.constants = WORDS(
		"aarch64", "fedora", "ppc64le", "rhel", "riscv64", "s390x",
		"sle_version", "suse_version", "x86_64"),

	.line_comments = { "#" },
	.quotes = "\"",
};
