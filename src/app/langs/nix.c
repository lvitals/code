/* Nix */
#include "../lang_def.h"

LANGUAGE(nix) {
	.name = "nix",
	.title = "Nix",
	.extensions = "nix",

	.keywords = WORDS(
		"...", "assert", "else", "if", "in", "inherit", "let", "or", "rec",
		"then", "with"),

	.constants = WORDS(
		"builtins", "builtins.currentSystem", "builtins.currentTime",
		"builtins.langVersion", "builtins.nixPath", "builtins.nixVersion",
		"builtins.storeDir", "false", "null", "true"),

	.functions = WORDS(
		"abort", "builtins.add", "builtins.all", "builtins.any",
		"builtins.attrNames", "builtins.attrValues", "builtins.baseNameOf",
		"builtins.bitAnd", "builtins.bitOr", "builtins.bitXor",
		"builtins.break", "builtins.catAttrs", "builtins.ceil",
		"builtins.compareVersions", "builtins.concatLists",
		"builtins.concatMap", "builtins.concatStringsSep", "builtins.deepSeq",
		"builtins.dirOf", "builtins.div", "builtins.elem", "builtins.elemAt",
		"builtins.fetchClosure", "builtins.fetchGit", "builtins.fetchTarball",
		"builtins.fetchurl", "builtins.filter", "builtins.filterSource",
		"builtins.findFile", "builtins.flakeRefToString", "builtins.floor",
		"builtins.foldl'", "builtins.fromJSON", "builtins.fromTOML",
		"builtins.functionArgs", "builtins.genList", "builtins.genericClosure",
		"builtins.getAttr", "builtins.getContext", "builtins.getEnv",
		"builtins.getFlake", "builtins.groupBy", "builtins.hasAttr",
		"builtins.hasContext", "builtins.hashFile", "builtins.hashString",
		"builtins.head", "builtins.import", "builtins.intersectAttrs",
		"builtins.isAttrs", "builtins.isBool", "builtins.isFloat",
		"builtins.isFunction", "builtins.isInt", "builtins.isList",
		"builtins.isNull", "builtins.isPath", "builtins.isString",
		"builtins.length", "builtins.lessThan", "builtins.listToAttrs",
		"builtins.map", "builtins.mapAttrs", "builtins.match", "builtins.mul",
		"builtins.outputOf", "builtins.parseDrvName", "builtins.parseFlakeRef",
		"builtins.partition", "builtins.path", "builtins.pathExists",
		"builtins.placeholder", "builtins.readDir", "builtins.readFile",
		"builtins.readFileType", "builtins.removeAttrs",
		"builtins.replaceStrings", "builtins.seq", "builtins.sort",
		"builtins.split", "builtins.splitVersion", "builtins.storePath",
		"builtins.stringLength", "builtins.sub", "builtins.substring",
		"builtins.tail", "builtins.throw", "builtins.toFile", "builtins.toJSON",
		"builtins.toPath", "builtins.toString", "builtins.toXML",
		"builtins.trace", "builtins.traceVerbose", "builtins.tryEval",
		"builtins.typeOf", "builtins.zipAttrsWith", "derivation", "import",
		"throw"),

	.line_comments = { "#" },
	.block_comments = { { "/" "*", "*" "/" } },
	.quotes = "\"",

	.blocks = BLOCKS(
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
