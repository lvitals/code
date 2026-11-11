/* TypeScript */
#include "../lang_def.h"

LANGUAGE(typescript) {
	.name = "typescript",
	.title = "TypeScript",
	.flags = F_BACKTICK | F_ATTR,
	.extensions = "tsx ts mts cts",
	.interpreters = "deno",

	.keywords = WORDS(
		"abstract", "as", "async", "await", "boolean", "break", "byte", "case",
		"catch", "char", "class", "const", "constructor", "continue",
		"debugger", "declare", "default", "delete", "do", "double", "else",
		"enum", "export", "extends", "false", "final", "finally", "float",
		"for", "function", "get", "goto", "if", "implements", "import", "in",
		"instanceof", "int", "interface", "is", "let", "long", "module",
		"namespace", "native", "new", "null", "of", "package", "private",
		"protected", "public", "require", "return", "set", "short", "static",
		"super", "switch", "synchronized", "this", "throw", "throws",
		"transient", "true", "try", "type", "typeof", "var", "void", "volatile",
		"while", "with", "yield"),

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
		"Uint8ClampedArray", "WeakMap", "WeakSet", "WebAssembly", "any",
		"bigint", "boolean", "never", "number", "object", "string", "symbol",
		"unknown", "void"),

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
