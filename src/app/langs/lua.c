/* Lua */
#include "../lang_def.h"

LANGUAGE(lua) {
	.name = "lua",
	.title = "Lua",
	.flags = F_LUALONG,
	.extensions = "lua luau",
	.interpreters = "lua luajit",

	.keywords = WORDS(
		"and", "break", "do", "else", "elseif", "end", "false", "for",
		"function", "goto", "if", "in", "local", "nil", "not", "or", "repeat",
		"return", "then", "true", "until", "while"),

	.constants = WORDS(
		"_ENV", "_G", "_VERSION", "coroutine", "io", "io.stderr", "io.stdin",
		"io.stdout", "math", "math.huge", "math.maxinteger", "math.mininteger",
		"math.pi", "os", "package", "package.config", "package.cpath",
		"package.loaded", "package.path", "package.preload",
		"package.searchers", "string", "table", "utf8", "utf8.charpattern"),

	.functions = WORDS(
		"assert", "collectgarbage", "coroutine.close", "coroutine.create",
		"coroutine.isyieldable", "coroutine.resume", "coroutine.running",
		"coroutine.status", "coroutine.wrap", "coroutine.yield", "debug",
		"debug.debug", "debug.gethook", "debug.getinfo", "debug.getlocal",
		"debug.getmetatable", "debug.getregistry", "debug.getupvalue",
		"debug.getuservalue", "debug.sethook", "debug.setlocal",
		"debug.setmetatable", "debug.setupvalue", "debug.setuservalue",
		"debug.traceback", "debug.upvalueid", "debug.upvaluejoin", "dofile",
		"error", "getmetatable", "io.close", "io.flush", "io.input", "io.lines",
		"io.open", "io.output", "io.popen", "io.read", "io.tmpfile", "io.type",
		"io.write", "ipairs", "load", "loadfile", "math.abs", "math.acos",
		"math.asin", "math.atan", "math.ceil", "math.cos", "math.deg",
		"math.exp", "math.floor", "math.fmod", "math.log", "math.max",
		"math.min", "math.modf", "math.rad", "math.random", "math.randomseed",
		"math.sin", "math.sqrt", "math.tan", "math.tointeger", "math.type",
		"math.ult", "next", "os.clock", "os.date", "os.difftime", "os.execute",
		"os.exit", "os.getenv", "os.remove", "os.rename", "os.setlocale",
		"os.time", "os.tmpname", "package.loadlib", "package.searchpath",
		"pairs", "pcall", "print", "rawequal", "rawget", "rawlen", "rawset",
		"require", "select", "setmetatable", "string.byte", "string.char",
		"string.dump", "string.find", "string.format", "string.gmatch",
		"string.gsub", "string.len", "string.lower", "string.match",
		"string.pack", "string.packsize", "string.rep", "string.reverse",
		"string.sub", "string.unpack", "string.upper", "table.concat",
		"table.insert", "table.move", "table.pack", "table.remove",
		"table.sort", "table.unpack", "tonumber", "tostring", "type",
		"utf8.char", "utf8.codepoint", "utf8.codes", "utf8.len", "utf8.offset",
		"warn", "xpcall"),

	.preproc = WORDS(
		"close", "const"),

	.line_comments = { "--" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("if", "end"),
		WORD_BLOCK("do", "end"),
		WORD_BLOCK("function", "end"),
		WORD_BLOCK("repeat", "until"),
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
