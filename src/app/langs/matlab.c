/* MATLAB */
#include "../lang_def.h"

LANGUAGE(matlab) {
	.name = "matlab",
	.title = "MATLAB",
	.interpreters = "octave",

	.keywords = WORDS(
		"break", "case", "catch", "continue", "do", "else", "elseif", "end",
		"end_try_catch", "end_unwind_protect", "endfor", "endfunction", "endif",
		"endswitch", "endwhile", "for", "function", "global", "if", "otherwise",
		"persistent", "replot", "return", "static", "switch", "try", "until",
		"unwind_protect", "unwind_protect_cleanup", "varargin", "varargout",
		"while"),

	.constants = WORDS(
		"Inf", "NaN", "false", "inf", "nan", "pi", "realmax", "realmin", "true"),

	.functions = WORDS(
		"abs", "ans", "any", "argvatan2", "automatic_replot", "axes", "axis",
		"ceil", "cla", "clear", "clf", "columns", "cos", "default_return_value",
		"define_all_return_values", "delete", "diff", "disp",
		"do_fortran_indexing", "doc", "double", "drawnow",
		"empty_list_elements_ok", "eps", "exp", "figure", "find", "fix",
		"floor", "fprintf", "gca", "gcf", "get", "gnuplot_binary", "grid",
		"help", "hist", "hold", "ignore_function_time_stamp",
		"implicit_str_to_num_ok", "isempty", "isnull", "length", "load", "log",
		"log10", "loglog", "max", "mean", "median", "min", "mod", "ndims",
		"num2str", "numel", "ok_to_lose_imaginary_part", "ones",
		"output_max_field_width", "output_precision", "page_screen_output",
		"pause", "plot", "prefer_column_vectors", "prefer_zero_one_indexing",
		"print_answer_id_name", "print_empty_dimensions", "printf", "quit",
		"rand", "randn", "rectangle", "rem", "repmat", "reshape",
		"resize_on_range_error", "return_last_computed_value", "round", "rows",
		"save", "save_precision", "semilogx", "semilogy", "set", "sign",
		"silent_functions", "sin", "size", "size_equal", "sizeof", "sort",
		"split_long_rows", "sprintf", "sqrt", "squeeze", "std", "strcmp",
		"subplot", "sum", "suppress_verbose_help_message", "tan", "tic",
		"title", "toc", "treat_neg_dim_as_zero", "uicontrol",
		"warn_assign_as_truth_value", "warn_comma_in_global_decl",
		"warn_divide_by_zero", "warn_function_name_clash",
		"whitespace_in_literal_matrix", "who", "xlabel", "ylabel", "zeros"),

	.line_comments = { "%#" },
	.block_comments = { { "%{", "%}" } },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		WORD_BLOCK("if", "end"),
		WORD_BLOCK("for", "end"),
		WORD_BLOCK("while", "end"),
		WORD_BLOCK("switch", "end"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]")),
};
