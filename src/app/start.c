/* The start of the editor: the memory, the arguments, the documents, the
 * terminal. */
#include <string.h>
#include "app.h"
#include "../core/mem.h"
#include "syntax.h"

/* --- the arguments ------------------------------------------------------- */

static void
usage(void)
{
	plat_print("Usage: ");
	plat_print(plat_name);
	plat_print(" [OPTIONS] [FILE|DIR]...\r\n"
	    "A DIR is the project's folder (Open, Save As, Open File in Project, Search in Project)\r\n"
	    "Options:\r\n"
	    "    -f, --foreground                      The window: keep the terminal until it closes\r\n"
	    "                                          (as EDITOR: git commit waits for it)\r\n"
	    "    -g, --goto <FILE:LINE[:CHARACTER]>    Open a file at the specified line and character position\r\n"
	    "    -h, --help                            Print this help message\r\n"
	    "        --lex <FILE> [LANGUAGE]           Print the colour of each character of FILE\r\n"
	    "                                          (to write a language: docs/LANGUAGES.md)\r\n"
	    "    -n, --no-line-numbers                 Do not count nor show the line numbers\r\n"
	    "    -v, --version                         Print the version number\r\n"
	    "\r\n");
}

/* name:line[:column] as name and the place: 0 if it has none */
static int
split_goto(char *name, pos_t *line, pos_t *col)
{
	char *p, *q;
	pos_t v[2] = { 0, 0 };
	int n = 0, i;

	for(i = 0; i < 2; i++) {
		p = strrchr(name, ':');
		if(p == 0 || p[1] == 0 || (path_sep == '\\' && p == name + 1))
			break;
		for(q = p + 1; *q >= '0' && *q <= '9'; q++)
			;
		if(*q)
			break;
		v[1] = v[0];
		v[0] = 0;
		for(q = p + 1; *q; q++)
			v[0] = v[0] * 10 + (pos_t)(*q - '0');
		*p = 0;
		n++;
	}
	if(n == 0)
		return 0;
	*line = v[0] ? v[0] - 1 : 0;
	*col = n == 2 && v[1] ? v[1] - 1 : 0;
	return 1;
}


/* --lex FILE [LANGUAGE]: each line of FILE, and under it the class of
 * each of its bytes, as the highlighting has them (its language as the
 * editor tells it, or the one named).  For who writes a language: what
 * it makes of a file, without the screen. */
static int
lex_file(const char *name, const char *lang)
{
	static const char cls_char[HL_COUNT] = { '.', 'K', 'T', 'S', 'C', 'N', 'P', 'D' };
	char cwd[PATH_MAX_], path[PATH_MAX_], *line = 0, *out = 0;
	unsigned char *cls = 0;
	pos_t ls, le, len, cap = 0;
	int id, state = 0, n, i, o;

	plat_cwd(cwd, sizeof cwd);
	path_join(path, cwd, name);
	path_normalize(path);
	if(file_kind(path) != 1 || !doc_open(path) || st.nerr) {
		plat_print(plat_name);
		plat_print(": cannot open ");
		plat_print(name);
		plat_print("\r\n");
		return 1;
	}
	id = lang ? lang_find(lang) : D->lang;
	if(lang && id == 0 && strcmp(lang, "text") != 0) {
		plat_print(plat_name);
		plat_print(": no language named ");
		plat_print(lang);
		plat_print("\r\n");
		return 1;
	}
	plat_print("# ");
	plat_print(name);
	plat_print(": ");
	plat_print(lang_name(id));
	plat_print(" (");
	plat_print(syns[id]->name);
	plat_print(")\r\n# . text  K keyword  T type, function  S string  C comment"
	    "  N number, constant  P preprocessor  D deleted\r\n");
	len = buf_len();
	for(ls = 0; ls < len; ls = le + 1) {
		le = buf_lineend(ls);
		n = (int)(le - ls > 1000000 ? 1000000 : le - ls);
		if((pos_t)n + 3 > cap) {
			cap = (pos_t)n + 3;
			mem_free(line);
			mem_free(out);
			mem_free(cls);
			line = mem_alloc(cap);
			out = mem_alloc(cap);
			cls = mem_alloc(cap);
			if(!line || !out || !cls) {
				plat_print("out of memory\r\n");
				return 1;
			}
		}
		buf_copy(ls, (pos_t)n, line);
		state = syn_line(id, line, n, state, cls);
		/* a letter for each character (its first byte), under it; a tab,
		   a control: shown as a blank */
		for(i = o = 0; i < n; i++) {
			if(((unsigned char)line[i] & 0xC0) != 0x80)
				out[o++] = cls_char[cls[i] < HL_COUNT ? cls[i] : 0];
			if((unsigned char)line[i] < 32)
				line[i] = ' ';
		}
		memcpy(line + n, "\n", 2);
		memcpy(out + o, "\n", 2);
		plat_print(line);
		plat_print(out);
	}
	mem_free(line);
	mem_free(out);
	mem_free(cls);
	return 0;
}

/* -1: on to the editor; else the code to exit with */
int
app_start(int argc, char **argv, int *cols, int *rows)
{
	char cwd[PATH_MAX_], path[PATH_MAX_];
	int i, opts = 1, gnext = 0, isgoto, n;
	pos_t line = 0, col = 0;

	st.menu = -1;
	settings_load();
	if(argc >= 3 && strcmp(argv[1], "--lex") == 0)
		return lex_file(argv[2], argc >= 4 ? argv[3] : 0);
	plat_cwd(cwd, sizeof cwd);
	strcpy(st.pdir, cwd);
	for(i = 1; i < argc; i++) {
		if(opts) {
			if(strcmp(argv[i], "--") == 0) {
				opts = gnext = 0;
				continue;
			}
			if(strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--goto") == 0) {
				gnext = 1;
				continue;
			}
			if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ||
			   strcmp(argv[i], "/?") == 0) {
				usage();
				return 0;
			}
			if(strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--foreground") == 0) {
				term_stay(1);
				continue;
			}
			if(strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-line-numbers") == 0) {
				no_line_numbers = 1;
				if(docs)
					D->linenums = 0;
				continue;
			}
			if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
				plat_print(plat_name);
				plat_print(" (Code Editor) version 1.0\r\n");
				return 0;
			}
		}
		isgoto = gnext && split_goto(argv[i], &line, &col);
		gnext = 0;
		path_join(path, cwd, argv[i]);
		path_normalize(path);
		/* a folder: the project's (File > Open Project Folder does the same) */
		if(file_kind(path) == 2) {
			project_set(path);
			continue;
		}
		if(doc_open(path)) {
			n = (int)(path_last(path) - path);
			memcpy(st.pdir, path, n);
			st.pdir[n] = 0;
			if(isgoto)
				cmd_goto(line, col);
		}
	}
	if(docs == 0)
		doc_new();
	project_update();	/* the folder started in, or the one given */
	term_set_font_size(settings_font_size());	/* the zoom, as it was */
	settings_use_font();
	if(!term_open(cols, rows)) {
		plat_print(plat_name);
		plat_print(term_font_size() ? ": the window cannot be made\r\n" :
		    ": the terminal cannot be used\r\n");
		return 1;
	}
	return -1;
}
