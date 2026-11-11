/* Org */
#include "../lang_def.h"

LANGUAGE(org) {
	.name = "org",
	.title = "Org",
	.extensions = "org",

	.constants = WORDS(
		"datetime"),

	.functions = WORDS(
		"April", "August", "Březen", "DELEGATED", "DONE", "December",
		"Dezember", "Di", "Dienstag", "Do", "Donnerstag", "Duben", "Februar",
		"February", "Fr", "Freitag", "Fri", "Friday", "INVALID", "Januar",
		"January", "Juli", "July", "June", "Juni", "Květen", "Leden",
		"Listopad", "Mai", "March", "May", "Mi", "Mittwoch", "Mo", "Mon",
		"Monday", "Montag", "März", "Ne", "Neděle", "November", "October",
		"Oktober", "Po", "Pondělí", "Prosince", "Pá", "Pátek", "Sa",
		"Samstag", "Sat", "Saturday", "September", "So", "Sobota", "Sonntag",
		"Srpen", "St", "Středa", "Sun", "Sunday", "TODO", "Thu", "Thursday",
		"Tue", "Tuesday", "WAITING", "WONTFIX", "Wed", "Wednesday", "Září",
		"Únor", "Út", "Úterý", "Červen", "Červenec", "Čt", "Čtvrtek",
		"Říjen"),

	.line_comments = { "# " },
	.quotes = "\"",
};
