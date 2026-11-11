/* Visual Basic */
#include "../lang_def.h"

LANGUAGE(vb) {
	.name = "vb",
	.title = "Visual Basic",
	.extensions = "bas cls ctl dob dsm dsr frm pag vb vba vbs",

	.keywords = WORDS(
		"And", "Call", "Case", "Class", "Const", "Continue", "Default", "Dim",
		"Do", "Each", "Else", "ElseIf", "Empty", "End", "Error", "Execute",
		"Exit", "Explicit", "False", "For", "Function", "Get", "If", "In", "Is",
		"Let", "Loop", "Mod", "Module", "New", "Next", "Not", "Nothing", "Null",
		"On", "Option", "Or", "Preserve", "Private", "Property", "Public",
		"Randomize", "ReDim", "Return", "Select", "Set", "Step", "Sub", "Then",
		"To", "True", "Until", "Wend", "While", "With", "Xor"),

	.types = WORDS(
		"Boolean", "Byte", "Char", "Date", "Decimal", "Double", "Long",
		"Object", "Short", "Single", "String"),

	.line_comments = { "'", "rem" },
	.quotes = "\"",

	.blocks = BLOCKS(
		WORD_BLOCK("If", "End If"),
		WORD_BLOCK("Select", "End Select"),
		WORD_BLOCK("For", "Next"),
		WORD_BLOCK("While", "End While"),
		WORD_BLOCK("While", "Wend"),
		WORD_BLOCK("Do", "Loop"),
		WORD_BLOCK("With", "End With"),
		WORD_BLOCK("Sub", "End Sub"),
		WORD_BLOCK("Function", "End Function"),
		WORD_BLOCK("Property", "End Property"),
		WORD_BLOCK("Module", "End Module"),
		WORD_BLOCK("Class", "End Class"),
		WORD_BLOCK("Try", "End Try")),
};
