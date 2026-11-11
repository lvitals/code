#include <stdlib.h>
#include <string.h>
#include "regex.h"

#define STEPS		2000000L	/* the most work for one start */
#define DEPTH		20000		/* the deepest recursion */

enum { N_CHAR, N_ANY, N_SET, N_BOL, N_EOL, N_WORDB, N_NWORDB, N_GROUP };

struct alt;

struct node {
	struct node *next;	/* in its sequence */
	struct node *all;	/* every node, to free them */
	char type, lazy;
	unsigned char c;
	unsigned char set[32];
	int min, max;		/* max < 0: no limit */
	struct alt *alts;	/* of a group */
	int cap;		/* its group, or -1 */
};

struct alt {
	struct node *first;
	struct alt *next, *all;
};

struct re {
	struct alt *top;
	struct node *nodes;
	struct alt *alts;
	int nocase, ncap;
	pos_t groups[2 * RE_GROUPS];
	pos_t end;
	long steps;
	int depth;
	const unsigned char *mem;	/* the text: these bytes (re_find_mem), */
	pos_t mlen;			/* or, when 0, the buffer B */
};

/* the text being matched: B, or the bytes given */
#define TLEN(r)		((r)->mem ? (r)->mlen : buf_len())
#define TCH(r, i)	((r)->mem ? (int)(r)->mem[i] : BCH(i))

/* the next character, the one before (a byte not UTF-8 alone) */
static pos_t
tnext(struct re *r, pos_t p)
{
	pos_t i = p + 1, len;

	if(!r->mem)
		return buf_next(p);
	len = r->mlen;
	if(p >= len)
		return len;
	if(r->mem[p] >= 0xC0)
		while(i < len && i < p + 4 && (r->mem[i] & 0xC0) == 0x80)
			i++;
	return i;
}

static pos_t
tprev(struct re *r, pos_t p)
{
	pos_t i;

	if(!r->mem)
		return buf_prev(p);
	if(p == 0)
		return 0;
	for(i = p - 1; i > 0 && i + 3 > p && (r->mem[i] & 0xC0) == 0x80; i--)
		;
	return tnext(r, i) == p ? i : p - 1;
}

/* --- compiling ----------------------------------------------------------- */

struct parse {
	struct re *r;
	const char *p;
	const char *err;
};

static struct node *
newnode(struct parse *ps, int type)
{
	struct node *n = calloc(1, sizeof *n);

	if(n == 0) {
		ps->err = "Out of memory";
		return 0;
	}
	n->type = (char)type;
	n->min = n->max = 1;
	n->cap = -1;
	n->all = ps->r->nodes;
	ps->r->nodes = n;
	return n;
}

static void
setbit(unsigned char *set, int c)
{
	set[(c & 0xFF) >> 3] |= (unsigned char)(1 << (c & 7));
}

static int
hasbit(const unsigned char *set, int c)
{
	return set[(c & 0xFF) >> 3] >> (c & 7) & 1;
}

static void
setclass(unsigned char *set, int k)
{
	int c;

	for(c = 0; c < 256; c++)
		if((k == 'd' && c >= '0' && c <= '9') ||
		   (k == 'w' && isword(c) && c < 0x80) || (k == 'w' && c >= 0x80) ||
		   (k == 's' && (c == ' ' || (c >= '\t' && c <= '\r'))))
			setbit(set, c);
}

static void
invert(unsigned char *set)
{
	int i;

	for(i = 0; i < 32; i++)
		set[i] = (unsigned char)~set[i];
}

/* the character of an escape (after \) */
static int
escaped(int c)
{
	switch(c) {
	case 'n': return '\n';
	case 't': return '\t';
	case 'r': return '\r';
	case 'f': return '\f';
	case 'v': return '\v';
	case '0': return 0;
	}
	return c;
}

static int
hexval(int c)
{
	return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 :
	    c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

static void
setnocase(unsigned char *set)
{
	int c;

	for(c = 'A'; c <= 'Z'; c++)
		if(hasbit(set, c) || hasbit(set, c + 32)) {
			setbit(set, c);
			setbit(set, c + 32);
		}
}

static struct alt *parse_alts(struct parse *ps);

/* [...] */
static struct node *
parse_set(struct parse *ps)
{
	struct node *n = newnode(ps, N_SET);
	int neg = 0, c, d, first = 1;

	if(n == 0)
		return 0;
	if(*ps->p == '^') {
		neg = 1;
		ps->p++;
	}
	for(;;) {
		c = (unsigned char)*ps->p;
		if(c == 0) {
			ps->err = "Missing ]";
			return 0;
		}
		ps->p++;
		if(c == ']' && !first)
			break;
		first = 0;
		if(c == '\\' && *ps->p) {
			c = (unsigned char)*ps->p++;
			if(strchr("dwsDWS", c)) {
				unsigned char t[32];

				memset(t, 0, sizeof t);
				setclass(t, c | 0x20);
				if(c < 'a')
					invert(t);
				for(d = 0; d < 32; d++)
					n->set[d] |= t[d];
				continue;
			}
			c = escaped(c);
		}
		if(*ps->p == '-' && ps->p[1] && ps->p[1] != ']') {
			ps->p++;
			d = (unsigned char)*ps->p++;
			if(d == '\\' && *ps->p)
				d = escaped((unsigned char)*ps->p++);
			if(d < c) {
				ps->err = "Bad range in [ ]";
				return 0;
			}
			for(; c <= d; c++)
				setbit(n->set, c);
			continue;
		}
		setbit(n->set, c);
	}
	if(ps->r->nocase)
		setnocase(n->set);
	if(neg) {
		invert(n->set);
		n->set['\n' >> 3] &= (unsigned char)~(1 << ('\n' & 7));	/* not a newline */
	}
	return n;
}

static int
number(struct parse *ps, int *v)
{
	int k = 0;

	if(*ps->p < '0' || *ps->p > '9')
		return 0;
	*v = 0;
	while(*ps->p >= '0' && *ps->p <= '9' && k++ < 6)
		*v = *v * 10 + (*ps->p++ - '0');
	return 1;
}

static struct node *
parse_atom(struct parse *ps)
{
	struct node *n;
	int c = (unsigned char)*ps->p++, h1, h2;

	switch(c) {
	case '.':
		return newnode(ps, N_ANY);
	case '^':
		return newnode(ps, N_BOL);
	case '$':
		return newnode(ps, N_EOL);
	case '[':
		return parse_set(ps);
	case '(':
		if((n = newnode(ps, N_GROUP)) == 0)
			return 0;
		if(ps->p[0] == '?' && ps->p[1] == ':')
			ps->p += 2;
		else if(ps->r->ncap < RE_GROUPS - 1)
			n->cap = ++ps->r->ncap;
		n->alts = parse_alts(ps);
		if(ps->err)
			return 0;
		if(*ps->p != ')') {
			ps->err = "Missing )";
			return 0;
		}
		ps->p++;
		return n;
	case ')':
		ps->err = "Unmatched )";
		return 0;
	case '*': case '+': case '?':
		ps->err = "Nothing to repeat";
		return 0;
	case '\\':
		c = (unsigned char)*ps->p;
		if(c == 0) {
			ps->err = "Trailing \\";
			return 0;
		}
		ps->p++;
		if(c == 'b' || c == 'B')
			return newnode(ps, c == 'b' ? N_WORDB : N_NWORDB);
		if(strchr("dwsDWS", c)) {
			if((n = newnode(ps, N_SET)) == 0)
				return 0;
			setclass(n->set, c | 0x20);
			if(c < 'a')
				invert(n->set);
			return n;
		}
		if(c == 'x' && (h1 = hexval(ps->p[0])) >= 0 && (h2 = hexval(ps->p[1])) >= 0) {
			ps->p += 2;
			c = h1 * 16 + h2;
		} else
			c = escaped(c);
		break;
	}
	if(ps->r->nocase && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
		if((n = newnode(ps, N_SET)) == 0)
			return 0;
		setbit(n->set, c);
		setnocase(n->set);
		return n;
	}
	if((n = newnode(ps, N_CHAR)) == 0)
		return 0;
	n->c = (unsigned char)c;
	return n;
}

/* a sequence, to | or ) */
static struct node *
parse_seq(struct parse *ps)
{
	struct node *first = 0, **tail = &first, *n;
	const char *save;
	int a, b;

	while(*ps->p && *ps->p != '|' && *ps->p != ')') {
		if((n = parse_atom(ps)) == 0)
			return 0;
		/* its repetition */
		for(;;) {
			if(*ps->p == '*' || *ps->p == '+' || *ps->p == '?') {
				a = *ps->p == '+';
				b = *ps->p == '?' ? 1 : -1;
				ps->p++;
			} else if(*ps->p == '{') {
				save = ps->p++;
				if(!number(ps, &a)) {
					ps->p = save;	/* a { of its own */
					break;
				}
				b = a;
				if(*ps->p == ',') {
					ps->p++;
					if(!number(ps, &b))
						b = -1;
				}
				if(*ps->p != '}' || (b >= 0 && b < a)) {
					ps->p = save;
					break;
				}
				ps->p++;
			} else
				break;
			if(n->type == N_BOL || n->type == N_EOL || n->type == N_WORDB ||
			   n->type == N_NWORDB) {
				ps->err = "Nothing to repeat";
				return 0;
			}
			n->min = a;
			n->max = b;
			if(*ps->p == '?') {
				n->lazy = 1;
				ps->p++;
			}
			if(*ps->p == '*' || *ps->p == '+' || *ps->p == '{') {
				ps->err = "Repeated twice";
				return 0;
			}
			break;
		}
		*tail = n;
		tail = &n->next;
	}
	return first;
}

static struct alt *
parse_alts(struct parse *ps)
{
	struct alt *first = 0, **tail = &first, *a;

	for(;;) {
		if((a = calloc(1, sizeof *a)) == 0) {
			ps->err = "Out of memory";
			return 0;
		}
		a->all = ps->r->alts;
		ps->r->alts = a;
		a->first = parse_seq(ps);
		if(ps->err)
			return 0;
		*tail = a;
		tail = &a->next;
		if(*ps->p != '|')
			return first;
		ps->p++;
	}
}

void
re_free(struct re *r)
{
	struct node *n, *nn;
	struct alt *a, *na;

	if(r == 0)
		return;
	for(n = r->nodes; n; n = nn) {
		nn = n->all;
		free(n);
	}
	for(a = r->alts; a; a = na) {
		na = a->all;
		free(a);
	}
	free(r);
}

struct re *
re_compile(const char *pat, int nocase, const char **err)
{
	struct parse ps;
	struct re *r = calloc(1, sizeof *r);

	*err = 0;
	if(r == 0) {
		*err = "Out of memory";
		return 0;
	}
	r->nocase = nocase;
	ps.r = r;
	ps.p = pat;
	ps.err = 0;
	r->top = parse_alts(&ps);
	if(!ps.err && *ps.p)
		ps.err = "Unmatched )";
	if(ps.err) {
		*err = ps.err;
		re_free(r);
		return 0;
	}
	return r;
}

/* --- matching ------------------------------------------------------------ */

/* what follows the end of a sequence in a group: one iteration more done */
struct frame {
	struct node *g;
	int count;
	pos_t start;
	struct frame *up;
};

static int m(struct re *r, struct node *n, pos_t p, struct frame *k);

/* one character of node n at p: past it, or POS_NONE */
static pos_t
one(struct re *r, struct node *n, pos_t p)
{
	pos_t len = TLEN(r);
	int c;

	if(p >= len)
		return POS_NONE;
	c = TCH(r, p);
	switch(n->type) {
	case N_CHAR:
		return c == n->c ? p + 1 : POS_NONE;
	case N_ANY:
		return c == '\n' ? POS_NONE : tnext(r, p);
	case N_SET:
		if(!hasbit(n->set, c))
			return POS_NONE;
		return c >= 0x80 ? tnext(r, p) : p + 1;
	}
	return POS_NONE;
}

static int
zero_width(struct re *r, struct node *n, pos_t p)
{
	pos_t len = TLEN(r);
	int a, b;

	switch(n->type) {
	case N_BOL:
		return p == 0 || TCH(r, p - 1) == '\n';
	case N_EOL:	/* before CR LF too: a file searched as it is */
		return p >= len || TCH(r, p) == '\n' ||
		    (TCH(r, p) == '\r' && p + 1 < len && TCH(r, p + 1) == '\n');
	default:
		a = p > 0 && isword(TCH(r, p - 1));
		b = p < len && isword(TCH(r, p));
		return (a != b) == (n->type == N_WORDB);
	}
}

/* iteration count + 1 of group g at p (or what follows it) */
static int
m_group(struct re *r, struct node *g, int count, pos_t p, struct frame *k)
{
	struct frame f;
	struct alt *a;
	int more = g->max < 0 || count < g->max, stop = count >= g->min;
	pos_t save = 0;

	if(g->lazy && stop && m(r, g->next, p, k))
		return 1;
	if(more) {
		if(g->cap >= 0)
			save = r->groups[2 * g->cap];
		for(a = g->alts; a; a = a->next) {
			f.g = g;
			f.count = count + 1;
			f.start = p;
			f.up = k;
			if(g->cap >= 0)
				r->groups[2 * g->cap] = p;
			if(m(r, a->first, p, &f))
				return 1;
		}
		if(g->cap >= 0)
			r->groups[2 * g->cap] = save;
	}
	if(!g->lazy && stop)
		return m(r, g->next, p, k);
	return 0;
}

static int
m(struct re *r, struct node *n, pos_t p, struct frame *k)
{
	struct node *g;
	pos_t q, save;
	int c, ok;

	if(++r->steps > STEPS || r->depth > DEPTH)
		return 0;
	r->depth++;
	if(n == 0) {	/* the end of a sequence */
		if(k == 0) {
			r->end = p;
			r->depth--;
			return 1;
		}
		g = k->g;
		save = g->cap >= 0 ? r->groups[2 * g->cap + 1] : 0;
		if(g->cap >= 0)
			r->groups[2 * g->cap + 1] = p;
		if(p == k->start && k->count > g->min)	/* empty: no more of it */
			ok = m(r, g->next, p, k->up);
		else
			ok = m_group(r, g, k->count, p, k->up);
		if(!ok && g->cap >= 0)
			r->groups[2 * g->cap + 1] = save;
		r->depth--;
		return ok;
	}
	switch(n->type) {
	case N_BOL: case N_EOL: case N_WORDB: case N_NWORDB:
		ok = zero_width(r, n, p) && m(r, n->next, p, k);
		break;
	case N_GROUP:
		ok = m_group(r, n, 0, p, k);
		break;
	default:	/* a character repeated: in a loop */
		c = 0;
		q = p;
		if(n->lazy) {
			ok = 0;
			for(;;) {
				if(c >= n->min && m(r, n->next, q, k)) {
					ok = 1;
					break;
				}
				if((n->max >= 0 && c >= n->max) || (q = one(r, n, q)) == POS_NONE)
					break;
				c++;
			}
			break;
		}
		while((n->max < 0 || c < n->max) && (save = one(r, n, q)) != POS_NONE) {
			q = save;
			c++;
		}
		ok = 0;
		for(; c >= n->min; c--) {
			if(m(r, n->next, q, k)) {
				ok = 1;
				break;
			}
			if(c == 0)
				break;
			q = n->type == N_CHAR || (q > p && TCH(r, q - 1) < 0x80) ? q - 1 : tprev(r, q);
			if(++r->steps > STEPS)
				break;
		}
		break;
	}
	r->depth--;
	return ok;
}

int
re_match(struct re *r, pos_t off, pos_t *end, pos_t *groups)
{
	struct node g;
	int i;

	for(i = 0; i < 2 * RE_GROUPS; i++)
		r->groups[i] = POS_NONE;
	memset(&g, 0, sizeof g);
	g.type = N_GROUP;
	g.min = g.max = 1;
	g.alts = r->top;
	g.cap = 0;
	r->steps = 0;
	r->depth = 0;
	if(!m_group(r, &g, 0, off, 0))
		return 0;
	r->groups[0] = off;
	r->groups[1] = r->end;
	*end = r->end;
	if(groups)
		memcpy(groups, r->groups, sizeof r->groups);
	return 1;
}

/* the first match that starts in [from, to); work: the steps it may take
 * (0: no limit), -1 when they ran out (*at: the start to go on from) */
static int
find(struct re *r, pos_t from, pos_t to, long work, pos_t *at, pos_t *end, pos_t *groups)
{
	struct node *f = r->top && !r->top->next ? r->top->first : 0;
	pos_t p, len = TLEN(r);
	long done = 0;

	if(to > len)
		to = len;
	for(p = from; p < to || (p == to && to == len && p == from); p++) {
		/* a first character that must be there: the others skipped */
		if(f && f->min > 0 && f->type == N_CHAR && p < len && TCH(r, p) != f->c)
			continue;
		if(f && f->min > 0 && f->type == N_SET && p < len && !hasbit(f->set, TCH(r, p)))
			continue;
		if(f && f->type == N_BOL && p > 0 && TCH(r, p - 1) != '\n')
			continue;
		if(work && done > work) {
			*at = p;
			return -1;
		}
		if(re_match(r, p, end, groups)) {
			*at = p;
			return 1;
		}
		done += r->steps;
		if(p >= len)
			break;
	}
	return 0;
}

int
re_find(struct re *r, pos_t from, pos_t to, pos_t *at, pos_t *end, pos_t *groups)
{
	return find(r, from, to, 0, at, end, groups);
}

int
re_find_mem(struct re *r, const char *p, pos_t n, pos_t from, pos_t to, long work,
    pos_t *at, pos_t *end)
{
	pos_t groups[2 * RE_GROUPS];
	int ok;

	r->mem = (const unsigned char *)p;
	r->mlen = n;
	ok = find(r, from, to, work, at, end, groups);
	r->mem = 0;
	return ok;
}

int
re_match_mem(struct re *r, const char *p, pos_t n, pos_t off, pos_t *end)
{
	pos_t groups[2 * RE_GROUPS];
	int ok;

	r->mem = (const unsigned char *)p;
	r->mlen = n;
	ok = re_match(r, off, end, groups);
	r->mem = 0;
	return ok;
}

long
re_work(const struct re *r)
{
	return r->steps;
}
