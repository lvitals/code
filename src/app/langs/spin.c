/* Spin */
#include "../lang_def.h"

LANGUAGE(spin) {
	.name = "spin",
	.title = "Spin",
	.extensions = "spin",

	.keywords = WORDS(
		"_clkfreq", "_clkmode", "_free", "_stack", "_xinfreq", "abort", "abs",
		"absneg", "add", "addabs", "adds", "addsx", "addx", "and", "andn",
		"byte", "bytefill", "bytemove", "call", "case", "chipver", "clkfreq",
		"clkmode", "clkset", "cmp", "cmps", "cmpsub", "cmpsx", "cmpx", "cnt",
		"cogid", "coginit", "cognew", "cogstop", "con", "constant", "ctra",
		"ctrb", "dat", "dira", "dirb", "djnz", "else", "elseif", "elseifnot",
		"enc", "false", "file", "fit", "float", "from", "frqa", "frqb", "hubop",
		"if", "if_a", "if_ae", "if_always", "if_b", "if_be", "if_c",
		"if_c_and_nz", "if_c_and_z", "if_c_eq_z", "if_c_ne_z", "if_c_or_nz",
		"if_c_or_z", "if_e", "if_nc", "if_nc_and_nz", "if_nc_and_z",
		"if_nc_or_nz", "if_nc_or_z", "if_ne", "if_never", "if_nz",
		"if_nz_and_c", "if_nz_and_nc", "if_nz_or_c", "if_nz_or_nc", "if_z",
		"if_z_and_c", "if_z_and_nc", "if_z_eq_c", "if_z_ne_c", "if_z_or_c",
		"if_z_or_nc", "ifnot", "ina", "inb", "jmp", "jmpret", "lockclr",
		"locknew", "lockret", "lockset", "long", "longfill", "longmove",
		"lookdown", "lookdownz", "lookup", "lookupz", "max", "maxs", "min",
		"mins", "mov", "movd", "movi", "movs", "mul", "muls", "muxc", "muxnc",
		"muxnz", "muxz", "neg", "negc", "negnc", "negnz", "negx", "negz",
		"next", "nop", "not", "nr", "obj", "ones", "or", "org", "other", "outa",
		"outb", "par", "phsa", "phsb", "pi", "pll16x", "pll1x", "pll2x",
		"pll4x", "pll8x", "posx", "pri", "pub", "quit", "rcfast", "rcl", "rcr",
		"rcslow", "rdbyte", "rdlong", "rdword", "reboot", "repeat", "res",
		"result", "ret", "return", "rev", "rol", "ror", "round", "sar", "shl",
		"shr", "spr", "step", "strcomp", "string", "strsize", "sub", "subabs",
		"subs", "subsx", "subx", "sumc", "sumnc", "sumnz", "sumz", "test",
		"testn", "tjnz", "tjz", "to", "true", "trunc", "until", "var", "vcfg",
		"vscl", "waitcnt", "waitpeq", "waitpne", "waitvid", "wc", "while",
		"word", "wordfill", "wordmove", "wr", "wrbyte", "wrlong", "wz",
		"xinput", "xor", "xtal1", "xtal2", "xtal3"),

	.line_comments = { "''", "'" },
	.block_comments = { { "{", "}" }, { "{{", "}}" } },
	.quotes = "\"",
};
