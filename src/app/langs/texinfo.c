/* Texinfo */
#include "../lang_def.h"

LANGUAGE(texinfo) {
	.name = "texinfo",
	.title = "Texinfo",
	.extensions = "texi",

	.keywords = WORDS(
		"!", "*", "-", ".", "/", ":", "?", "@", "LaTeX", "TeX", "abbr",
		"acronym", "anchor", "arrow", "asis", "author", "b", "backslashcar",
		"bullet", "center", "cindex", "cite", "click", "clicksequence",
		"clickstyle", "code", "comma", "command", "contents", "copyright",
		"defcodeindex", "detailmenu", "dfn", "dircategory", "direntry",
		"display", "dmn", "documentdescription", "documentencoding",
		"documentlanguage", "dots", "email", "end", "enddots", "env", "equiv",
		"error", "euro", "exdent", "expansion", "file", "findex", "flushleft",
		"flushright", "format", "frenchspacing", "ftable", "geq", "group",
		"guillementright", "guillemetleft", "guillemotleft", "guillemotright",
		"hashcar", "headitem", "headitemfont", "hyphenation", "i", "indent",
		"indentedblock", "indicateurl", "inforef", "insertcopying", "item",
		"itemx", "kbd", "key", "kindex", "leq", "lisp", "math", "menu", "minus",
		"need", "node", "noindent", "novalidate", "option", "page", "pindex",
		"point", "pounds", "print", "printindex", "pxref", "quotation", "r",
		"raggedright", "ref", "refill", "registeredsymbol", "result", "samp",
		"sansserif", "sc", "setcontentsaftertitlepage", "setfilename",
		"setshortcontentsaftertitlepage", "settitle", "shortcontents",
		"shorttitlepage", "slanted", "smalldisplay", "smallformat",
		"smallindentedblock", "smalllisp", "smallquotation", "sp", "subtitle",
		"summarycontents", "syncodeindex", "synindex", "t", "tab", "table",
		"textdegree", "tie", "tindex", "title", "titlefont", "today", "top",
		"url", "var", "verb", "verbatim", "vindex", "vskip", "vtable", "w",
		"xref", "xrefautomaticsectiontitle", "{", "}"),

	.functions = WORDS(
		"appendix", "appendixsec", "appendixsection", "appendixsubsec",
		"appendixsubsection", "appendixsubsubsec", "appendixsubsubsection",
		"centerchap", "chapheading", "chapter", "heading", "lowersections",
		"majorheading", "part", "raisesections", "section", "subheading",
		"subsection", "subsubheading", "subsubsection", "unnumbered",
		"unnumberedsec", "unnumberedsubsec", "unnumberedsubsection",
		"unnumberedsubsubsec", "unnumberedsubsubsection"),

	.preproc = WORDS(
		"afivepaper", "afourlatex", "afourpaper", "afourwide", "alias",
		"allowcodebreaks", "bye", "caption", "cartouche", "clear", "copying",
		"definfounclose", "docbook", "end", "endchapter", "enumerate",
		"example", "finalout", "float", "fonttextsize", "footnote",
		"footnotestyle", "html", "ifclear", "ifcommanddefined",
		"ifcommandnotdefined", "ifdocbook", "ifhtml", "ifinfo", "ifnotdocbook",
		"ifnothtml", "ifnotinfo", "ifnotplaintext", "ifnottex", "ifnotxml",
		"ifplaintext", "ifset", "iftex", "ifxml", "image", "include",
		"inlinefmt", "inlinefmtifelse", "inlineifclear", "inlineifset",
		"inlineraw", "itemize", "listoffloats", "macro", "multitable",
		"pagesizes", "set", "shortcaption", "smallbook", "smallexample",
		"startchapter", "tex", "titlepage", "unmacro", "value",
		"verbatiminclude", "xml"),

	.line_comments = { "@c" },
};
