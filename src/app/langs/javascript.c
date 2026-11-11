/* JavaScript */
#include "../lang_def.h"

LANGUAGE(javascript) {
	.name = "javascript",
	.title = "JavaScript",
	.flags = F_BACKTICK,
	.extensions = "cjs jsx mjs js jsfl",
	.interpreters = "node nodejs",

	.keywords = WORDS(
		"abstract", "async", "await", "boolean", "break", "byte", "case",
		"catch", "char", "class", "const", "continue", "debugger", "default",
		"delete", "do", "double", "else", "enum", "export", "extends", "false",
		"final", "finally", "float", "for", "function", "get", "goto", "if",
		"implements", "import", "in", "instanceof", "int", "interface", "let",
		"long", "native", "new", "null", "of", "package", "private",
		"protected", "public", "return", "set", "short", "static", "super",
		"switch", "synchronized", "this", "throw", "throws", "transient",
		"true", "try", "typeof", "var", "void", "volatile", "while", "with",
		"yield"),

	.types = WORDS(
		"AggregateError", "Array", "ArrayBuffer", "AsyncFunction",
		"AsyncGenerator", "AsyncGeneratorFunction", "Atomics", "BigInt",
		"BigInt64Array", "BigUint64Array", "Boolean", "DataView", "Date",
		"Error", "EvalError", "Float32Array", "Float64Array", "Function",
		"Generator", "GeneratorFunction", "Int16Array", "Int32Array",
		"Int8Array", "InternalError", "Intl", "JSON", "Map", "Math", "Number",
		"Object", "Promise", "Proxy", "RangeError", "ReferenceError", "Reflect",
		"RegExp", "Set", "SharedArrayBuffer", "String", "Symbol", "SyntaxError",
		"TypeError", "URIError", "Uint16Array", "Uint32Array", "Uint8Array",
		"Uint8ClampedArray", "WeakMap", "WeakSet", "WebAssembly"),

	.constants = WORDS(
		"Infinity", "NaN", "arguments", "globalThis", "undefined"),

	.functions = WORDS(
		"decodeURI", "decodeURIComponent", "encodeURI", "encodeURIComponent",
		"eval", "isFinite", "isNaN", "parseFloat", "parseInt"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
