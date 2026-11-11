/* A query matched against a path (finder.h).  Each word of the query is
 * looked for in the path: from its end back to find where it can start
 * last (so a match in the name of the file is found before one in its
 * folders), then forward from there for the shortest span, which is
 * scored. */
#include <string.h>
#include "finder.h"

#define WORDS		8
#define WORD_MAX	64

static char word[WORDS][WORD_MAX];
static int wlen[WORDS], wcase[WORDS], nwords;

/* what a letter scores, and what is added to it */
enum {
	SC_MATCH = 16,
	SC_START = 10,		/* at the start of the path */
	SC_FOLDER = 9,		/* after a '/' */
	SC_WORD = 7,		/* after _ - . or a blank, or a capital after a small one */
	SC_ALNUM = 5,		/* a letter or digit after another sign */
	SC_NEXT = 8,		/* right after the one before */
	SC_GAP = 3,		/* letters skipped: the first of them */
	SC_GAPMORE = 1,		/* each of the others */
	SC_NAME = 20		/* the whole word in the name of the file */
};

static int
lower(int c)
{
	return c >= 'A' && c <= 'Z' ? c + 32 : c;
}

static int
isalnum_(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
	    c >= 0x80;
}

int
fz_set(const char *q)
{
	int n, k, cs;

	nwords = 0;
	for(;;) {
		while(*q == ' ')
			q++;
		if(*q == 0 || nwords == WORDS)
			break;
		for(n = 0, cs = 0; q[n] && q[n] != ' '; n++)
			cs |= q[n] >= 'A' && q[n] <= 'Z';
		k = n < WORD_MAX - 1 ? n : WORD_MAX - 1;
		memcpy(word[nwords], q, (size_t)k);
		word[nwords][k] = 0;
		wlen[nwords] = k;
		wcase[nwords++] = cs;
		q += n;
	}
	return nwords;
}

static int
same(int a, int b, int cs)
{
	return cs ? a == b : lower(a) == lower(b);
}

/* the bonus of a match at s[i] */
static int
bonus(const unsigned char *s, int i)
{
	int p, c = s[i];

	if(i == 0)
		return SC_START;
	p = s[i - 1];
	if(p == '/')
		return SC_FOLDER;
	if(p == '_' || p == '-' || p == '.' || p == ' ')
		return SC_WORD;
	if(p >= 'a' && p <= 'z' && c >= 'A' && c <= 'Z')
		return SC_WORD;
	if(!isalnum_(p) && isalnum_(c))
		return SC_ALNUM;
	return 0;
}

/* word w in s: where its shortest span starts and ends (the last letter
 * included), 0 if it is not there */
static int
span(int w, const unsigned char *s, int n, int *start, int *end)
{
	const char *p = word[w];
	int k = wlen[w], i, j, cs = wcase[w];

	/* back from the end: the latest start */
	for(i = n - 1, j = k - 1; i >= 0 && j >= 0; i--)
		if(same(s[i], (unsigned char)p[j], cs))
			j--;
	if(j >= 0)
		return 0;
	*start = i + 1;
	/* forward from it: the earliest end */
	for(i = *start, j = 0; j < k; i++)
		if(same(s[i], (unsigned char)p[j], cs))
			j++;
	*end = i - 1;
	/* back from that end: the latest start, the span the shortest */
	for(i = *end, j = k - 1; j >= 0; i--)
		if(same(s[i], (unsigned char)p[j], cs))
			j--;
	*start = i + 1;
	return 1;
}

/* the score of word w over [start, end] of s; mark: the bytes it takes */
static int
score_span(int w, const unsigned char *s, int start, int end, int name, unsigned char *mark)
{
	const char *p = word[w];
	int i, j = 0, sc = 0, prev = -2, gap = 0, cs = wcase[w];

	for(i = start; i <= end && j < wlen[w]; i++) {
		if(!same(s[i], (unsigned char)p[j], cs)) {
			sc -= gap++ ? SC_GAPMORE : SC_GAP;
			continue;
		}
		gap = 0;
		sc += SC_MATCH + bonus(s, i) + (prev == i - 1 ? SC_NEXT : 0);
		prev = i;
		j++;
		if(mark)
			mark[i] = 1;
	}
	if(start >= name)
		sc += SC_NAME;
	return sc;
}

static int
match(const char *str, int n, unsigned char *mark)
{
	const unsigned char *s = (const unsigned char *)str;
	int w, start, end, sc = 0, name;

	for(name = n; name > 0 && s[name - 1] != '/'; name--)
		;
	for(w = 0; w < nwords; w++) {
		if(!span(w, s, n, &start, &end))
			return -1;
		sc += score_span(w, s, start, end, name, mark);
	}
	return sc > 0 ? sc : 0;	/* -1 is for no match */
}

int
fz_score(const char *s, int n)
{
	return nwords ? match(s, n, 0) : 0;
}

void
fz_marks(const char *s, int n, unsigned char *mark)
{
	memset(mark, 0, (size_t)n);
	if(nwords)
		match(s, n, mark);
}
