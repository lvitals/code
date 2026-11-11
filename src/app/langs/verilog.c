/* Verilog */
#include "../lang_def.h"

LANGUAGE(verilog) {
	.name = "verilog",
	.title = "Verilog",
	.extensions = "sv v ver",

	.keywords = WORDS(
		"`autoexpand_vectornets", "`celldefine", "`default_nettype", "`define",
		"`else", "`endcelldefine", "`endif", "`endprotect", "`endprotected",
		"`expand_vectornets", "`ifdef", "`ifndef", "`include",
		"`noexpand_vectornets", "`noremove_gatename", "`noremove_netname",
		"`nounconnected_drive", "`protect", "`protected", "`remove_gatename",
		"`remove_netname", "`resetall", "`signed", "`timescale",
		"`unconnected_drive", "`undef", "`unsigned", "always", "assign",
		"begin", "case", "casex", "casez", "deassign", "default", "disable",
		"else", "end", "endcase", "endfunction", "endgenerate", "endmodule",
		"endprimitive", "endspecify", "endtable", "endtask", "for", "force",
		"forever", "fork", "function", "generate", "highz0", "highz1", "if",
		"initial", "join", "large", "macromodule", "medium", "module",
		"negedge", "posedge", "primitive", "pull0", "pull1", "release",
		"repeat", "small", "specify", "strong0", "strong1", "table", "task",
		"wait", "weak0", "weak1", "while"),

	.types = WORDS(
		"defparam", "event", "inout", "input", "integer", "output", "parameter",
		"realtime", "reg", "scalared", "supply0", "supply1", "time", "tri",
		"tri0", "tri1", "triand", "trior", "trireg", "vectored", "wand", "wire",
		"wor"),

	.functions = WORDS(
		"$cleartrace", "$display", "$finish", "$monitoroff", "$monitoron",
		"$printtimescale", "$random", "$realtime", "$settrace", "$showscopes",
		"$showvars", "$stime", "$stop", "$time", "$timeformat", "and", "buf",
		"bufif0", "bufif1", "cmos", "nand", "nmos", "nor", "not", "notif0",
		"notif1", "or", "pmos", "pulldown", "pullup", "rcmos", "rnmos", "rpmos",
		"rtran", "rtranif0", "rtranif1", "tran", "tranif0", "tranif1", "xnor",
		"xor"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "\"",

	.blocks = BLOCKS(
		WORD_BLOCK("case", "endcase"),
		WORD_BLOCK("casex", "endcase"),
		WORD_BLOCK("casez", "endcase"),
		WORD_BLOCK("function", "endfunction"),
		WORD_BLOCK("fork", "join"),
		WORD_BLOCK("table", "endtable"),
		WORD_BLOCK("task", "endtask"),
		WORD_BLOCK("generate", "endgenerate"),
		WORD_BLOCK("specify", "endspecify"),
		WORD_BLOCK("primitive", "endprimitive"),
		WORD_BLOCK("module", "endmodule"),
		WORD_BLOCK("begin", "end"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("{", "}")),
};
