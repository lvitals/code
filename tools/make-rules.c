/* make-rules: the rules of the objects, in the Makefile (make rules).
 *
 *   make-rules Makefile src
 *
 * One rule for each .c under src, its object in build/obj (src/app/main.c:
 * build/obj/app/main.o), in the order of the paths as bytes:
 *
 *   build/obj/app/main.o: src/app/main.c
 *   	$(CCO) src/app/main.c
 *
 * A POSIX make has no rule to put an object in another folder (its .c.o
 * makes it next to its source), so each one has its own rule, written
 * between the two lines of the Makefile that mark them (what was there
 * goes).  src/platform/glfw is left out: its rules (the flags of GLFW,
 * stb's code without warnings) are written in the Makefile by hand.
 *
 * C99 and POSIX (dirent, rename), nothing else: the Makefile is made again
 * as Makefile.tmp, then renamed over it. */
#define _XOPEN_SOURCE 700
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define BEGIN	"# --- the objects: made by tools/make-rules (make rules) ---"
#define END	"# --- the end of the objects ---"
#define SKIP	"src/platform/glfw"
#define PATH_MAX_	1024
#define LINE_MAX_	4096

static char **paths;
static int npaths, cap;

static void
die(const char *msg, const char *name)
{
	fprintf(stderr, "make-rules: %s%s\n", msg, name ? name : "");
	exit(1);
}

static void
keep(const char *path)
{
	char **p;

	if(npaths == cap) {
		cap = cap ? cap * 2 : 256;
		if((p = realloc(paths, (size_t)cap * sizeof *paths)) == 0)
			die("out of memory", 0);
		paths = p;
	}
	if((paths[npaths] = malloc(strlen(path) + 1)) == 0)
		die("out of memory", 0);
	strcpy(paths[npaths++], path);
}

/* the .c files under dir, all the way down */
static void
walk(const char *dir)
{
	char path[PATH_MAX_];
	struct dirent *e;
	struct stat s;
	size_t n, nd = strlen(dir), ne;
	DIR *d;

	if(strcmp(dir, SKIP) == 0)
		return;
	if((d = opendir(dir)) == 0)
		die("cannot read the folder ", dir);
	while((e = readdir(d)) != 0) {
		if(e->d_name[0] == '.')
			continue;
		ne = strlen(e->d_name);
		if(nd + 1 + ne + 1 > sizeof path)
			die("a path too long in ", dir);
		memcpy(path, dir, nd);
		path[nd] = '/';
		memcpy(path + nd + 1, e->d_name, ne + 1);
		if(stat(path, &s) != 0)
			continue;
		n = strlen(path);
		if(S_ISDIR(s.st_mode))
			walk(path);
		else if(S_ISREG(s.st_mode) && n > 2 && strcmp(path + n - 2, ".c") == 0)
			keep(path);
	}
	closedir(d);
}

static int
by_bytes(const void *a, const void *b)
{
	return strcmp(*(char *const *)a, *(char *const *)b);
}

/* a line of f, without its newline: 0 at the end */
static int
line(FILE *f, char *s, int max)
{
	size_t n;

	if(fgets(s, max, f) == 0)
		return 0;
	n = strlen(s);
	if(n && s[n - 1] == '\n')
		s[--n] = 0;
	else if(!feof(f))
		die("a line too long in the Makefile", 0);
	return 1;
}

int
main(int argc, char **argv)
{
	char s[LINE_MAX_], tmp[PATH_MAX_];
	const char *make, *src;
	size_t k;
	FILE *in, *out;
	int i, state = 0;	/* 0 before the rules, 1 in them, 2 after */

	if(argc != 3)
		die("usage: make-rules Makefile src", 0);
	make = argv[1];
	src = argv[2];
	k = strlen(src);
	walk(src);
	qsort(paths, (size_t)npaths, sizeof *paths, by_bytes);
	if(strlen(make) + 5 > sizeof tmp)
		die("a path too long: ", make);
	sprintf(tmp, "%s.tmp", make);
	if((in = fopen(make, "r")) == 0)
		die("cannot read ", make);
	if((out = fopen(tmp, "w")) == 0)
		die("cannot write ", tmp);
	while(line(in, s, sizeof s)) {
		if(state == 1 && strcmp(s, END) != 0)
			continue;	/* the rules of before */
		if(state == 1)
			state = 2;
		fprintf(out, "%s\n", s);
		if(state == 0 && strcmp(s, BEGIN) == 0) {
			state = 1;
			for(i = 0; i < npaths; i++) {
				/* src/X.c: build/obj/X.o */
				fprintf(out, "build/obj%.*s.o: %s\n\t$(CCO) %s\n",
				    (int)(strlen(paths[i]) - k - 2), paths[i] + k, paths[i],
				    paths[i]);
			}
		}
	}
	fclose(in);
	if(ferror(out) | fclose(out) || state != 2) {
		remove(tmp);
		die(state == 2 ? "cannot write " : "no lines to mark the rules in ", make);
	}
	if(rename(tmp, make) != 0)
		die("cannot rename over ", make);
	return 0;
}
