/* PowerShell */
#include "../lang_def.h"

LANGUAGE(powershell) {
	.name = "powershell",
	.title = "PowerShell",
	.flags = F_NOCASE | F_VARS,
	.extensions = "psm1 ps1",
	.interpreters = "pwsh powershell",

	.keywords = WORDS(
		"Begin", "Break", "Confirm", "Continue", "Debug", "Do", "Else", "End",
		"ErrorAction", "ErrorVariable", "Exit", "For", "ForEach",
		"ForEach-Object", "Get-Date", "Get-Random", "If", "OutBuffer",
		"OutVariable", "Param", "Pause", "Powershell", "Process", "Read-Host",
		"Return", "Switch", "Verbose", "WhatIf", "While", "Write-Host", "and",
		"array", "as", "band", "boolean", "bor", "byte", "char", "contains",
		"datetime", "day", "dayofweek", "dayofyear", "decimal", "double", "eq",
		"ge", "gt", "hashtable", "hour", "int", "is", "isnot", "le", "like",
		"long", "lt", "match", "millisecond", "minute", "month", "ne",
		"nomatch", "not", "notcontains", "notlike", "or", "replace", "second",
		"single", "string", "timeofday", "xml", "year"),

	.line_comments = { "#" },
	.quotes = "\"",

	.blocks = BLOCKS(
		OP_BLOCK("{", "}")),
};
