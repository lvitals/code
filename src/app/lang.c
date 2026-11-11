/* The language of a document: what the status bar names, and how its
 * text is highlighted.  Decided here, once, when a file is opened or
 * saved: the editor only draws the text.  Nothing here is a language
 * server (no Clang, no LSP).
 *
 * The languages are those of the table (syntax.h, langs.c): a file is known by
 * its name (Makefile, CMakeLists.txt), else by its suffix (each one after
 * a dot: x.cmake.in), else a shebang (#!/usr/bin/env python3), else the
 * first lines (#include, public class, local). */
#include <string.h>
#include "app.h"
#include "syntax.h"

int
lang_find(const char *name)
{
	int i;

	for(i = 1; name && i < nsyns; i++)
		if(strcmp(syns[i]->name, name) == 0)
			return i;
	return 0;
}

static int
lower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

static int
eq(const char *a, const char *b)
{
	unsigned char x, y;

	while(*a && *b) {
		x = (unsigned char)*a++;
		y = (unsigned char)*b++;
		if(x >= 'A' && x <= 'Z')
			x = (unsigned char)(x + 32);
		if(y >= 'A' && y <= 'Z')
			y = (unsigned char)(y + 32);
		if(x != y)
			return 0;
	}
	return *a == *b;
}

/* w is one of the words (apart by spaces) of list: exactly as written,
 * or in any case */
static int
inlist(const char *list, const char *w, int anycase)
{
	const char *p, *e;
	size_t n = strlen(w), i;

	for(p = list; p && *p; p = e) {
		while(*p == ' ')
			p++;
		for(e = p; *e && *e != ' '; e++)
			;
		if((size_t)(e - p) != n || n == 0)
			continue;
		for(i = 0; i < n; i++)
			if(anycase ? lower((unsigned char)p[i]) != lower((unsigned char)w[i]) :
			    p[i] != w[i])
				break;
		if(i == n)
			return 1;
	}
	return 0;
}

/* the field of the languages that tell a file */
enum { BY_FILE, BY_SUFFIX, BY_INTERP };

/* the language whose field has key: exactly as written first, then in
 * any case; 0 if none.  A file's whole name is looked for among the
 * names and the suffixes (a file named "makefile"), a suffix among the
 * suffixes and the names */
static int
find(int by, const char *key)
{
	const struct syn *l;
	int i, anycase;

	if(key == 0 || key[0] == 0)
		return 0;
	for(anycase = 0; anycase < 2; anycase++)
		for(i = 1; i < nsyns; i++) {
			l = syns[i];
			if(by == BY_INTERP ? inlist(l->interpreters, key, anycase) :
			    by == BY_FILE ? inlist(l->filenames, key, anycase) ||
			    inlist(l->extensions, key, anycase) :
			    inlist(l->extensions, key, anycase) || inlist(l->filenames, key, anycase))
				return i;
		}
	return 0;
}

static int
starts(const char *s, const char *pre)
{
	while(*pre)
		if(*s++ != *pre++)
			return 0;
	return 1;
}

/* the language one of whose paths the path ends with: .git/config */
static int
ends(const char *path)
{
	const char *p, *e;
	size_t n, m, i;
	int k;

	if(path == 0)
		return 0;
	n = strlen(path);
	for(k = 1; k < nsyns; k++)
		for(p = syns[k]->paths; p && *p; p = e) {
			while(*p == ' ')
				p++;
			for(e = p; *e && *e != ' '; e++)
				;
			m = (size_t)(e - p);
			if(m == 0 || m > n)
				continue;
			for(i = 0; i < m && lower((unsigned char)path[n - m + i]) ==
			    lower((unsigned char)p[i]); i++)
				;
			if(i == m)
				return k;
		}
	return 0;
}

static pos_t
text0(void)
{
	if(buf_len() >= 3 && BCH(0) == 0xEF && BCH(1) == 0xBB && BCH(2) == 0xBF)
		return 3;
	return 0;
}

/* the program of a #! line: python3.11 is python, lua5.4 lua */
static int
interp(const char *w)
{
	char base[64];
	size_t n;
	int id;

	if((id = find(BY_INTERP, w)) != 0)
		return id;
	n = strlen(w);
	while(n > 0 && ((w[n - 1] >= '0' && w[n - 1] <= '9') || w[n - 1] == '.'))
		n--;
	if(n == 0 || n >= sizeof base || n == strlen(w))
		return 0;
	memcpy(base, w, n);
	base[n] = 0;
	return find(BY_INTERP, base);
}

static int
shebang(void)
{
	char line[128];
	pos_t n, o = text0();
	char *p, *w;

	if(buf_len() < o + 2 || BCH(o) != '#' || BCH(o + 1) != '!')
		return 0;
	n = buf_lineend(o) - o;
	if(n >= sizeof line)
		n = sizeof line - 1;
	buf_copy(o, n, line);
	line[n] = 0;
	p = line + 2;
	for(;;) {
		while(*p == ' ' || *p == '\t')
			p++;
		if(*p == 0)
			return 0;
		w = p;
		while(*p && *p != ' ' && *p != '\t')
			p++;
		if(*p)
			*p++ = 0;
		{
			char *s, *b = w;

			for(s = w; *s; s++)
				if(*s == '/')
					b = s + 1;
			w = b;
		}
		if(w[0] == '-')	/* env -S */
			continue;
		if(eq(w, "env"))
			continue;
		return interp(w);
	}
}

/* the first lines, only when the name said nothing */
enum {
	F_C = 1, F_CPP = 2, F_JAVA = 4, F_LUA = 8, F_JS = 16, F_PY = 32,
	F_XML = 64, F_RS = 128, F_GO = 256, F_JSON = 512, F_RB = 1024,
	F_SH = 2048, F_SQL = 4096, F_FUNC = 8192, F_END = 16384
};

static int
pick(unsigned f)
{
	static const struct {
		unsigned f;
		const char *lang;
	} order[] = {
		{ F_XML, "xml" }, { F_JAVA, "java" }, { F_GO, "go" }, { F_LUA, "lua" },
		{ F_RS, "rust" }, { F_PY, "python" }, { F_RB, "ruby" }, { F_CPP, "cpp" },
		{ F_C, "c" }, { F_JS, "javascript" }, { F_SQL, "sql" }, { F_SH, "bash" },
		{ F_JSON, "json" }, { 0, 0 }
	};
	int i;

	for(i = 0; order[i].f; i++) {
		if(order[i].f == F_SQL && (f & F_FUNC) && (f & F_END))
			return lang_find("lua");
		if(f & order[i].f)
			return lang_find(order[i].lang);
	}
	return 0;
}

static void
line_lang(const char *line, unsigned *f)
{
	if(line[0] == '#') {
		if(starts(line, "#include") || starts(line, "#define") ||
		   starts(line, "#if") || starts(line, "#pragma") || starts(line, "#undef"))
			*f |= F_C;
		if(strstr(line, "iostream") || strstr(line, "<vector>") ||
		   strstr(line, "<string>") || strstr(line, "<memory>"))
			*f |= F_CPP;
		return;
	}
	if(strstr(line, "std::") || starts(line, "namespace") || starts(line, "template"))
		*f |= F_CPP;
	if(starts(line, "public class ") || starts(line, "public interface ") ||
	   starts(line, "public enum ") || starts(line, "import java."))
		*f |= F_JAVA;
	if(starts(line, "package main") || starts(line, "func "))
		*f |= F_GO;
	else if(starts(line, "package "))
		*f |= F_JAVA;
	if(starts(line, "local ") || starts(line, "--[[") || starts(line, "require(") ||
	   starts(line, "require \""))
		*f |= F_LUA;
	if(starts(line, "fn ") || starts(line, "fn(") || starts(line, "pub ") ||
	   starts(line, "impl "))
		*f |= F_RS;
	if(starts(line, "def "))
		*f |= strchr(line, ':') ? F_PY : F_RB;
	if(starts(line, "class "))
		*f |= strchr(line, ':') ? F_PY : F_CPP;
	if(starts(line, "from ") && strstr(line, " import"))
		*f |= F_PY;
	if(starts(line, "import ")) {
		if(starts(line, "import java."))
			*f |= F_JAVA;
		else if(strstr(line, " from ") || starts(line, "import {") || strstr(line, "React"))
			*f |= F_JS;
		else
			*f |= F_PY;
	}
	if(starts(line, "export ") || strstr(line, "=>") || starts(line, "const ") ||
	   starts(line, "let "))
		*f |= F_JS;
	if(starts(line, "function "))
		*f |= F_FUNC;
	if(strcmp(line, "end") == 0)
		*f |= F_END;
	if(starts(line, "require '"))
		*f |= F_RB;
	if(starts(line, "<html") || starts(line, "<HTML") || starts(line, "<!DOCTYPE") ||
	   starts(line, "<?xml") || starts(line, "<svg"))
		*f |= F_XML;
	if(starts(line, "SELECT ") || starts(line, "select ") || starts(line, "CREATE ") ||
	   starts(line, "create "))
		*f |= F_SQL;
	if(strcmp(line, "fi") == 0 || strcmp(line, "esac") == 0)
		*f |= F_SH;
}

static int
from_text(void)
{
	char b[768];
	pos_t n, o = text0();
	char *p, *e, *line;
	unsigned f = 0;
	int lines = 0;

	if(buf_len() <= o)
		return 0;
	n = buf_len() - o;
	if(n > sizeof b - 1)
		n = sizeof b - 1;
	buf_copy(o, n, b);
	b[n] = 0;
	p = b;
	while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	if((*p == '{' || *p == '[') && strchr(p, '"'))
		f |= F_JSON;
	for(p = b; *p && lines < 40; lines++) {
		line = p;
		while(*p && *p != '\n')
			p++;
		e = p;
		if(*p)
			p++;
		while(line < e && (*line == ' ' || *line == '\t' || *line == '\r'))
			line++;
		*e = 0;
		if(line < e)
			line_lang(line, &f);
	}
	return pick(f);
}

const char *
lang_name(int id)
{
	return syns[id > 0 && id < nsyns ? id : 0]->title;
}

int
lang_detect(const char *path)
{
	const char *base = path_last(path), *s;
	int id;

	if((id = find(BY_FILE, base)) != 0)
		return id;
	for(s = base; *s; s++)	/* each suffix: x.cmake.in, then x.in */
		if(*s == '.' && s[1] && (id = find(BY_SUFFIX, s + 1)) != 0)
			return id;
	if(strchr(base + (base[0] == '.'), '.'))
		return 0;	/* a suffix we do not know stays plain */
	if((id = ends(path)) != 0)
		return id;
	if((id = shebang()) != 0)
		return id;
	return from_text();
}
