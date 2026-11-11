/* The files under a folder, found a step at a time (finder.h): a stack
 * of the folders being read, each with its entries sorted, and the
 * ignore rules of the folders on the stack. */
#include <stdlib.h>
#include <string.h>
#include "finder.h"

#define RULES_FILE_MAX	(1 << 20)	/* a .gitignore longer is not read */

/* --- the files found ----------------------------------------------------- */

static char root[PATH_MAX_];
static char *paths;		/* the paths, each ended by a 0 */
static size_t pused, pcap;
static size_t *poff;		/* where each starts */
static int nfiles, fcap;

static int
add_file(const char *rel, size_t n)
{
	size_t *no;
	char *np;

	if(nfiles >= WALK_MAX)
		return 0;
	if(nfiles == fcap) {
		fcap = fcap ? fcap * 2 : 4096;
		if((no = realloc(poff, (size_t)fcap * sizeof *poff)) == 0)
			return 0;
		poff = no;
	}
	if(pused + n + 1 > pcap) {
		pcap = pcap ? pcap * 2 : 1 << 18;
		while(pused + n + 1 > pcap)
			pcap *= 2;
		if((np = realloc(paths, pcap)) == 0)
			return 0;
		paths = np;
	}
	poff[nfiles++] = pused;
	memcpy(paths + pused, rel, n + 1);
	pused += n + 1;
	return 1;
}

int
walk_count(void)
{
	return nfiles;
}

const char *
walk_path(int i)
{
	return paths + poff[i];
}

const char *
walk_root(void)
{
	return root;
}

/* --- the rules of .gitignore -------------------------------------------- */

int
glob_match(const char *p, const char *s)
{
	const char *q;
	int neg, ok;

	for(;;)
		switch(*p) {
		case 0:
			return *s == 0;
		case '*':
			if(p[1] == '*') {
				p += 2;
				if(*p == '/') {		/* no folder, or any */
					for(p++;; s++) {
						if(glob_match(p, s))
							return 1;
						if((s = strchr(s, '/')) == 0)
							return 0;
					}
				}
				for(;; s++) {		/* anything, folders too */
					if(glob_match(p, s))
						return 1;
					if(*s == 0)
						return 0;
				}
			}
			for(p++;; s++) {	/* anything in one name */
				if(glob_match(p, s))
					return 1;
				if(*s == 0 || *s == '/')
					return 0;
			}
		case '?':
			if(*s == 0 || *s == '/')
				return 0;
			p++;
			s++;
			break;
		case '[':
			q = p + 1;
			neg = *q == '!' || *q == '^';
			q += neg;
			for(ok = 0; *q && (*q != ']' || q == p + 1 + neg); )
				if(q[1] == '-' && q[2] && q[2] != ']') {
					ok |= (unsigned char)*s >= (unsigned char)q[0] &&
					    (unsigned char)*s <= (unsigned char)q[2];
					q += 3;
				} else
					ok |= *q++ == *s;
			if(*q != ']') {		/* no class: a [ itself */
				if(*s != '[')
					return 0;
				p++;
				s++;
				break;
			}
			if(*s == 0 || *s == '/' || ok == neg)
				return 0;
			p = q + 1;
			s++;
			break;
		case '\\':
			if(p[1])
				p++;
			/* fall through */
		default:
			if(*p != *s)
				return 0;
			p++;
			s++;
		}
}

struct rule {
	size_t pat;		/* in rtext */
	int base;		/* the length of the folder it is of */
	char neg;		/* !: what it matches is kept */
	char dir;		/* x/: folders only */
	char anchored;		/* a / in it: a path from its folder */
};

static char *rtext;
static size_t rused, rcap;
static struct rule *rules;
static int nrules, rulecap;

static void
add_rule(char *line, int base)
{
	struct rule r;
	size_t n;
	char *nt;
	struct rule *nr;

	n = strlen(line);
	while(n && line[n - 1] == ' ' && !(n > 1 && line[n - 2] == '\\'))
		line[--n] = 0;
	if(n == 0 || line[0] == '#')
		return;
	r.neg = line[0] == '!';
	if(r.neg)
		line++;
	else if(line[0] == '\\' && (line[1] == '!' || line[1] == '#'))
		line++;
	n = strlen(line);
	r.dir = n > 0 && line[n - 1] == '/';
	if(r.dir)
		line[--n] = 0;
	r.anchored = strchr(line, '/') != 0;
	if(line[0] == '/')
		line++;
	n = strlen(line);
	if(n == 0)
		return;
	if(rused + n + 1 > rcap) {
		rcap = rcap ? rcap * 2 : 4096;
		while(rused + n + 1 > rcap)
			rcap *= 2;
		if((nt = realloc(rtext, rcap)) == 0)
			return;
		rtext = nt;
	}
	if(nrules == rulecap) {
		rulecap = rulecap ? rulecap * 2 : 64;
		if((nr = realloc(rules, (size_t)rulecap * sizeof *rules)) == 0)
			return;
		rules = nr;
	}
	r.pat = rused;
	r.base = base;
	memcpy(rtext + rused, line, n + 1);
	rused += n + 1;
	rules[nrules++] = r;
}

/* the rules of the file at path, for the folder rel (its length: base) */
static void
read_rules(const char *path, int base)
{
	char *text, *line, *e;
	pos_t size;
	int h, n, k = 0;

	if((h = file_open(path, 0)) < 0)
		return;
	size = file_size(h);
	if(size == POS_NONE || size > RULES_FILE_MAX || (text = malloc((size_t)size + 1)) == 0) {
		file_close(h);
		return;
	}
	while(k < (int)size && (n = file_read(h, text + k, (int)size - k)) > 0)
		k += n;
	file_close(h);
	text[k] = 0;
	for(line = text; *line; line = e) {
		e = line + strcspn(line, "\n");
		if(*e)
			*e++ = 0;
		n = (int)strlen(line);
		if(n && line[n - 1] == '\r')
			line[n - 1] = 0;
		add_rule(line, base);
	}
	free(text);
}

/* rel (a path from the root) left out: the last rule that matches says,
 * those of the deeper folders first */
static int
ignored(const char *rel, int isdir)
{
	const char *base = strrchr(rel, '/');
	int i;

	base = base ? base + 1 : rel;
	for(i = nrules - 1; i >= 0; i--) {
		const struct rule *r = &rules[i];

		if(r->dir && !isdir)
			continue;
		if(glob_match(rtext + r->pat, r->anchored ? rel + r->base : base))
			return !r->neg;
	}
	return 0;
}

/* --- the folders being read ---------------------------------------------- */

struct frame {
	char *rel;		/* the folder from the root, ended by a '/' ("" the root) */
	char *names;		/* its entries, each ended by a 0 */
	int *ent;		/* where each starts, in the order of the names */
	char *kind;
	int n, at;
	int nrules;		/* the rules before its own */
	size_t rused;
};

static struct frame *stack;
static int depth, stackcap;
static const struct frame *sort_frame;	/* whose entries qsort orders */

static int
by_name(const void *a, const void *b)
{
	const struct frame *f = sort_frame;

	return strcmp(f->names + f->ent[*(const int *)a], f->names + f->ent[*(const int *)b]);
}

static void
pop(void)
{
	struct frame *f = &stack[--depth];

	nrules = f->nrules;
	rused = f->rused;
	free(f->rel);
	free(f->names);
	free(f->ent);
	free(f->kind);
}

/* the folder rel read: its entries, and its rules */
static void
push(const char *rel)
{
	char path[PATH_MAX_ + 256], name[256];
	struct frame *f, *ns;
	size_t used = 0, cap = 4096, n, rl = strlen(rel);
	int k, cnt = 0, entcap = 64, *order, *ne;
	char *nn, *nk;

	if(depth == stackcap) {
		stackcap = stackcap ? stackcap * 2 : 16;
		if((ns = realloc(stack, (size_t)stackcap * sizeof *stack)) == 0)
			return;
		stack = ns;
	}
	f = &stack[depth];
	memset(f, 0, sizeof *f);
	f->nrules = nrules;
	f->rused = rused;
	if((f->rel = malloc(rl + 1)) == 0 || (f->names = malloc(cap)) == 0 ||
	   (f->ent = malloc((size_t)entcap * sizeof *f->ent)) == 0 ||
	   (f->kind = malloc((size_t)entcap)) == 0) {
		free(f->rel);
		free(f->names);
		free(f->ent);
		return;		/* without memory: the folder is left out */
	}
	memcpy(f->rel, rel, rl + 1);
	depth++;
	if(strlen(root) + rl + 32 >= sizeof path)
		return;
	strcpy(path, root);
	strcat(path, rel);
	if(dir_open(path)) {
		while((k = dir_entry(name, (int)sizeof name)) != DK_END) {
			n = strlen(name);
			if(name[0] == '.' || k == DK_OTHER)	/* hidden: . .. too */
				continue;
			if(cnt == entcap) {
				entcap *= 2;
				if((ne = realloc(f->ent, (size_t)entcap * sizeof *ne)) == 0)
					break;
				f->ent = ne;
				if((nk = realloc(f->kind, (size_t)entcap)) == 0)
					break;
				f->kind = nk;
			}
			if(used + n + 1 > cap) {
				while(used + n + 1 > cap)
					cap *= 2;
				if((nn = realloc(f->names, cap)) == 0)
					break;
				f->names = nn;
			}
			memcpy(f->names + used, name, n + 1);
			f->ent[cnt] = (int)used;
			f->kind[cnt++] = (char)k;
			used += n + 1;
		}
		dir_close();
	}
	/* in the order of the names: indices sorted, then the entries and
	   their kinds taken in that order */
	if(cnt > 1 && (order = malloc((size_t)cnt * sizeof *order)) != 0) {
		if((ne = malloc((size_t)cnt * sizeof *ne)) != 0 && (nk = malloc((size_t)cnt)) != 0) {
			for(k = 0; k < cnt; k++)
				order[k] = k;
			sort_frame = f;
			qsort(order, (size_t)cnt, sizeof *order, by_name);
			for(k = 0; k < cnt; k++) {
				ne[k] = f->ent[order[k]];
				nk[k] = f->kind[order[k]];
			}
			free(f->ent);
			free(f->kind);
			f->ent = ne;
			f->kind = nk;
		} else
			free(ne);
		free(order);
	}
	f->n = cnt;
	/* its rules: they hold for what is under it */
	if(rl == 0) {
		strcpy(path, root);
		strcat(path, ".git/info/exclude");
		read_rules(path, 0);
	}
	strcpy(path, root);
	strcat(path, rel);
	k = (int)strlen(path);
	strcpy(path + k, ".gitignore");
	read_rules(path, (int)rl);
	strcpy(path + k, ".ignore");
	read_rules(path, (int)rl);
}

void
walk_start(const char *dir)
{
	walk_stop();
	strncpy(root, dir, sizeof root - 1);
	root[sizeof root - 1] = 0;
	push("");
}

int
walk_done(void)
{
	return depth == 0;
}

int
walk_step(int budget)
{
	char rel[PATH_MAX_];
	struct frame *f;
	const char *name;
	size_t n;

	while(budget-- > 0 && depth > 0) {
		f = &stack[depth - 1];
		if(f->at == f->n) {
			pop();
			continue;
		}
		name = f->names + f->ent[f->at];
		n = strlen(f->rel);
		if(n + strlen(name) + 2 > sizeof rel) {
			f->at++;
			continue;
		}
		memcpy(rel, f->rel, n);
		strcpy(rel + n, name);
		if(f->kind[f->at++] == DK_DIR) {
			if(!ignored(rel, 1)) {
				strcat(rel, "/");
				push(rel);	/* f is not to be used after it */
			}
		} else if(!ignored(rel, 0) && !add_file(rel, strlen(rel)))
			while(depth > 0)	/* full: the walk ends */
				pop();
	}
	return depth > 0;
}

void
walk_stop(void)
{
	while(depth > 0)
		pop();
	free(stack);
	stack = 0;
	stackcap = 0;
	free(paths);
	free(poff);
	paths = 0;
	poff = 0;
	pused = pcap = 0;
	nfiles = fcap = 0;
	free(rtext);
	free(rules);
	rtext = 0;
	rules = 0;
	rused = rcap = 0;
	nrules = rulecap = 0;
}
