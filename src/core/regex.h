/* regex: regular expressions over the text (B), in C99 alone.
 *
 *   .  [abc] [^a-z]  \d \w \s \D \W \S  \b \B  ^ $ (of a line)
 *   * + ? {m} {m,} {m,n}, and *? +? ?? {..}? (as few as can be)
 *   ( ) groups (1 to 9 captured), (?: ) not captured, a|b
 *   \n \t \r \. and the like: the character itself
 *
 * A match is looked for by backtracking; a repeated character (or class,
 * or .) is matched in a loop, not by recursion, so .* over a long line
 * costs no stack. */
#ifndef REGEX_H
#define REGEX_H

#include "buf.h"

#define RE_GROUPS	10	/* the whole match, then 1 to 9 */

struct re;

/* 0 and a message at *err when pat is not one */
struct re *re_compile(const char *pat, int nocase, const char **err);
void re_free(struct re *r);
/* a match that starts at off: its end, and the groups (start, end) */
int re_match(struct re *r, pos_t off, pos_t *end, pos_t *groups);
/* the first match that starts in [from, to): at *at, ending at *end */
int re_find(struct re *r, pos_t from, pos_t to, pos_t *at, pos_t *end, pos_t *groups);
/* the same over the n bytes at p, not the text B: a struct re matches one
 * text at a time (each thread its own, re_compile'd).  work: the steps it
 * may take (0: no limit); -1 when they ran out, *at the start to go on
 * from (a thread then sees whether its search is still wanted) */
int re_find_mem(struct re *r, const char *p, pos_t n, pos_t from, pos_t to, long work,
    pos_t *at, pos_t *end);
int re_match_mem(struct re *r, const char *p, pos_t n, pos_t off, pos_t *end);
long re_work(const struct re *r);	/* the steps the last match took */

#endif
