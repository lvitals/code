/* PKGBUILD */
#include "../lang_def.h"

LANGUAGE(pkgbuild) {
	.name = "pkgbuild",
	.title = "PKGBUILD",
	.flags = F_VARS,
	.filenames = "PKGBUILD",

	.keywords = WORDS(
		"-G", "-L", "-N", "-Np", "-O", "-S", "-a", "-b", "-c", "-d", "-e",
		"-ef", "-eq", "-f", "-g", "-ge", "-gt", "-h", "-i", "-k", "-le", "-lt",
		"-n", "-ne", "-nt", "-o", "-ot", "-p", "-r", "-s", "-t", "-u", "-w",
		"-x", "-z", "case", "cd", "clone", "co", "continue", "cp", "do", "done",
		"echo", "elif", "else", "esac", "fi", "for", "gconf-merge-schema",
		"git", "if", "in", "install", "ln", "local", "make", "mkdir", "msg",
		"patch", "return", "rm", "sed", "svn", "then", "while"),

	.constants = WORDS(
		"arch", "backup", "changelog", "checkdepends", "conflicts", "depends",
		"epoch", "groups", "install", "license", "makedepends", "md5sums",
		"noextract", "optdepends", "options", "pkgbase", "pkgdesc", "pkgname",
		"pkgrel", "pkgver", "provides", "replaces", "sha1sums", "sha256sums",
		"sha384sums", "sha512sums", "source", "url", "validpgpkeys"),

	.functions = WORDS(
		"build", "check", "package", "pkgver", "prepare"),

	.line_comments = { "#" },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		OP_BLOCK("(", ")"),
		OP_BLOCK("{", "}")),
};
