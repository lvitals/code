/* The settings: the options of the View menu and of Find, kept from one
 * run to the next in a JSON file of the platform (plat_config):
 *
 *   {
 *     "word_wrap": false,
 *     "auto_indent": true,
 *     ...
 *   }
 *
 * Besides the options, three numbers: "tab_width" (1 to 8) and
 * "expand_tabs" (Tab puts in spaces) say how a text that has no
 * indentation yet is indented (a file that has one keeps it);
 * "ruler_column" marks a column (80: the one after the 80th; 0: none);
 * "font_size" is the window's zoom.  One is a string: "font", the font
 * of a window, a file (.ttf, .otf, .ttc) or a family in the font folders
 * ("JetBrains Mono", "JetBrains Mono:Bold"); "" is the one in the program.
 *
 * All of them are written, each time one changes.  What is written is
 * checked first: the text made is parsed as JSON and each value read
 * back must be the one meant; after the save the file is read and
 * checked again.  A file that is not valid JSON is not taken (the
 * defaults are) and the editor says so; a name it does not know, or a
 * value that is not true or false, is left aside.
 *
 * File > Preferences opens the file itself, as a document (made with
 * the values first, if there is none); when it is saved the settings are
 * read from it again and given to the documents open.  A new document
 * takes the settings; turning an option on or off (the View menu, Find)
 * changes it there, and in the file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "../core/mem.h"

static char expand;		/* Tab puts in spaces (a text not indented yet) */

static const struct {
	const char *name, *label;
	int action;		/* of a document, or 0: */
	char *global;		/* one of Find */
} names[] = {
	{ "word_wrap", "Word Wrap", A_WRAP, 0 },
	{ "auto_indent", "Auto Indent", A_AUTOINDENT, 0 },
	{ "show_tabs", "Show Tabs", A_SHOWTABS, 0 },
	{ "show_line_ends", "Show Line Ends", A_SHOWEOL, 0 },
	{ "show_end_of_file", "Show End of File", A_SHOWEOF, 0 },
	{ "line_numbers", "Line Numbers", A_LINENUMS, 0 },
	{ "relative_line_numbers", "Relative Line Numbers", A_RELNUMS, 0 },
	{ "highlight_current_line", "Highlight Current Line", A_CURLINE, 0 },
	{ "syntax_highlighting", "Syntax Highlighting", A_SYNTAX, 0 },
	{ "match_brackets", "Highlight Matching Brackets", A_MATCHBR, 0 },
	{ "backspace_unindents", "Backspace Removes a Whole Indent", A_BSINDENT, 0 },
	{ "indent_guides", "Show Indent Guides", A_GUIDES, 0 },
	{ "find_match_case", "Find: Match Case", 0, &st.mcase },
	{ "find_whole_word", "Find: Whole Word", 0, &st.wword },
	{ "find_use_regex", "Find: Use Regex", 0, &st.regex },
	{ "show_hidden_files", "Open, Save As: Hidden Files", 0, &st.phidden },
	{ "expand_tabs", "Tab Inserts Spaces", 0, &expand },
};
#define NNAMES	((int)(sizeof names / sizeof names[0]))
#define JSON_MAX	8192

/* the values, as a new document takes them */
static char value[NNAMES] = { 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1 };

/* the option of action a in document d */
static char *
field(struct doc *d, int a)
{
	switch(a) {
	case A_WRAP: return &d->wrap;
	case A_AUTOINDENT: return &d->autoindent;
	case A_SHOWTABS: return &d->showtabs;
	case A_SHOWEOL: return &d->showeol;
	case A_SHOWEOF: return &d->showeof;
	case A_LINENUMS: return &d->linenums;
	case A_RELNUMS: return &d->relnums;
	case A_CURLINE: return &d->curline;
	case A_SYNTAX: return &d->syntax;
	case A_MATCHBR: return &d->matchbr;
	case A_BSINDENT: return &d->bsindent;
	case A_GUIDES: return &d->guides;
	}
	return 0;
}

/* --- JSON (RFC 8259): a parser that checks all of it ---------------------- */

struct json {
	const char *p, *e;
	int depth;
	/* each member of the top object that is true or false, a whole
	   number or a string: to here */
	void (*member)(const char *name, int len, int v);
	void (*number)(const char *name, int len, long v);
	void (*string)(const char *name, int len, const char *s, int n);
	long num;	/* the last number, when whole */
	const char *str;	/* the last string, as written */
	int slen;
	char isnum, isstr;	/* the last value was one */
};

static void
ws(struct json *j)
{
	while(j->p < j->e && (*j->p == ' ' || *j->p == '\t' || *j->p == '\n' || *j->p == '\r'))
		j->p++;
}

static int
hex4(const char *p)
{
	int i, c;

	for(i = 0; i < 4; i++) {
		c = p[i];
		if(!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
			return 0;
	}
	return 1;
}

/* a string: its contents (as written, escapes kept) at *s, *n */
static int
jstring(struct json *j, const char **s, int *n)
{
	unsigned char c;

	if(j->p >= j->e || *j->p != '"')
		return 0;
	*s = ++j->p;
	while(j->p < j->e && *j->p != '"') {
		c = (unsigned char)*j->p;
		if(c < 0x20)
			return 0;	/* a control character, unescaped */
		if(c == '\\') {
			if(++j->p >= j->e)
				return 0;
			c = (unsigned char)*j->p;
			if(c == 'u') {
				if(j->e - j->p < 5 || !hex4(j->p + 1))
					return 0;
				j->p += 4;
			} else if(!strchr("\"\\/bfnrt", c))
				return 0;
		}
		j->p++;
	}
	if(j->p >= j->e)
		return 0;
	*n = (int)(j->p - *s);
	j->p++;
	return 1;
}

static int
digits(struct json *j)
{
	const char *s = j->p;

	while(j->p < j->e && *j->p >= '0' && *j->p <= '9')
		j->p++;
	return j->p > s;
}

static int jnumber_syntax(struct json *j);

static int
jnumber(struct json *j)
{
	const char *s = j->p;
	int ok;

	ok = jnumber_syntax(j);
	j->isnum = 0;
	if(ok && j->p - s < 10) {	/* a whole number, small: its value */
		const char *p = s;
		long v = 0;
		int neg = *p == '-';

		for(p += neg; p < j->p && *p >= '0' && *p <= '9'; p++)
			v = v * 10 + (*p - '0');
		if(p == j->p) {
			j->num = neg ? -v : v;
			j->isnum = 1;
		}
	}
	return ok;
}

static int
jnumber_syntax(struct json *j)
{
	if(j->p < j->e && *j->p == '-')
		j->p++;
	if(j->p < j->e && *j->p == '0')
		j->p++;
	else if(!digits(j))
		return 0;
	if(j->p < j->e && *j->p == '.') {
		j->p++;
		if(!digits(j))
			return 0;
	}
	if(j->p < j->e && (*j->p == 'e' || *j->p == 'E')) {
		j->p++;
		if(j->p < j->e && (*j->p == '+' || *j->p == '-'))
			j->p++;
		if(!digits(j))
			return 0;
	}
	return 1;
}

static int
word(struct json *j, const char *w)
{
	size_t n = strlen(w);

	if((size_t)(j->e - j->p) < n || memcmp(j->p, w, n) != 0)
		return 0;
	j->p += n;
	return 1;
}

/* a value: -1 not JSON, else 0, or 1 + its truth for true and false */
static int
jvalue(struct json *j)
{
	const char *s, *name;
	int n, len, v, top;

	ws(j);
	if(j->p >= j->e || ++j->depth > 64)
		return -1;
	j->isnum = j->isstr = 0;
	switch(*j->p) {
	case '{':
		top = j->depth == 1;
		j->p++;
		ws(j);
		if(j->p < j->e && *j->p == '}') {
			j->p++;
			break;
		}
		for(;;) {
			ws(j);
			if(!jstring(j, &name, &len))
				return -1;
			ws(j);
			if(j->p >= j->e || *j->p++ != ':')
				return -1;
			if((v = jvalue(j)) < 0)
				return -1;
			if(top && v == 0 && j->isnum && j->number)
				j->number(name, len, j->num);
			if(top && v == 0 && j->isstr && j->string)
				j->string(name, len, j->str, j->slen);
			if(top && v > 0 && j->member)
				j->member(name, len, v - 1);
			ws(j);
			if(j->p < j->e && *j->p == ',') {
				j->p++;
				continue;
			}
			if(j->p < j->e && *j->p == '}') {
				j->p++;
				break;
			}
			return -1;
		}
		break;
	case '[':
		j->p++;
		ws(j);
		if(j->p < j->e && *j->p == ']') {
			j->p++;
			break;
		}
		for(;;) {
			if(jvalue(j) < 0)
				return -1;
			ws(j);
			if(j->p < j->e && *j->p == ',') {
				j->p++;
				continue;
			}
			if(j->p < j->e && *j->p == ']') {
				j->p++;
				break;
			}
			return -1;
		}
		break;
	case '"':
		if(!jstring(j, &s, &n))
			return -1;
		j->depth--;
		j->str = s;
		j->slen = n;
		j->isstr = 1;
		return 0;
	case 't':
		if(!word(j, "true"))
			return -1;
		j->depth--;
		return 2;
	case 'f':
		if(!word(j, "false"))
			return -1;
		j->depth--;
		return 1;
	case 'n':
		if(!word(j, "null"))
			return -1;
		break;
	default:
		if(!jnumber(j))
			return -1;
		j->depth--;
		return 0;	/* isnum: its value, if whole */
	}
	j->depth--;
	j->isnum = j->isstr = 0;	/* an object, an array, null */
	return 0;
}

/* the UTF-8 of a JSON string [s, s + n) (as written): its length, or -1
 * if longer than max - 1 */
static int
junescape(const char *s, int n, char *out, int max)
{
	const char *e = s + n;
	unsigned long c, d;
	int o = 0;

	while(s < e) {
		c = (unsigned char)*s++;
		if(c == '\\') {
			c = (unsigned char)*s++;
			switch(c) {
			case 'b': c = '\b'; break;
			case 'f': c = '\f'; break;
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
			case 'u':
				c = strtoul((char[5]){ s[0], s[1], s[2], s[3], 0 }, 0, 16);
				s += 4;
				if(c >= 0xD800 && c < 0xDC00 && e - s >= 6 && s[0] == '\\' && s[1] == 'u') {
					d = strtoul((char[5]){ s[2], s[3], s[4], s[5], 0 }, 0, 16);
					if(d >= 0xDC00 && d < 0xE000) {
						c = 0x10000 + ((c - 0xD800) << 10) + (d - 0xDC00);
						s += 6;
					}
				}
				if(o + 4 >= max)
					return -1;
				o += ui_utf8((rune)c, out + o);
				continue;
			}	/* " \ / as they are */
		}
		if(o + 1 >= max)
			return -1;
		out[o++] = (char)c;
	}
	out[o] = 0;
	return o;
}

/* the text [p, p + n) is one JSON object, all of it: its members to m */
static int
json_parse(const char *p, int n, void (*m)(const char *, int, int),
    void (*num)(const char *, int, long), void (*str)(const char *, int, const char *, int))
{
	struct json j;

	j.p = p;
	j.e = p + n;
	j.depth = 0;
	j.member = m;
	j.number = num;
	j.string = str;
	j.isnum = j.isstr = 0;
	if(n >= 3 && memcmp(p, "\xef\xbb\xbf", 3) == 0)
		j.p += 3;	/* a BOM */
	ws(&j);
	if(j.p >= j.e || *j.p != '{' || jvalue(&j) < 0)
		return 0;
	ws(&j);
	return j.p == j.e;
}

/* --- the file ------------------------------------------------------------ */

static char got[NNAMES];	/* what a parse found, -1 for none */
static int font_px;		/* the size of the text of a window; 0: its
				   default (a terminal has its own) */
static int tab_width = 4;	/* of a text not indented yet */
static char font[PATH_MAX_];	/* of a window: a file or a name; "": its own */
static char got_font[PATH_MAX_];	/* what a parse found */
static char has_font;
static int ruler;		/* the column marked (0: none) */

/* the settings that are numbers, each in its range */
static const struct {
	const char *name;
	int *v, min, max;
} nums[] = {
	{ "tab_width", &tab_width, 1, 8 },
	{ "ruler_column", &ruler, 0, 9999 },
	{ "font_size", &font_px, 0, 200 },
};
#define NNUMS	((int)(sizeof nums / sizeof nums[0]))
static long got_num[NNUMS];	/* what a parse found, -1 for none */

static void
take_num(const char *name, int len, long v)
{
	int i;

	for(i = 0; i < NNUMS; i++)
		if((size_t)len == strlen(nums[i].name) && memcmp(name, nums[i].name, (size_t)len) == 0 &&
		   v >= nums[i].min && v <= nums[i].max)
			got_num[i] = v;
}
static char invalid;		/* the file was not valid: kept aside first */

static void
take_str(const char *name, int len, const char *s, int n)
{
	if(len == 4 && memcmp(name, "font", 4) == 0 && junescape(s, n, got_font, sizeof got_font) >= 0)
		has_font = 1;
}

/* before a parse: nothing found */
static void
got_none(void)
{
	int i;

	memset(got, -1, sizeof got);
	for(i = 0; i < NNUMS; i++)
		got_num[i] = -1;
	got_font[0] = 0;
	has_font = 0;
}

static void
take(const char *name, int len, int v)
{
	int i;

	for(i = 0; i < NNAMES; i++)
		if((size_t)len == strlen(names[i].name) && memcmp(name, names[i].name, (size_t)len) == 0)
			got[i] = (char)v;
}

/* the file read: its length, -1 if there is none, -2 if too long */
static int
readfile(const char *path, char *buf, int max)
{
	int h, n, len = 0;

	if((h = file_open(path, 0)) < 0)
		return -1;
	while(len < max && (n = file_read(h, buf + len, max - len)) > 0)
		len += n;
	n = len == max && file_read(h, buf, 1) > 0;
	file_close(h);
	return n ? -2 : len;
}

static int settings_read(const char *path);

void
settings_load(void)
{
	char path[PATH_MAX_];
	int i;

	if(!plat_config(path, sizeof path))
		return;
	if(!settings_read(path) && file_kind(path) == 1)
		invalid = 1;	/* said once; the editor goes on, with the defaults */
	for(i = 0; i < NNAMES; i++)
		if(names[i].global)
			*names[i].global = value[i];
}

/* the JSON of the values, at buf: its length */
static int
make_json(char *buf)
{
	int i;

	strcpy(buf, "{\n");
	for(i = 0; i < NNAMES; i++) {
		strcat(buf, "  \"");
		strcat(buf, names[i].name);
		strcat(buf, "\": ");
		strcat(buf, value[i] ? "true" : "false");
		strcat(buf, ",\n");
	}
	for(i = 0; i < NNUMS; i++)
		sprintf(buf + strlen(buf), "  \"%s\": %d,\n", nums[i].name, *nums[i].v);
	strcat(buf, "  \"font\": \"");
	{	/* the string, escaped */
		const unsigned char *s = (const unsigned char *)font;
		char *o = buf + strlen(buf);

		for(; *s; s++)
			if(*s == '"' || *s == '\\') {
				*o++ = '\\';
				*o++ = (char)*s;
			} else if(*s < 0x20)
				o += sprintf(o, "\\u%04x", *s);
			else
				*o++ = (char)*s;
		*o = 0;
	}
	strcat(buf, "\"\n}\n");
	return (int)strlen(buf);
}

/* the text parses, and holds each value as meant */
static int
check_json(const char *buf, int len)
{
	int i;

	got_none();
	if(!json_parse(buf, len, take, take_num, take_str))
		return 0;
	if(strcmp(got_font, font) != 0)
		return 0;
	for(i = 0; i < NNAMES; i++)
		if(got[i] != value[i])
			return 0;
	for(i = 0; i < NNUMS; i++)
		if(got_num[i] != *nums[i].v)
			return 0;
	return 1;
}

static void settings_save(void);

/* the values read from path: 1, or 0 (said) if it is not valid JSON */
static int
settings_read(const char *path)
{
	char *buf;
	int i, len, ok = 0;

	if((buf = mem_alloc(JSON_MAX)) == 0)
		return 0;
	got_none();
	len = readfile(path, buf, JSON_MAX);
	if(len >= 0 && json_parse(buf, len, take, take_num, take_str)) {
		if(has_font)
			strcpy(font, got_font);
		for(i = 0; i < NNAMES; i++)
			if(got[i] >= 0)
				value[i] = got[i];
		for(i = 0; i < NNUMS; i++)
			if(got_num[i] >= 0)
				*nums[i].v = (int)got_num[i];
		ok = 1;
	} else if(len != -1)
		error("Invalid Settings: Invalid JSON", 0);
	mem_free(buf);
	return ok;
}

void
settings_open(void)
{
	char path[PATH_MAX_];

	if(!plat_config(path, sizeof path)) {
		error("Invalid Settings: No folder for them", 0);
		return;
	}
	if(file_kind(path) != 1)
		settings_save();	/* none yet: made with the values */
	doc_open(path);
}

void
settings_saved(const char *path)
{
	char cfg[PATH_MAX_], was[PATH_MAX_];
	struct doc *d;
	int i;

	if(!plat_config(cfg, sizeof cfg) || strcmp(cfg, path) != 0)
		return;
	invalid = 0;	/* written by its user: what it holds is theirs */
	strcpy(was, font);
	if(!settings_read(path))
		return;
	if(strcmp(was, font) != 0)
		settings_use_font();
	for(i = 0; i < NNAMES; i++)
		if(names[i].global)
			*names[i].global = value[i];
	for(d = docs; d; d = d->next)
		settings_apply(d);
	rows_stale = 1;
	bar_stale = 1;
}

static void
settings_save(void)
{
	static char buf[JSON_MAX], back[JSON_MAX];
	char path[PATH_MAX_];
	int h, len, n, ok;

	if(!plat_config(path, sizeof path))
		return;
	if(invalid) {	/* what was there, as path.invalid, before it goes */
		char old[JSON_MAX], keep[PATH_MAX_ + 8];
		int k = readfile(path, old, (int)sizeof old);

		strcpy(keep, path);
		strcat(keep, ".invalid");
		if(k >= 0 && (h = file_open(keep, 1)) >= 0) {
			file_write(h, old, k);
			file_close(h);
		}
		invalid = 0;
	}
	len = make_json(buf);
	if(!check_json(buf, len)) {	/* before: what is to be written */
		error("Invalid Settings: Invalid JSON", 0);	/* not written */
		return;
	}
	if((h = file_open(path, 1)) < 0) {
		error("Invalid Settings: Cannot write ", path);
		return;
	}
	ok = file_write(h, buf, len) == len;
	if(file_close(h) < 0)
		ok = 0;
	/* after: what the file holds now */
	n = ok ? readfile(path, back, (int)sizeof back) : -1;
	if(!ok || n != len || memcmp(back, buf, (size_t)len) != 0 || !check_json(back, n))
		error("Invalid Settings: Not written correctly ", path);
	else
		doc_reload(path);	/* open in the editor: as it is now */
}

/* the settings, to a new document */
void
settings_apply(struct doc *d)
{
	int i;

	for(i = 0; i < NNAMES; i++)
		if(names[i].action)
			*field(d, names[i].action) = value[i];
	if(no_line_numbers)
		d->linenums = 0;
	if(!d->b.detected) {	/* the file's own indentation goes first */
		d->b.tabs = !expand;
		d->b.tabsize = (char)tab_width;
	} else if(d->b.tabs)
		d->b.tabsize = (char)tab_width;
}

int
settings_ruler(void)
{
	return ruler;
}

/* setting i turned on or off (in the active document, or in Find), and
 * saved */
static void
settings_flip(int i)
{
	char *f = names[i].global;

	if(f == 0) {
		if(docs == 0)
			return;
		f = field(D, names[i].action);
	}
	*f = !*f;
	value[i] = *f;
	settings_save();
}

/* the option of action a turned on or off in the active document, and
 * saved: 1 if a is one */
int
settings_toggle(int a)
{
	int i;

	for(i = 0; i < NNAMES && names[i].action != a; i++)
		;
	if(i == NNAMES || names[i].action == 0 || docs == 0)
		return 0;
	settings_flip(i);
	return 1;
}

/* the font of the settings to the window (nothing in a terminal); one
 * that cannot be had is said, and the one shown stays */
void
settings_use_font(void)
{
	switch(term_set_font(font)) {
	case FONT_NOTFOUND:
		error("Font not found: ", font);
		break;
	case FONT_BAD:
		error("Cannot use the font: ", font);
		break;
	}
}

int
settings_font_size(void)
{
	return font_px;
}

/* the size of the text of the window changed (the zoom): kept */
void
settings_set_font_size(int px)
{
	if(px == font_px)
		return;
	font_px = px;
	settings_save();
}

/* an option of Find changed in its panel: kept */
void
settings_find_changed(void)
{
	int i, changed = 0;

	for(i = 0; i < NNAMES; i++)
		if(names[i].global && value[i] != *names[i].global) {
			value[i] = *names[i].global;
			changed = 1;
		}
	if(changed)
		settings_save();
}
