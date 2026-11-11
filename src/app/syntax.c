/* Syntax highlighting, in C alone: the languages (src/app/langs/, one
 * file each) read by one lexer of lines here,
 * or by one of a few lexers of their own kind (Diff, Markdown, XML,
 * key=value files).
 *
 * A line is lexed from the state at its start: 0, or what the line
 * before left open (a block comment, a string of many lines, a tag).
 * The editor lexes a window of the text before the screen to know the
 * state at its top (as editors with lexers do): the text before it is
 * never read.
 *
 * The words of a language (some have thousands) are put in a hash table
 * the first time it is lexed. */
#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "syntax.h"

/* the states a line can end in */
enum {
	ST_NONE,
	ST_BCOMMENT = 1,	/* + which of bc[] */
	ST_LONG = 4,		/* + which of ls[] */
	ST_BACKTICK = 8, ST_MDCODE, ST_TAG, ST_XCOMMENT,
	ST_LUASTR = 16,		/* + the count of = of [==[ */
	ST_LUACOM = 48
};

int
syn_has(int lang)
{
	return lang > 0 && lang < nsyns && syns[lang]->kind != K_PLAIN;
}

static int
isw(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
	    c == '_' || c >= 0x80;
}

static int
lower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

/* s at p + i */
static int
at(const char *p, int n, int i, const char *s)
{
	int k = s ? (int)strlen(s) : 0;

	return k > 0 && i + k <= n && memcmp(p + i, s, (size_t)k) == 0;
}

/* the line is a comment by its column (.column_comments): one of the
 * characters in that column, or (any, 1) the first that is not blank.  A
 * letter is one when no letter or digit follows it (Fortran's "C  text",
 * not a CALL of free form at the start of the line) */
static int
col_comment(const struct syn *s, const char *p, int n, int any)
{
	const struct column_comment *cc;
	int k, i, c;

	for(k = 0; k < 3 && s->column_comments[k].column > 0; k++) {
		cc = &s->column_comments[k];
		if(any)
			for(i = 0; i < n && (p[i] == ' ' || p[i] == '\t'); i++)
				;
		else
			i = cc->column - 1;
		if(i >= n || p[i] == 0 || !strchr(cc->chars, p[i]))
			continue;
		c = (unsigned char)p[i];
		if(((c | 32) >= 'a' && (c | 32) <= 'z') && i + 1 < n && isw((unsigned char)p[i + 1]))
			continue;
		return 1;
	}
	return 0;
}

static void
mark(unsigned char *cls, int a, int b, int c)
{
	if(cls)
		for(; a < b; a++)
			cls[a] = (unsigned char)c;
}

/* the end of what closes s at i (past it), or -1 if not on this line */
static int
find(const char *p, int n, int i, const char *s)
{
	for(; i < n; i++)
		if(at(p, n, i, s))
			return i + (int)strlen(s);
	return -1;
}

/* a string from i (its quote q): its end, past the quote, or n */
static int
string_end(const char *p, int n, int i, int q)
{
	for(i++; i < n; i++) {
		if(p[i] == '\\' && q != '`')
			i++;
		else if(p[i] == q)
			return i + 1;
	}
	return n;
}

/* a number from i: its end */
static int
number_end(const char *p, int n, int i)
{
	for(i++; i < n; i++) {
		if((p[i] == '+' || p[i] == '-') && (p[i - 1] == 'e' || p[i - 1] == 'E') &&
		   !(p[0] == '0' && (p[1] == 'x' || p[1] == 'X')))
			continue;
		if(!isw((unsigned char)p[i]) && p[i] != '.')
			break;
	}
	return i;
}

/* [==[ at i: the count of = (or -1) */
static int
lualong(const char *p, int n, int i)
{
	int k = 0;

	if(i >= n || p[i] != '[')
		return -1;
	for(i++; i < n && p[i] == '='; i++)
		k++;
	return i < n && p[i] == '[' && k < 30 ? k : -1;
}

/* ]==] with k = from i: past it, or -1 */
static int
lualong_end(const char *p, int n, int i, int k)
{
	int j, m;

	for(; i < n; i++) {
		if(p[i] != ']')
			continue;
		for(j = i + 1, m = 0; j < n && p[j] == '='; j++)
			m++;
		if(m == k && j < n && p[j] == ']')
			return j + 1;
	}
	return -1;
}

/* --- the words: a hash table of each language ---------------------------- */

struct word {
	const char *p;		/* in a list of the table: not copied */
	unsigned char len, cls;
};

static struct wtab {
	struct word *w;
	unsigned mask;		/* its size, less one (a power of 2) */
	unsigned char extra[256];	/* characters of words, past letters and
					   digits: '-' of CSS, '?' of Ruby */
	unsigned char first[256];	/* that start one: '@', '$' */
} *wtabs[256];

static unsigned
hash(const char *s, int n, int nocase)
{
	unsigned h = 2166136261u;
	int i;

	for(i = 0; i < n; i++)
		h = (h ^ (unsigned)(nocase ? lower((unsigned char)s[i]) : (unsigned char)s[i])) *
		    16777619u;
	return h;
}

static void
wtab_add(struct wtab *t, const char *const *list, int cls, int nocase)
{
	const char *p, *e;
	unsigned h;
	int i;

	for(; list && *list; list++) {
		p = *list;
		e = p + strlen(p);
		if(e == p || e - p > 255)
			continue;
		for(i = 0; i < e - p; i++)
			if(!isw((unsigned char)p[i]))
				t->extra[(unsigned char)p[i]] = 1;
		if(!isw((unsigned char)p[0]))
			t->first[(unsigned char)p[0]] = 1;
		h = hash(p, (int)(e - p), nocase) & t->mask;
		while(t->w[h].p) {	/* already one: the first list wins */
			if(t->w[h].len == e - p && (nocase ?
			    strncmp(t->w[h].p, p, (size_t)(e - p)) == 0 : memcmp(t->w[h].p, p, (size_t)(e - p)) == 0))
				break;
			h = (h + 1) & t->mask;
		}
		if(t->w[h].p == 0) {
			t->w[h].p = p;
			t->w[h].len = (unsigned char)(e - p);
			t->w[h].cls = (unsigned char)cls;
		}
	}
}

static size_t
count_words(const char *const *list)
{
	size_t n = 0;

	for(; list && *list; list++)
		n++;
	return n;
}

static struct wtab *
wtab_of(int lang)
{
	const struct syn *s = syns[lang];
	struct wtab *t;
	size_t n;
	unsigned size = 64;
	int nocase = (s->flags & F_NOCASE) != 0;

	if(wtabs[lang])
		return wtabs[lang];
	n = count_words(s->keywords) + count_words(s->types) + count_words(s->constants) +
	    count_words(s->functions) + count_words(s->preproc);
	while(size < n * 2 + 2)
		size *= 2;
	if((t = calloc(1, sizeof *t)) == 0 || (t->w = calloc(size, sizeof *t->w)) == 0) {
		free(t);
		return 0;
	}
	t->mask = size - 1;
	wtab_add(t, s->keywords, HL_KEYWORD, nocase);
	wtab_add(t, s->types, HL_TYPE, nocase);
	wtab_add(t, s->constants, HL_NUMBER, nocase);
	wtab_add(t, s->functions, HL_TYPE, nocase);
	wtab_add(t, s->preproc, HL_PREPROC, nocase);
	wtabs[lang] = t;
	return t;
}

/* the class of the word s (n bytes): HL_TEXT if none */
static int
word_class(struct wtab *t, const char *s, int n, int nocase)
{
	unsigned h;
	int i;

	if(t == 0 || n > 255)
		return HL_TEXT;
	for(h = hash(s, n, nocase) & t->mask; t->w[h].p; h = (h + 1) & t->mask) {
		if(t->w[h].len != n)
			continue;
		for(i = 0; i < n; i++)
			if(nocase ? lower((unsigned char)t->w[h].p[i]) != lower((unsigned char)s[i]) :
			    t->w[h].p[i] != s[i])
				break;
		if(i == n)
			return t->w[h].cls;
	}
	return HL_TEXT;
}

/* --- code ---------------------------------------------------------------- */

/* the end, past it, of what closes the state st from i: -1 if not on
 * this line */
static int
close_state(const struct syn *s, const char *p, int n, int i, int st)
{
	if(st >= ST_LUACOM)
		return lualong_end(p, n, i, st - ST_LUACOM);
	if(st >= ST_LUASTR)
		return lualong_end(p, n, i, st - ST_LUASTR);
	if(st == ST_BACKTICK)
		return find(p, n, i, "`");
	if(st >= ST_LONG)
		return find(p, n, i, s->long_strings[st - ST_LONG][1]);
	return find(p, n, i, s->block_comments[st - ST_BCOMMENT][1]);
}

static int
state_class(int st)
{
	return st == ST_LUACOM || (st >= ST_BCOMMENT && st < ST_LONG) || st > ST_LUACOM ?
	    HL_COMMENT : HL_STRING;
}

static int
lex_code(int lang, const char *p, int n, int st, unsigned char *cls)
{
	const struct syn *s = syns[lang];
	struct wtab *t = wtab_of(lang);
	int i = 0, j, k, c, nocase = s->flags & F_NOCASE, start = 1, hit;
	int dash = (s->flags & F_DASH) != 0;

	if(st == ST_NONE && col_comment(s, p, n, 0)) {	/* * in column 7 */
		mark(cls, 0, n, HL_COMMENT);
		return ST_NONE;
	}
	if(st != ST_NONE) {	/* what the line before left open */
		j = close_state(s, p, n, 0, st);
		mark(cls, 0, j < 0 ? n : j, state_class(st));
		if(j < 0)
			return st;
		i = j;
	}
	while(i < n) {
		c = (unsigned char)p[i];
		if(c == ' ' || c == '\t') {
			mark(cls, i, i + 1, HL_TEXT);
			i++;
			continue;
		}
		if(start && c == '#' && (s->flags & F_HASHPRE) &&
		   !(i + 1 < n && p[i + 1] == '[' && (s->flags & F_ATTR))) {	/* #include */
			for(j = i + 1; j < n; j++) {
				for(k = 0; k < 4 && s->line_comments[k] && !at(p, n, j, s->line_comments[k]); k++)
					;
				if((k < 4 && s->line_comments[k]) || (s->block_comments[0][0] && at(p, n, j, s->block_comments[0][0])))
					break;
			}
			mark(cls, i, j, HL_PREPROC);
			i = j;
			start = 0;
			continue;
		}
		start = 0;
		/* a long comment of Lua, before its line comment */
		if((s->flags & F_LUALONG) && at(p, n, i, "--") && (k = lualong(p, n, i + 2)) >= 0) {
			j = lualong_end(p, n, i + 2, k);
			mark(cls, i, j < 0 ? n : j, HL_COMMENT);
			if(j < 0)
				return ST_LUACOM + k;
			i = j;
			continue;
		}
		for(hit = 0, k = 0; k < 3 && s->block_comments[k][0]; k++)	/* block comments */
			if(at(p, n, i, s->block_comments[k][0])) {
				j = find(p, n, i + (int)strlen(s->block_comments[k][0]), s->block_comments[k][1]);
				mark(cls, i, j < 0 ? n : j, HL_COMMENT);
				if(j < 0)
					return ST_BCOMMENT + k;
				i = j;
				hit = 1;
				break;
			}
		if(hit)
			continue;
		for(k = 0; k < 4 && s->line_comments[k]; k++)	/* comments to the end */
			if(at(p, n, i, s->line_comments[k])) {
				mark(cls, i, n, HL_COMMENT);
				return ST_NONE;
			}
		for(hit = 0, k = 0; k < 4 && s->long_strings[k][0]; k++)	/* strings of many */
			if(at(p, n, i, s->long_strings[k][0])) {
				j = find(p, n, i + (int)strlen(s->long_strings[k][0]), s->long_strings[k][1]);
				mark(cls, i, j < 0 ? n : j, HL_STRING);
				if(j < 0)
					return ST_LONG + k;
				i = j;
				hit = 1;
				break;
			}
		if(hit)
			continue;
		if((s->flags & F_LUALONG) && (k = lualong(p, n, i)) >= 0) {
			j = lualong_end(p, n, i + 1, k);
			mark(cls, i, j < 0 ? n : j, HL_STRING);
			if(j < 0)
				return ST_LUASTR + k;
			i = j;
			continue;
		}
		if(c == '`' && (s->flags & F_BACKTICK)) {
			j = find(p, n, i + 1, "`");
			mark(cls, i, j < 0 ? n : j, HL_STRING);
			if(j < 0)
				return ST_BACKTICK;
			i = j;
			continue;
		}
		if(s->quotes && c && strchr(s->quotes, c) && !(c == '\'' && (s->flags & F_NOQ1) &&
		   !(i + 2 < n && (p[i + 2] == '\'' || p[i + 1] == '\\')))) {
			j = string_end(p, n, i, c);
			mark(cls, i, j, HL_STRING);
			i = j;
			continue;
		}
		if((s->flags & F_VARS) && c == '$' && i + 1 < n &&
		   (isw((unsigned char)p[i + 1]) || p[i + 1] == '{' || p[i + 1] == '(')) {
			j = i + 1;
			if(p[j] == '{' || p[j] == '(')
				j = find(p, n, j, p[j] == '{' ? "}" : ")");
			else
				while(j < n && isw((unsigned char)p[j]))
					j++;
			if(j < 0)
				j = n;
			mark(cls, i, j, HL_TYPE);
			i = j;
			continue;
		}
		if((s->flags & F_ATTR) && (c == '@' || (c == '#' && i + 1 < n && p[i + 1] == '[')) &&
		   i + 1 < n) {
			j = i + 1;
			if(p[j] == '[')
				j = find(p, n, j, "]");
			else
				while(j < n && (isw((unsigned char)p[j]) || p[j] == '.'))
					j++;
			if(j < 0)
				j = n;
			mark(cls, i, j, HL_PREPROC);
			i = j;
			continue;
		}
		if((c >= '0' && c <= '9') || (c == '.' && i + 1 < n && p[i + 1] >= '0' && p[i + 1] <= '9')) {
			j = number_end(p, n, i);
			mark(cls, i, j, HL_NUMBER);
			i = j;
			continue;
		}
		if(isw(c) || (t && t->first[c])) {
			for(j = i + 1; j < n && (isw((unsigned char)p[j]) ||
			    (t && t->extra[(unsigned char)p[j]]) || (dash && p[j] == '-')); j++)
				;
			/* the longest that is a word, else up to the first
			   character that is not a letter (a-b: a); a name with -
			   in it is one (DISPLAY-TOTAL is no DISPLAY) */
			k = word_class(t, p + i, j - i, nocase);
			if(k == HL_TEXT && t && !dash)
				for(j = i + 1; j < n && isw((unsigned char)p[j]); j++)
					;
			if(k == HL_TEXT)
				k = word_class(t, p + i, j - i, nocase);
			mark(cls, i, j, k);
			i = j;
			continue;
		}
		mark(cls, i, i + 1, HL_TEXT);
		i++;
	}
	return ST_NONE;
}

/* --- the other kinds ----------------------------------------------------- */

static int
lex_diff(const char *p, int n, unsigned char *cls)
{
	int c = n ? p[0] : 0;

	if(at(p, n, 0, "+++") || at(p, n, 0, "---") || at(p, n, 0, "diff ") || at(p, n, 0, "index "))
		mark(cls, 0, n, HL_KEYWORD);
	else if(c == '@')
		mark(cls, 0, n, HL_PREPROC);
	else
		mark(cls, 0, n, c == '+' ? HL_STRING : c == '-' ? HL_DELETED : HL_TEXT);
	return ST_NONE;
}

static int
lex_md(const char *p, int n, int st, unsigned char *cls)
{
	int i = 0, j;

	while(i < n && (p[i] == ' ' || p[i] == '\t'))
		i++;
	if(at(p, n, i, "```") || at(p, n, i, "~~~")) {
		mark(cls, 0, n, HL_PREPROC);
		return st == ST_MDCODE ? ST_NONE : ST_MDCODE;
	}
	if(st == ST_MDCODE) {
		mark(cls, 0, n, HL_STRING);
		return st;
	}
	mark(cls, 0, n, HL_TEXT);
	if(i < n && p[i] == '#') {
		mark(cls, 0, n, HL_KEYWORD);
		return ST_NONE;
	}
	if(i < n && p[i] == '>') {
		mark(cls, 0, n, HL_COMMENT);
		return ST_NONE;
	}
	if(i + 1 < n && (p[i] == '-' || p[i] == '*' || p[i] == '+') && p[i + 1] == ' ')
		mark(cls, i, i + 1, HL_PREPROC);
	for(; i < n; i++) {
		if(p[i] == '`') {
			j = find(p, n, i + 1, "`");
			mark(cls, i, j < 0 ? n : j, HL_STRING);
			i = j < 0 ? n : j - 1;
		} else if((p[i] == '*' || p[i] == '_') && i + 1 < n && p[i + 1] == p[i]) {
			j = find(p, n, i + 2, p[i] == '*' ? "**" : "__");
			if(j > 0) {
				mark(cls, i, j, HL_TYPE);
				i = j - 1;
			}
		} else if(p[i] == '[') {
			j = find(p, n, i + 1, "](");
			if(j > 0 && (j = find(p, n, j, ")")) > 0) {
				mark(cls, i, j, HL_NUMBER);
				i = j - 1;
			}
		}
	}
	return ST_NONE;
}

static int
lex_xml(const struct syn *s, const char *p, int n, int st, unsigned char *cls)
{
	int i = 0, j;

	mark(cls, 0, n, HL_TEXT);
	while(i < n) {
		if(st == ST_XCOMMENT) {
			j = find(p, n, i, "-->");
			mark(cls, i, j < 0 ? n : j, HL_COMMENT);
			if(j < 0)
				return st;
			i = j;
			st = ST_NONE;
		} else if(st == ST_TAG) {	/* the attributes, to > */
			if(p[i] == '>' || at(p, n, i, "/>")) {
				j = i + (p[i] == '/' ? 2 : 1);
				mark(cls, i, j, HL_KEYWORD);
				i = j;
				st = ST_NONE;
			} else if(s->quotes && strchr(s->quotes, p[i])) {
				j = string_end(p, n, i, p[i]);
				mark(cls, i, j, HL_STRING);
				i = j;
			} else if(isw((unsigned char)p[i])) {
				for(j = i; j < n && (isw((unsigned char)p[j]) || p[j] == '-' || p[j] == ':'); j++)
					;
				mark(cls, i, j, HL_TYPE);
				i = j;
			} else
				i++;
		} else if(at(p, n, i, "<!--")) {
			st = ST_XCOMMENT;
		} else if(p[i] == '<') {
			for(j = i + 1; j < n && (isw((unsigned char)p[j]) || strchr("/!?-:.", p[j])); j++)
				;
			mark(cls, i, j, HL_KEYWORD);
			i = j;
			st = ST_TAG;
		} else if(p[i] == '&') {
			j = find(p, n, i, ";");
			if(j > 0 && j - i < 12) {
				mark(cls, i, j, HL_NUMBER);
				i = j;
			} else
				i++;
		} else
			i++;
	}
	return st;
}

/* ini, properties, TOML, YAML: [sections], keys, then values as code */
static int
lex_keyval(int lang, const char *p, int n, int st, unsigned char *cls)
{
	const struct syn *s = syns[lang];
	int i = 0, j, k;

	if(st != ST_NONE)
		return lex_code(lang, p, n, st, cls);
	while(i < n && (p[i] == ' ' || p[i] == '\t'))
		i++;
	mark(cls, 0, i, HL_TEXT);
	for(k = 0; k < 4 && s->line_comments[k]; k++)
		if(at(p, n, i, s->line_comments[k])) {
			mark(cls, i, n, HL_COMMENT);
			return ST_NONE;
		}
	if(col_comment(s, p, n, 1)) {	/* ; or # first: INI's */
		mark(cls, i, n, HL_COMMENT);
		return ST_NONE;
	}
	if(i < n && p[i] == '[') {
		mark(cls, i, n, HL_KEYWORD);
		return ST_NONE;
	}
	if(at(p, n, i, "- ")) {	/* a YAML item */
		mark(cls, i, i + 2, HL_PREPROC);
		i += 2;
	}
	for(j = i; j < n && p[j] != '=' && p[j] != ':' && !(s->quotes && strchr(s->quotes, p[j])); j++)
		;
	if(j < n && (p[j] == '=' || p[j] == ':') && j > i) {
		mark(cls, i, j, HL_TYPE);
		mark(cls, j, j + 1, HL_TEXT);
		i = j + 1;
	}
	return lex_code(lang, p + i, n - i, ST_NONE, cls ? cls + i : 0);
}

int
syn_line(int lang, const char *p, int n, int state, unsigned char *cls)
{
	const struct syn *s;

	if(!syn_has(lang)) {
		mark(cls, 0, n, HL_TEXT);
		return ST_NONE;
	}
	s = syns[lang];
	switch(s->kind) {
	case K_DIFF:
		return lex_diff(p, n, cls);
	case K_MD:
		return lex_md(p, n, state, cls);
	case K_XML:
		return lex_xml(s, p, n, state, cls);
	case K_KEYVAL:
		return lex_keyval(lang, p, n, state, cls);
	}
	return lex_code(lang, p, n, state, cls);
}
