/* Lisp */
#include "../lang_def.h"

LANGUAGE(lisp) {
	.name = "lisp",
	.title = "Lisp",
	.flags = F_DASH,
	.extensions = "cl el lisp lsp",

	.keywords = WORDS(
		"abort", "assert", "block", "break", "case", "catch", "ccase", "cerror",
		"cond", "ctypecase", "declaim", "declare", "defclass", "defconstant",
		"defgeneric", "define-compiler-macro", "define-condition",
		"define-method-combination", "define-modify-macro",
		"define-setf-expander", "define-symbol-macro", "defmacro", "defmethod",
		"defpackage", "defparameter", "defsetf", "defstruct", "deftype",
		"defun", "defvar", "do", "do*", "do-all-symbols", "do-external-symbols",
		"do-symbols", "dolist", "dotimes", "ecase", "error", "etypecase",
		"eval-when", "flet", "handler-bind", "handler-case", "if",
		"ignore-errors", "in-package", "labels", "lambda", "let", "let*",
		"locally", "loop", "macrolet", "multiple-value-bind", "nil", "proclaim",
		"prog", "prog*", "prog1", "prog2", "progn", "progv", "provide",
		"require", "restart-bind", "restart-case", "restart-name", "return",
		"return-from", "signal", "symbol-macrolet", "t", "tagbody", "the",
		"throw", "typecase", "unless", "unwind-protect", "when",
		"with-accessors", "with-compilation-unit", "with-condition-restarts",
		"with-hash-table-iterator", "with-input-from-string", "with-open-file",
		"with-open-stream", "with-output-to-string", "with-package-iterator",
		"with-simple-restart", "with-slots", "with-standard-io-syntax"),

	.line_comments = { ";" },
	.block_comments = { { "#|", "|#" } },
	.quotes = "\"",

	.blocks = BLOCKS(
		OP_BLOCK("(", ")"),
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
