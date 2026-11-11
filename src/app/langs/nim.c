/* Nim */
#include "../lang_def.h"

LANGUAGE(nim) {
	.name = "nim",
	.title = "Nim",
	.flags = F_BACKTICK | F_NOCASE | F_ATTR,
	.extensions = "nim",

	.keywords = WORDS(
		"addr", "and", "as", "asm", "atomic", "bind", "block", "break", "case",
		"cast", "const", "continue", "converter", "discard", "distinct", "div",
		"do", "elif", "else", "end", "enum", "except", "export", "finally",
		"for", "from", "generic", "if", "import", "in", "include", "interface",
		"is", "isnot", "iterator", "lambda", "let", "macro", "method", "mixin",
		"mod", "nil", "not", "notin", "object", "of", "or", "out", "proc",
		"ptr", "raise", "ref", "return", "shared", "shl", "static", "template",
		"try", "tuple", "type", "var", "when", "while", "with", "without",
		"xor", "yield"),

	.types = WORDS(
		"Ordinal", "TChannel", "TInteger", "TNumber", "TOrdinal", "TReal",
		"TSignedInt", "TSlice", "TThread", "TUnsignedInt", "any", "array",
		"auto", "bool", "char", "cstring", "expr", "float", "float32",
		"float64", "int", "int16", "int32", "int64", "int8", "openarray",
		"pointer", "range", "seq", "set", "stmt", "string", "typeDesc", "uint",
		"uint16", "uint32", "uint64", "uint8", "varargs", "void"),

	.constants = WORDS(
		"CompileDate", "CompileTime", "NimMajor", "NimMinor", "NimPatch",
		"NimVersion", "QuitFailure", "QuitSuccess", "appType", "cpuEndian",
		"hostCPU", "hostOS", "inf", "isMainModule", "nan", "neginf", "off", "on"),

	.functions = WORDS(
		"Close", "CurrentSourcePath", "EndOfFile", "FlushFile",
		"GC_addCycleRoot", "GC_disable", "GC_disableMarkAndSweep", "GC_enable",
		"GC_enableMarkAndSweep", "GC_fullCollect", "GC_getStatistics", "GC_ref",
		"GC_setStrategy", "GC_unref", "InstatiationInfo", "IsOnStack", "Open",
		"ReadBytes", "ReadChars", "abs", "accumulateResult", "add",
		"addQuitProc", "alloc", "alloc0", "allocCStringArray", "allocShared",
		"allocShared0", "assert", "astToStr", "atomicDec", "atomicInc", "card",
		"chr", "clamp", "cmp", "compareAndSwap", "compileOption", "compiles",
		"contains", "copyMem", "countdown", "countup", "createThread",
		"cstringArrayToSeq", "dealloc", "deallocCStringArray", "deallocShared",
		"debugEcho", "dec", "defined", "definedInScope", "del", "delete",
		"doAssert", "each", "echo", "equalMem", "eval", "excl", "fieldPairs",
		"fields", "fileHandle", "find", "finished", "getCurrentException",
		"getCurrentExceptionMsg", "getFilePos", "getFileSize", "getFreeMem",
		"getOccupiedMem", "getRefcount", "getStackTrace", "getTotalMem",
		"getTypeInfo", "gorge", "high", "inc", "incl", "insert", "internalNew",
		"isNil", "items", "joinThread", "joinThreads", "len", "likely", "lines",
		"locals", "low", "map", "max", "min", "moveMem", "myThreadId", "new",
		"newException", "newSeq", "newString", "newStringOfCap",
		"nimDestroyRange", "onFailedAssert", "onRaise", "ord", "pairs", "peek",
		"pop", "pred", "quit", "raiseAssert", "rand", "rawEnv", "rawProc",
		"readAll", "readBuffer", "readChar", "readFile", "readLine", "ready",
		"realloc", "reallocShared", "recv", "repopen", "repr", "reset",
		"running", "safeAdd", "send", "setControlCHook", "setFilePos", "setLen",
		"shallow", "shallowCopy", "sizeof", "slurp", "staticExec", "staticRead",
		"substr", "succ", "swap", "threadId", "toBiggestFloat", "toBiggestInt",
		"toFloat", "toInt", "toU16", "toU32", "toU8", "unlikely", "unsafeNew",
		"write", "writeBuffer", "writeBytes", "writeChars", "writeFile",
		"writeStackTrace", "writeln", "ze", "ze64", "zeroMem"),

	.line_comments = { "#" },
	.block_comments = { { "#[", "]#" } },
	.long_strings = { { "\"\"\"", "\"\"\"" } },
	.quotes = "'\"",
};
