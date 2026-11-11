/* Protobuf */
#include "../lang_def.h"

LANGUAGE(protobuf) {
	.name = "protobuf",
	.title = "Protobuf",
	.extensions = "proto",

	.keywords = WORDS(
		"contained", "default", "extend", "extensions", "group", "import",
		"max", "message", "oneof", "option", "optional", "package", "repeated",
		"required", "reserved", "returns", "rpc", "service", "syntax", "to"),

	.types = WORDS(
		"bool", "bytes", "double", "enum", "false", "fixed32", "fixed64",
		"float", "int32", "int64", "sfixed32", "sfixed64", "sint32", "sint64",
		"string", "true", "uint32", "uint64"),

	.line_comments = { "//" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "'\"",
};
