/* Rust */
#include "../lang_def.h"

LANGUAGE(rust) {
	.name = "rust",
	.title = "Rust",
	.flags = F_HASHPRE | F_ATTR | F_NOQ1,
	.extensions = "rs",

	.keywords = WORDS(
		"SelfTy", "as", "async", "await", "break", "const", "continue", "crate",
		"dyn", "else", "enum", "extern", "false", "fn", "for", "if", "impl",
		"in", "let", "loop", "match", "mod", "move", "mut", "pub", "ref",
		"return", "self", "static", "struct", "super", "trait", "true", "type",
		"union", "unsafe", "use", "where", "while"),

	.types = WORDS(
		"array", "bool", "char", "f32", "f64", "fn", "i128", "i16", "i32",
		"i64", "i8", "isize", "never", "pointer", "reference", "slice", "str",
		"tuple", "u128", "u16", "u32", "u64", "u8", "unit", "usize"),

	.functions = WORDS(
		"assert", "assert_eq", "assert_ne", "cfg", "column", "compile_error",
		"concat", "concat_bytes", "concat_idents", "const_format_args", "dbg",
		"debug_assert", "debug_assert_eq", "debug_assert_ne", "env", "eprint",
		"eprintln", "file", "format", "format_args", "format_args_nl",
		"include", "include_bytes", "include_str", "line", "log_syntax",
		"matches", "module_path", "option_env", "panic", "print", "println",
		"stringify", "thread_local", "todo", "trace_macros", "try",
		"unimplemented", "unreachable", "vec", "write", "writeln"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("(", ")"),
		OP_BLOCK("{", "}")),
};
