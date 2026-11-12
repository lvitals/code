/* enc-tables: the tables of the encodings (src/core/enc_data.h), asked
 * of the iconv of glibc once, as C (make enc-tables):
 *
 *   enc-tables > src/core/enc_data.c
 *
 * The editor does not use iconv: it reads these tables, kept in the
 * sources.  Only this tool needs it, to make them again.
 *
 * Each byte, then each byte after a first one that is not a character
 * alone, then (EUC-JP) each after two, is decoded; what decodes is a
 * character of the table.  One that iconv writes otherwise (a second
 * sequence for a character: cp932's NEC and IBM ones) is marked decode
 * only.  The rows the same as one already in the pool, or a part of one,
 * are not added again: GB2312 is in GBK, EUC-KR in cp949, Big5 in cp950.
 * GB18030's four bytes are kept as runs of the BMP (from the linear
 * index): the rest is computed. */
#include <errno.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/core/enc_data.h"

/* the label (as the editor shows it), iconv's name */
static const char *const sbs[][2] = {
	{ "US-ASCII", "ASCII" },
	{ "ISO-8859-2", "ISO-8859-2" }, { "ISO-8859-3", "ISO-8859-3" },
	{ "ISO-8859-4", "ISO-8859-4" }, { "ISO-8859-5", "ISO-8859-5" },
	{ "ISO-8859-6", "ISO-8859-6" }, { "ISO-8859-7", "ISO-8859-7" },
	{ "ISO-8859-8", "ISO-8859-8" }, { "ISO-8859-9", "ISO-8859-9" },
	{ "ISO-8859-10", "ISO-8859-10" }, { "ISO-8859-11", "ISO-8859-11" },
	{ "ISO-8859-13", "ISO-8859-13" }, { "ISO-8859-14", "ISO-8859-14" },
	{ "ISO-8859-16", "ISO-8859-16" },
	{ "IBM850", "IBM850" }, { "cp851", "IBM851" }, { "IBM857", "IBM857" },
	{ "IBM00858", "IBM858" }, { "IBM860", "IBM860" }, { "IBM861", "IBM861" },
	{ "IBM862", "IBM862" }, { "IBM863", "IBM863" }, { "IBM864", "IBM864" },
	{ "IBM865", "IBM865" }, { "IBM866", "IBM866" }, { "IBM868", "IBM868" },
	{ "IBM869", "IBM869" },
	{ "KOI8-R", "KOI8-R" }, { "KOI8-U", "KOI8-U" }, { "KOI8-RU", "KOI8-RU" },
	{ "windows-1250", "CP1250" }, { "windows-1251", "CP1251" },
	{ "windows-1253", "CP1253" }, { "windows-1254", "CP1254" },
	{ "windows-1255", "CP1255" }, { "windows-1256", "CP1256" },
	{ "windows-1257", "CP1257" }, { "windows-1258", "CP1258" },
	{ "macintosh", "MACINTOSH" }, { "TIS-620", "TIS-620" },
	{ "windows-874", "CP874" },
	{ "IBM037", "IBM037" }, { "IBM273", "IBM273" }, { "IBM277", "IBM277" },
	{ "IBM278", "IBM278" }, { "IBM280", "IBM280" }, { "IBM284", "IBM284" },
	{ "IBM285", "IBM285" }, { "IBM297", "IBM297" }, { "IBM420", "IBM420" },
	{ "IBM424", "IBM424" }, { "IBM437", "IBM437" }, { "IBM500", "IBM500" },
	{ "IBM775", "IBM775" }, { "IBM870", "IBM870" }, { "IBM871", "IBM871" },
	{ "IBM918", "IBM918" }, { "IBM1026", "IBM1026" }, { "IBM1047", "IBM1047" },
	{ "IBM01140", "IBM1140" }, { "IBM01141", "IBM1141" },
	{ "IBM01142", "IBM1142" }, { "IBM01143", "IBM1143" },
	{ "IBM01144", "IBM1144" }, { "IBM01145", "IBM1145" },
	{ "IBM01146", "IBM1146" }, { "IBM01147", "IBM1147" },
	{ "IBM01148", "IBM1148" }, { "IBM01149", "IBM1149" },
	{ "x-mac-cyrillic", "MACCYRILLIC" }, { "x-mac-ce", "MAC-CENTRALEUROPE" },
	{ "x-mac-ukrainian", "MACUKRAINIAN" },
	{ "ARMSCII-8", "ARMSCII-8" }, { "Georgian-PS", "GEORGIAN-PS" },
	{ "PT154", "PT154" }, { "VISCII", "VISCII" }, { "TCVN", "TCVN5712-1" },
	{ "cp737", "CP737" }, { "cp856", "CP856" }, { "cp1125", "CP1125" }
};
static const char *const mbs[][2] = {
	{ "Shift_JIS", "SHIFT_JIS" }, { "windows-31j", "CP932" },
	{ "EUC-JP", "EUC-JP" },
	{ "GB2312", "EUC-CN" }, { "GBK", "GBK" }, { "GB18030", "GB18030" },
	{ "Big5", "BIG5" }, { "windows-950", "CP950" }, { "Big5-HKSCS", "BIG5-HKSCS" },
	{ "EUC-KR", "EUC-KR" }, { "windows-949", "CP949" }, { "JOHAB", "JOHAB" }
};
#define NSB	(int)(sizeof sbs / sizeof sbs[0])
#define NMB	(int)(sizeof mbs / sizeof mbs[0])

static iconv_t dcd, ecd;	/* to UTF-32LE, from it */

static void
die(const char *s, const char *name)
{
	fprintf(stderr, "enc-tables: %s%s\n", s, name);
	exit(1);
}

static void
open_cds(const char *name)
{
	if(dcd)
		iconv_close(dcd);
	if(ecd)
		iconv_close(ecd);
	if((dcd = iconv_open("UTF-32LE", name)) == (iconv_t)-1 ||
	   (ecd = iconv_open(name, "UTF-32LE")) == (iconv_t)-1)
		die("iconv has no ", name);
}

/* n bytes decoded: 1, the characters to c (*nc); 0 cut short; -1 none */
static int
decode(const unsigned char *b, int n, unsigned long *c, int *nc)
{
	char *in = (char *)b, *o;
	unsigned char out[64];
	size_t il = (size_t)n, ol = sizeof out;
	int i;

	iconv(dcd, 0, 0, 0, 0);
	o = (char *)out;
	if(iconv(dcd, &in, &il, &o, &ol) == (size_t)-1)
		return errno == EINVAL ? 0 : -1;
	iconv(dcd, 0, 0, &o, &ol);
	*nc = (int)((sizeof out - ol) / 4);
	if(il || *nc == 0 || *nc > 2)
		return -1;
	for(i = 0; i < *nc; i++)
		c[i] = (unsigned long)out[4 * i] | (unsigned long)out[4 * i + 1] << 8 |
		    (unsigned long)out[4 * i + 2] << 16 | (unsigned long)out[4 * i + 3] << 24;
	return 1;
}

/* the characters c written back are the n bytes b */
static int
roundtrip(const unsigned long *c, int nc, const unsigned char *b, int n)
{
	unsigned char in[8], out[32];
	char *ip = (char *)in, *o = (char *)out;
	size_t il = 4 * (size_t)nc, ol = sizeof out;
	int i;

	for(i = 0; i < nc; i++) {
		in[4 * i] = (unsigned char)c[i];
		in[4 * i + 1] = (unsigned char)(c[i] >> 8);
		in[4 * i + 2] = (unsigned char)(c[i] >> 16);
		in[4 * i + 3] = (unsigned char)(c[i] >> 24);
	}
	iconv(ecd, 0, 0, 0, 0);
	if(iconv(ecd, &ip, &il, &o, &ol) == (size_t)-1)
		return 0;
	iconv(ecd, 0, 0, &o, &ol);
	return (int)(sizeof out - ol) == n && memcmp(out, b, (size_t)n) == 0;
}

/* --- what is written ----------------------------------------------------- */

static unsigned short *pool;
static size_t npool, cpool;
static struct encd_row *rows;
static size_t nrows, crows;
static struct encd_x *xs;
static size_t nxs, cxs;
static unsigned long *donly;
static size_t ndonly, cdonly;

static void *
grow(void *p, size_t *cap, size_t need, size_t size)
{
	if(need <= *cap)
		return p;
	*cap = need * 2 + 64;
	if((p = realloc(p, *cap * size)) == 0)
		die("out of memory", "");
	return p;
}

/* n values in the pool (where they are already, if they are): where */
static unsigned
pool_add(const unsigned short *v, size_t n)
{
	size_t i;

	for(i = 0; i + n <= npool; i++)
		if(pool[i] == v[0] && memcmp(pool + i, v, n * sizeof *v) == 0)
			return (unsigned)i;
	pool = grow(pool, &cpool, npool + n, sizeof *pool);
	memcpy(pool + npool, v, n * sizeof *v);
	npool += n;
	return (unsigned)(npool - n);
}

static void
add_x(unsigned long key, const unsigned long *c, int nc)
{
	xs = grow(xs, &cxs, nxs + 1, sizeof *xs);
	xs[nxs].key = key;
	xs[nxs].c = c[0];
	xs[nxs++].c2 = nc > 1 ? c[1] : 0;
}

static void
add_donly(unsigned long key)
{
	donly = grow(donly, &cdonly, ndonly + 1, sizeof *donly);
	donly[ndonly++] = key;
}

/* the value of n bytes b in a table (an exception, decode only, kept) */
static unsigned short
value(const unsigned char *b, int n, int *r)
{
	unsigned long c[2], key = 0;
	int nc, i;

	*r = decode(b, n, c, &nc);
	if(*r <= 0)
		return ENCD_NONE;
	for(i = 0; i < n; i++)
		key = key << 8 | b[i];
	key = ENCD_KEY(n, key);
	if(!roundtrip(c, nc, b, n))
		add_donly(key);
	if(nc == 1 && c[0] < 0xFFFE)
		return (unsigned short)c[0];
	add_x(key, c, nc);
	return ENCD_X;
}

static int
by_key(const void *a, const void *b)
{
	unsigned long x = *(const unsigned long *)a, y = *(const unsigned long *)b;

	return x < y ? -1 : x > y;
}

static int
by_xkey(const void *a, const void *b)
{
	return by_key(&((const struct encd_x *)a)->key, &((const struct encd_x *)b)->key);
}

/* the row of the bytes after the prefix pre (n bytes): 0 if none is
 * there (those cut short, the prefix of a longer one, are none) */
static int
row(const unsigned char *pre, int n, int gb)
{
	unsigned short v[256];
	unsigned char b[4];
	int t, lo = 256, hi = -1, r;

	memcpy(b, pre, (size_t)n);
	for(t = 0; t < 256; t++) {
		v[t] = ENCD_NONE;
		if(gb && n == 1 && t >= 0x30 && t <= 0x39)
			continue;	/* GB18030's four bytes: computed */
		b[n] = (unsigned char)t;
		v[t] = value(b, n + 1, &r);
		if(v[t] != ENCD_NONE) {
			if(t < lo)
				lo = t;
			hi = t;
		}
	}
	if(hi < 0)
		return 0;
	rows = grow(rows, &crows, nrows + 1, sizeof *rows);
	rows[nrows].npre = (unsigned char)n;
	rows[nrows].pre0 = pre[0];
	rows[nrows].pre1 = n > 1 ? pre[1] : 0;
	rows[nrows].lo = (unsigned char)lo;
	rows[nrows].hi = (unsigned char)hi;
	rows[nrows++].off = pool_add(v + lo, (size_t)(hi - lo + 1));
	return 1;
}

/* the 256 bytes alone: where in the pool; lead: those that start more */
static unsigned
bytes_alone(char *lead)
{
	unsigned short v[256];
	unsigned char b;
	int t, r;

	for(t = 0; t < 256; t++) {
		b = (unsigned char)t;
		v[t] = value(&b, 1, &r);
		if(v[t] == ENCD_X)
			die("a byte beyond the BMP", "");
		lead[t] = r == 0;
		if(lead[t])
			v[t] = ENCD_LEAD;
	}
	return pool_add(v, 256);
}

static void
print_u16(const char *name, const unsigned short *v, size_t n)
{
	size_t i;

	printf("const unsigned short %s[%lu] = {", name, (unsigned long)(n ? n : 1));
	for(i = 0; i < n; i++)
		printf("%s0x%04X%s", i % 10 ? " " : "\n\t", v[i], i + 1 < n ? "," : "");
	if(n == 0)
		printf("0");
	printf("\n};\n\n");
}

int
main(void)
{
	static unsigned sboff[NSB], sbd[NSB][2];
	static unsigned mbsb[NMB], mbr[NMB][2], mbx[NMB][2], mbd[NMB][2];
	unsigned long gbidx[1024], gbc[1024], c[2], prev = 0, i, idx;
	unsigned char pre[4];
	char lead[256];
	int e, t, u, nc, ngb = 0;
	size_t d0, x0;

	for(e = 0; e < NSB; e++) {
		open_cds(sbs[e][1]);
		d0 = ndonly;
		sboff[e] = bytes_alone(lead);
		for(t = 0; t < 256; t++)
			if(lead[t])
				die("a lead byte in ", sbs[e][0]);
		qsort(donly + d0, ndonly - d0, sizeof *donly, by_key);
		sbd[e][0] = (unsigned)d0;
		sbd[e][1] = (unsigned)(ndonly - d0);
	}
	for(e = 0; e < NMB; e++) {
		int gb = strcmp(mbs[e][0], "GB18030") == 0;

		open_cds(mbs[e][1]);
		d0 = ndonly;
		x0 = nxs;
		mbsb[e] = bytes_alone(lead);
		mbr[e][0] = (unsigned)nrows;
		for(t = 0; t < 256; t++) {
			if(!lead[t])
				continue;
			pre[0] = (unsigned char)t;
			row(pre, 1, gb);
		}
		/* three bytes: the second of those cut short (EUC-JP's 8F) */
		for(t = 0; t < 256; t++) {
			if(!lead[t])
				continue;
			for(u = 0; u < 256; u++) {
				if(gb && u >= 0x30 && u <= 0x39)
					continue;
				pre[0] = (unsigned char)t;
				pre[1] = (unsigned char)u;
				if(decode(pre, 2, c, &nc) == 0)
					row(pre, 2, 0);
			}
		}
		mbr[e][1] = (unsigned)(nrows - mbr[e][0]);
		qsort(donly + d0, ndonly - d0, sizeof *donly, by_key);
		qsort(xs + x0, nxs - x0, sizeof *xs, by_xkey);
		mbx[e][0] = (unsigned)x0;
		mbx[e][1] = (unsigned)(nxs - x0);
		mbd[e][0] = (unsigned)d0;
		mbd[e][1] = (unsigned)(ndonly - d0);
		if(!gb)
			continue;
		/* the four bytes of the BMP: runs */
		for(idx = 0; idx < 39420; idx++) {
			i = idx;
			pre[3] = (unsigned char)(0x30 + i % 10);
			i /= 10;
			pre[2] = (unsigned char)(0x81 + i % 126);
			i /= 126;
			pre[1] = (unsigned char)(0x30 + i % 10);
			pre[0] = (unsigned char)(0x81 + i / 10);
			if(decode(pre, 4, c, &nc) <= 0 || nc != 1)
				c[0] = ENCD_GBNONE;
			if(idx == 0 || c[0] != prev + 1 || c[0] == ENCD_GBNONE) {
				if(ngb > 0 && gbc[ngb - 1] == ENCD_GBNONE && c[0] == ENCD_GBNONE) {
					prev = c[0];
					continue;
				}
				if(ngb == 1024)
					die("too many runs in GB18030", "");
				gbidx[ngb] = idx;
				gbc[ngb++] = c[0];
			}
			prev = c[0];
		}
	}

	printf("/* Made by tools/enc-tables.c (make enc-tables), from the iconv of\n"
	    " * glibc: not to be edited.  What it is: enc_data.h. */\n"
	    "#include \"enc_data.h\"\n\n");
	print_u16("encd_pool", pool, npool);
	printf("const struct encd_sb encd_sb[%d] = {\n", NSB);
	for(e = 0; e < NSB; e++)
		printf("\t{ \"%s\", %u, %u, %u }%s\n", sbs[e][0], sboff[e], sbd[e][0], sbd[e][1],
		    e + 1 < NSB ? "," : "");
	printf("};\nconst int encd_nsb = %d;\n\n", NSB);
	printf("const struct encd_row encd_rows[%lu] = {\n", (unsigned long)nrows);
	for(i = 0; i < nrows; i++)
		printf("\t{ %u, 0x%02X, 0x%02X, 0x%02X, 0x%02X, %u }%s\n", rows[i].npre, rows[i].pre0,
		    rows[i].pre1, rows[i].lo, rows[i].hi, rows[i].off, i + 1 < nrows ? "," : "");
	printf("};\n\n");
	printf("const struct encd_x encd_x[%lu] = {\n", (unsigned long)(nxs ? nxs : 1));
	for(i = 0; i < nxs; i++)
		printf("\t{ 0x%08lXUL, 0x%05lXUL, 0x%05lXUL }%s\n", xs[i].key, xs[i].c, xs[i].c2,
		    i + 1 < nxs ? "," : "");
	if(nxs == 0)
		printf("\t{ 0, 0, 0 }\n");
	printf("};\n\n");
	printf("const struct encd_mb encd_mb[%d] = {\n", NMB);
	for(e = 0; e < NMB; e++)
		printf("\t{ \"%s\", %u, %u, %u, %u, %u, %u, %u }%s\n", mbs[e][0], mbsb[e],
		    mbr[e][0], mbr[e][1], mbx[e][0], mbx[e][1], mbd[e][0], mbd[e][1],
		    e + 1 < NMB ? "," : "");
	printf("};\nconst int encd_nmb = %d;\n\n", NMB);
	printf("const unsigned long encd_donly[%lu] = {", (unsigned long)(ndonly ? ndonly : 1));
	for(i = 0; i < ndonly; i++)
		printf("%s0x%08lXUL%s", i % 6 ? " " : "\n\t", donly[i], i + 1 < ndonly ? "," : "");
	if(ndonly == 0)
		printf("0");
	printf("\n};\n\n");
	printf("const struct encd_gbr encd_gb18030[%d] = {\n", ngb ? ngb : 1);
	for(t = 0; t < ngb; t++)
		printf("\t{ %lu, 0x%04lXUL }%s\n", gbidx[t], gbc[t], t + 1 < ngb ? "," : "");
	if(ngb == 0)
		printf("\t{ 0, 0 }\n");
	printf("};\nconst int encd_ngb18030 = %d;\n", ngb);
	return 0;
}
