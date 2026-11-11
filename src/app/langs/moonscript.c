/* MoonScript */
#include "../lang_def.h"

LANGUAGE(moonscript) {
	.name = "moonscript",
	.title = "MoonScript",
	.flags = F_LUALONG,
	.extensions = "moon",

	.keywords = WORDS(
		"and", "break", "class", "continue", "do", "else", "elseif", "export",
		"extends", "false", "for", "from", "if", "import", "in", "local", "nil",
		"not", "or", "return", "super", "switch", "then", "true", "unless",
		"using", "when", "while", "with"),

	.constants = WORDS(
		"_ENV", "_G", "_VERSION"),

	.functions = WORDS(
		"assert", "bind_methods", "collectgarbage", "copy", "coroutine",
		"coroutine.create", "coroutine.isyieldable", "coroutine.resume",
		"coroutine.running", "coroutine.status", "coroutine.wrap",
		"coroutine.yield", "debug", "debug.debug", "debug.gethook",
		"debug.getinfo", "debug.getlocal", "debug.getmetatable",
		"debug.getregistry", "debug.getupvalue", "debug.getuservalue",
		"debug.sethook", "debug.setlocal", "debug.setmetatable",
		"debug.setupvalue", "debug.setuservalue", "debug.traceback",
		"debug.upvalue", "debug.upvalueid", "debug.upvaluejoin", "defaultbl",
		"dofile", "error", "extend", "fold", "getmetatable", "io", "io.close",
		"io.flush", "io.input", "io.lines", "io.open", "io.output", "io.popen",
		"io.read", "io.stderr", "io.stdin", "io.stdout", "io.tmpfile",
		"io.type", "io.write", "ipairs", "is_object", "load", "loadfile",
		"math", "math.abs", "math.acos", "math.asin", "math.atan", "math.ceil",
		"math.cos", "math.deg", "math.exp", "math.floor", "math.fmod",
		"math.huge", "math.log", "math.max", "math.maxinteger", "math.min",
		"math.mininteger", "math.modf", "math.pi", "math.rad", "math.random",
		"math.randomseed", "math.sin", "math.sqrt", "math.tan",
		"math.tointeger", "math.type", "math.ult", "mixin", "mixin_object",
		"mixin_table", "next", "os", "os.clock", "os.date", "os.difftime",
		"os.execute", "os.exit", "os.getenv", "os.remove", "os.rename",
		"os.setlocale", "os.time", "os.tmpname", "p", "package",
		"package.config", "package.cpath", "package.loaded", "package.loadlib",
		"package.path", "package.preload", "package.searchers",
		"package.searchpath", "pairs", "pcall", "print", "rawequal", "rawget",
		"rawlen", "rawset", "require", "run_with_scope", "select",
		"setmetatable", "string", "string.byte", "string.char", "string.dump",
		"string.find", "string.format", "string.gmatch", "string.gsub",
		"string.len", "string.lower", "string.match", "string.pack",
		"string.packsize", "string.rep", "string.reverse", "string.sub",
		"string.unpack", "string.upper", "table", "table.concat",
		"table.insert", "table.move", "table.pack", "table.remove",
		"table.sort", "table.unpack", "tonumber", "tostring", "type", "utf8",
		"utf8.char", "utf8.charpattern", "utf8.codepoint", "utf8.codes",
		"utf8.len", "utf8.offset", "xpcall"),

	.line_comments = { "--" },
	.quotes = "'\"",
};
