/* VHDL */
#include "../lang_def.h"

LANGUAGE(vhdl) {
	.name = "vhdl",
	.title = "VHDL",
	.extensions = "vh vhd vhdl",

	.keywords = WORDS(
		"abs", "access", "after", "alias", "all", "and", "architecture",
		"array", "assert", "attribute", "begin", "block", "body", "buffer",
		"bus", "case", "component", "configuration", "constant", "disconnect",
		"downto", "else", "elsif", "end", "entity", "error", "exit", "failure",
		"false", "file", "for", "function", "generate", "generic", "group",
		"guarded", "if", "impure", "in", "inertial", "inout", "is", "label",
		"library", "linkage", "literal", "loop", "map", "mod", "nand", "new",
		"next", "nor", "not", "note", "null", "of", "on", "open", "or",
		"others", "out", "package", "port", "postponed", "procedure", "process",
		"pure", "range", "record", "register", "reject", "rem", "report",
		"return", "rol", "ror", "select", "severity", "shared", "signal", "sla",
		"sll", "sra", "srl", "subtype", "then", "to", "transport", "true",
		"type", "unaffected", "units", "until", "use", "variable", "wait",
		"warning", "when", "while", "with", "xnor", "xor"),

	.types = WORDS(
		"bit", "bit_vector", "boolean", "character", "integer", "line",
		"mux_bit", "mux_vectory", "natural", "positive", "qsim_12state",
		"qsim_12state_vector", "qsim_state", "qsim_state_vector",
		"qsim_strength", "real", "reg_bit", "reg_vector", "severity_level",
		"signed", "std_logic", "std_logic_vector", "std_ulogic",
		"std_ulogic_vector", "string", "text", "time", "unsigned", "wor_bit",
		"wor_vector"),

	.constants = WORDS(
		"ACTIVE", "ASCENDING", "BASE", "DELAYED", "DRIVING", "EVENT", "HIGH",
		"IMAGE", "INSTANCE", "LAST", "LEFT", "LEFTOF", "LENGTH", "LOW", "PATH",
		"POS", "PRED", "QUIET", "RANGE", "REVERSE", "RIGHT", "RIGHTOF",
		"SIMPLE", "STABLE", "SUCC", "TRANSACTION", "VAL", "VALUE"),

	.functions = WORDS(
		"resize", "rising_edge", "rotate_left", "rotate_right", "shift_left",
		"shift_right", "signed", "std_match", "to_bit", "to_bitvector",
		"to_integer", "to_signed", "to_stdlogicvector", "to_stdulogic",
		"to_stdulogicvector", "to_unsigned", "unsigned"),

	.line_comments = { "--" },
	.quotes = "'\"",
};
