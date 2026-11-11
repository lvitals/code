/* The jump list: where the cursor was before each jump (Go to Line, a
 * search, Ctrl+Home and Ctrl+End, Go to Matching Bracket, Jump to Mark, a
 * click far away, another document), to go back to (Alt+Left) and forward
 * again (Alt+Right).  The places move with the edits of their text; a
 * document closed leaves the list. */
#include <string.h>
#include "app.h"

#define JMAX	100

static struct jump {
	struct doc *d;
	pos_t at;
} jl[JMAX];
static int jn;		/* the places */
static int ji;		/* the one gone to last; jn: none, the newest is next */
static char going;	/* going to one: the document it activates is no jump */

static int
same_line(const struct jump *j, struct doc *d, pos_t at)
{
	struct buf *o = B;
	int r;

	if(j->d != d)
		return 0;
	B = &d->b;
	r = buf_linestart(j->at > buf_len() ? buf_len() : j->at) ==
	    buf_linestart(at > buf_len() ? buf_len() : at);
	B = o;
	return r;
}

static void
put(struct doc *d, pos_t at)
{
	if(jn > 0 && same_line(&jl[jn - 1], d, at)) {
		jl[jn - 1].at = at;	/* the same line: the newest place on it */
		return;
	}
	if(jn == JMAX) {
		memmove(jl, jl + 1, (JMAX - 1) * sizeof jl[0]);
		jn--;
	}
	jl[jn].d = d;
	jl[jn++].at = at;
}

void
jump_push(void)
{
	if(docs == 0 || going)
		return;
	jn = ji < jn ? ji + 1 : jn;	/* a new way: the places ahead go */
	put(D, D->cur);
	ji = jn;
}

static void
go(struct jump *j)
{
	if(j->d != D) {
		going = 1;
		doc_activate(j->d);
		going = 0;
	}
	D->cur = j->at > buf_len() ? buf_len() : j->at;
	D->anchor = POS_NONE;
	D->pref = -1;
	cursor_visible();
	rows_stale = 1;
	ui_again = 1;
}

void
jump_go(int dir)
{
	int k;

	if(docs == 0)
		return;
	if(dir < 0 && ji == jn) {	/* from the newest: here, to come back to */
		put(D, D->cur);
		ji = jn - 1;
	}
	/* the next place on another line than the cursor's */
	for(k = ji + dir; k >= 0 && k < jn && same_line(&jl[k], D, D->cur); k += dir)
		;
	if(k < 0 || k >= jn)
		return;
	ji = k;
	go(&jl[ji]);
}

/* off..off+ndel became n bytes in the active document */
void
jump_adjust(pos_t off, pos_t ndel, pos_t n)
{
	int i;

	for(i = 0; i < jn; i++)
		if(jl[i].d == D && jl[i].at >= off)
			jl[i].at = jl[i].at < off + ndel ? off : jl[i].at - ndel + n;
}

/* document d closes: its places go */
void
jump_forget(struct doc *d)
{
	int i, k = 0, nji = ji;

	for(i = 0; i < jn; i++) {
		if(jl[i].d == d) {
			if(i < ji)
				nji--;
			continue;
		}
		jl[k++] = jl[i];
	}
	jn = k;
	ji = nji > jn ? jn : nji < 0 ? 0 : nji;
}
