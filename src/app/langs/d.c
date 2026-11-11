/* D */
#include "../lang_def.h"

LANGUAGE(d) {
	.name = "d",
	.title = "D",
	.flags = F_HASHPRE,
	.extensions = "d di",

	.keywords = WORDS(
		"__gshared", "__parameters", "__thread", "__traits", "__vector",
		"abstract", "align", "asm", "assert", "auto", "body", "break", "case",
		"cast", "catch", "const", "continue", "debug", "default", "delete",
		"deprecated", "do", "else", "export", "extern", "false", "final",
		"finally", "for", "foreach", "foreach_reverse", "goto", "if",
		"immutable", "import", "in", "inout", "invariant", "is", "lazy",
		"macro", "mixin", "new", "nothrow", "null", "out", "override", "pragma",
		"private", "protected", "public", "pure", "ref", "return", "scope",
		"shared", "static", "super", "switch", "synchronized", "this",
		"throwtrue", "try", "typeid", "typeof", "unittest", "version",
		"virtual", "volatile", "while", "with"),

	.types = WORDS(
		"alias", "bool", "byte", "cdouble", "cent", "cfloat", "char", "class",
		"creal", "dchar", "delegate", "double", "dstring", "enum", "equals_t",
		"float", "function", "hash_t", "idouble", "ifloat", "int", "interface",
		"ireal", "long", "module", "package", "ptrdiff_t", "real", "short",
		"size_t", "string", "struct", "template", "typedef", "ubyte", "ucent",
		"uint", "ulong", "union", "ushort", "void", "wchar", "wstring"),

	.constants = WORDS(
		"__DATE__", "__EOF__", "__FILE__", "__FUNCTION__", "__LINE__",
		"__MODULE__", "__PRETTY_FUNCTION__", "__TIMESTAMP__", "__TIME__",
		"__VENDOR__", "__VERSION__", "exit", "failure", "success", "version"),

	.functions = WORDS(
		"AArch64", "AIX", "ARM", "ARM_HardFloat", "ARM_SoftFP", "ARM_SoftFloat",
		"ARM_Thumb", "Alpha", "Alpha_HardFloat", "Alpha_SoftFloat", "Android",
		"BSD", "BigEndian", "Cygwin", "D_Coverage", "D_Ddoc", "D_HardFloat",
		"D_InlineAsm_X86", "D_InlineAsm_X86_64", "D_LP64", "D_NoBoundsChecks",
		"D_PIC", "D_SIMD", "D_SoftFloat", "D_Version2", "D_X32", "DigitalMars",
		"DragonFlyBSD", "FreeBSD", "GNU", "HPPA", "HPPA64", "Haiku", "Hurd",
		"IA64", "LDC", "LittleEndian", "MIPS32", "MIPS64", "MIPS_EABI",
		"MIPS_HardFloat", "MIPS_N32", "MIPS_N64", "MIPS_O32", "MIPS_O64",
		"MIPS_SoftFloat", "NetBSD", "OSX", "OpenBSD", "PPC", "PPC64",
		"PPC_HardFloat", "PPC_SoftFloat", "Posix", "S390", "S390X", "SDC", "SH",
		"SH64", "SPARC", "SPARC64", "SPARC_HardFloat", "SPARC_SoftFloat",
		"SPARC_V8Plus", "SkyOS", "Solaris", "SysV3", "SysV4", "Win32", "Win64",
		"Windows", "X86", "X86_64", "alignof", "all", "allMembers", "assert",
		"classInstanceSize", "compiles", "derivedMembers", "dig", "dup",
		"epsilon", "getAttributes", "getMember", "getOverloads",
		"getProtection", "getUnitTests", "getVirtualFunctions",
		"getVirtualIndex", "getVirtualMethods", "hasMember", "identifier",
		"idup", "im", "infinity", "init", "isAbstractClass",
		"isAbstractFunction", "isArithmetic", "isAssociativeArray",
		"isFinalClass", "isFinalFunction", "isFloating", "isIntegral", "isLazy",
		"isNested", "isOut", "isOverrideFunction", "isPOD", "isRef", "isSame",
		"isScalar", "isStaticArray", "isStaticFunction", "isUnsigned",
		"isVirtualFunction", "isVirtualMethod", "keys", "length", "linux",
		"mangleof", "mant_dig", "max", "max_10_exp", "max_exp", "min",
		"min_10_exp", "min_exp", "min_normal", "nan", "none", "offsetof",
		"parent", "property", "ptr", "re", "rehash", "reverse", "sizeof",
		"sort", "stringof", "trait", "tupleof", "unittest", "values"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" }, { "/+", "+/" } },
	.long_strings = { { "q\"", "\"" } },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
