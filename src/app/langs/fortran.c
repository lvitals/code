/* Fortran */
#include "../lang_def.h"

LANGUAGE(fortran) {
	.name = "fortran",
	.title = "Fortran",
	.flags = F_NOCASE,
	.extensions = "f for ftn fpp f77 f90 f95 f03 f08",

	.keywords = WORDS(
		".and.", ".eq.", ".eqv.", ".false.", ".ge.", ".gt.", ".le.", ".lt.",
		".ne.", ".neqv.", ".not.", ".or.", ".true.", ".xor.", "access",
		"action", "advance", "assignment", "block", "call", "case", "contains",
		"continue", "cycle", "default", "do", "else", "elseif", "elsewhere",
		"end", "enddo", "endif", "entry", "equivalence", "exit", "external",
		"forall", "function", "goto", "if", "in", "include", "inout", "intent",
		"interface", "module", "only", "optional", "out", "pause", "pointer",
		"precision", "procedure", "program", "recursive", "result", "return",
		"save", "select", "sequence", "size", "stat", "stop", "subroutine",
		"target", "then", "type", "use", "where", "while"),

	.types = WORDS(
		"allocatable", "allocate", "allocated", "bind", "c_bool", "c_char",
		"c_double", "c_double_complex", "c_float", "c_float128",
		"c_float128_complex", "c_float_complex", "c_funptr", "c_int",
		"c_int128_t", "c_int16_t", "c_int32_t", "c_int64_t", "c_int8_t",
		"c_intptr_t", "c_long", "c_long_double", "c_long_double_complex",
		"c_long_long", "c_new_line", "c_null_char", "c_null_ptr", "c_short",
		"c_signed_char", "c_size_t", "character", "complex", "data",
		"deallocate", "dimension", "double", "explicit", "implicit", "integer",
		"kind", "logical", "none", "parameter", "precision", "real"),

	.functions = WORDS(
		"abs", "acos", "aimag", "aint", "alog", "alog10", "amax0", "amax1",
		"amin0", "amin1", "amod", "anint", "asin", "assign", "atan", "atan2",
		"backspace", "c_associated", "c_f_pointer", "c_sizeof", "cabs", "ccos",
		"ceiling", "char", "clog", "close", "cmplx", "conjg", "cos", "cosh",
		"csin", "csqrt", "dabs", "dacos", "dasin", "datan", "datan2", "dble",
		"dcmplx", "dcos", "dcosh", "ddim", "dexp", "dfloat", "dim", "dint",
		"dlog", "dlog10", "dmax1", "dmin1", "dmod", "dnint", "dprod", "dreal",
		"dsign", "dsin", "dsinh", "dsqrt", "dtan", "dtanh", "endfile", "exp",
		"float", "floor", "format", "iabs", "ichar", "idim", "idint", "idnint",
		"ifix", "imag", "index", "inquire", "int", "isign", "len", "lge", "lgt",
		"lle", "llt", "log", "log10", "logical", "matmul", "max", "max0",
		"max1", "min", "min0", "min1", "mod", "modulo", "nint", "nullify",
		"open", "print", "read", "real", "reshape", "rewind", "sign", "sin",
		"sinh", "sngl", "sqrt", "tan", "tanh", "transfer", "transpose", "write",
		"zext"),

	.line_comments = { "!" },
	.column_comments = { COLUMN(1, "CcDd!*") },
	.quotes = "'\"",
};
