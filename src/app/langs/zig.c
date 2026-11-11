/* Zig */
#include "../lang_def.h"

LANGUAGE(zig) {
	.name = "zig",
	.title = "Zig",
	.flags = F_ATTR,
	.extensions = "zig zon",

	.keywords = WORDS(
		"align", "allowzero", "and", "asm", "async", "await", "break",
		"callconv", "catch", "comptime", "const", "continue", "defer", "else",
		"errdefer", "extern", "fn", "for", "if", "inline", "linksection",
		"noalias", "noasync", "noinline", "nosuspend", "or", "orelse", "packed",
		"pub", "resume", "return", "self", "suspend", "switch", "test",
		"threadlocalanyframe", "try", "unreachable", "usingnamespace", "var",
		"volatile", "while"),

	.types = WORDS(
		"anyerror", "anytype", "bool", "c_int", "c_long", "c_longdouble",
		"c_longlong", "c_short", "c_uint", "c_ulong", "c_ulonglong", "c_ushort",
		"c_void", "comptime_float", "comptime_int", "enum", "error", "f128",
		"f16", "f32", "f64", "i128", "i16", "i32", "i64", "i8", "isize",
		"noreturn", "struct", "type", "u128", "u16", "u32", "u64", "u8",
		"union", "usize", "void"),

	.constants = WORDS(
		"false", "null", "true", "undefined"),

	.functions = WORDS(
		"Frame", "TagType", "This", "Type", "TypeOf", "addWithOverflow",
		"alignCast", "alignOf", "as", "asyncCall", "atomicLoad", "atomicRmw",
		"atomicStore", "bitCast", "bitOffsetOf", "bitReverse", "bitSizeOf",
		"boolToInt", "breakpoint", "byteOffsetOf", "byteSwap", "cDefine",
		"cImport", "cInclude", "cUndef", "call", "ceil", "clz", "cmpxchgStrong",
		"cmpxchgWeak", "compileError", "compileLog", "cos", "ctz", "divExact",
		"divFloor", "divTrunc", "embedFile", "enumToInt", "errSetCast",
		"errorName", "errorReturnTrace", "errorToInt", "exp", "exp2", "export",
		"fabs", "fence", "field", "fieldParentPtr", "floatCast", "floatToInt",
		"floor", "frame", "frameAddress", "frameSize", "hasDecl", "hasField",
		"import", "intCast", "intToEnum", "intToError", "intToFloat",
		"intToPtr", "log", "log10", "log2", "memcpy", "memset", "mod", "mulAdd",
		"mulWithOverflow", "panic", "popCount", "ptrCast", "ptrToInt", "reduce",
		"rem", "returnAddress", "round", "setAlignStack", "setCold",
		"setEvalBranchQuota", "setFloatMode", "setRuntimeSafety", "shlExact",
		"shlWithOverflow", "shrExact", "shuffle", "sin", "sizeOf", "splat",
		"sqrt", "src", "subWithOverflow", "tagName", "trunc", "truncate",
		"typeInfo", "typeName", "unionInit", "wasmMemoryGrow", "wasmMemorySize"),

	.line_comments = { "///", "//" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
