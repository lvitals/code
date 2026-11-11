#include <string.h>
#include "enc.h"

const char *const enc_names[ENC_N] = {
	"UTF-8", "UTF-8 BOM", "UTF-16 LE", "UTF-16 BE",
	"Windows-1252", "ISO-8859-1", "ISO-8859-15"
};

/* the characters of 80h-9Fh in Windows-1252 (the C1 controls where it
 * has none) */
static const unsigned short cp1252[32] = {
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
	0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
};

/* where ISO-8859-15 differs from ISO-8859-1: the byte, its character */
static const unsigned short latin9[8][2] = {
	{ 0xA4, 0x20AC }, { 0xA6, 0x0160 }, { 0xA8, 0x0161 }, { 0xB4, 0x017D },
	{ 0xB8, 0x017E }, { 0xBC, 0x0152 }, { 0xBD, 0x0153 }, { 0xBE, 0x0178 }
};

/* --- what a file is in --------------------------------------------------- */

int
enc_bom(int e, const char **bom)
{
	switch(e) {
	case ENC_UTF8_BOM:
		*bom = "\xEF\xBB\xBF";
		return 3;
	case ENC_UTF16LE:
		*bom = "\xFF\xFE";
		return 2;
	case ENC_UTF16BE:
		*bom = "\xFE\xFF";
		return 2;
	}
	*bom = "";
	return 0;
}

int
enc_sniff_bom(const char *p, pos_t n, int *skip)
{
	const unsigned char *u = (const unsigned char *)p;

	*skip = 0;
	if(n >= 3 && u[0] == 0xEF && u[1] == 0xBB && u[2] == 0xBF) {
		*skip = 3;
		return ENC_UTF8_BOM;
	}
	if(n >= 2 && u[0] == 0xFF && u[1] == 0xFE) {
		*skip = 2;
		return ENC_UTF16LE;
	}
	if(n >= 2 && u[0] == 0xFE && u[1] == 0xFF) {
		*skip = 2;
		return ENC_UTF16BE;
	}
	return ENC_AUTO;
}

int
enc_sniff_utf16(const char *p, pos_t n)
{
	pos_t i, pairs = n / 2, ze = 0, zo = 0;

	if(pairs < 2)
		return ENC_AUTO;
	for(i = 0; i < pairs * 2; i += 2) {
		ze += p[i] == 0;
		zo += p[i + 1] == 0;
	}
	/* text with most characters ASCII: one byte of each pair zero, the
	   other seldom (a low byte of 00h: Ā, Ȁ, ...) */
	if(zo * 10 >= pairs * 4 && ze * 10 <= zo)
		return ENC_UTF16LE;
	if(ze * 10 >= pairs * 4 && zo * 10 <= ze)
		return ENC_UTF16BE;
	return ENC_AUTO;
}

int
u8_check(struct u8check *c, const char *p, pos_t n, int last)
{
	const unsigned char *u = (const unsigned char *)p, *e = u + n;
	unsigned b;

	while(u < e) {
		b = *u;
		if(c->need) {
			if((int)b < c->lo || (int)b > c->hi)
				return 0;
			c->lo = 0x80;
			c->hi = 0xBF;
			c->need--;
			u++;
			continue;
		}
		if(b < 0x80) {	/* ASCII: eight at a time when they all are */
			while(e - u >= 8) {
				unsigned long long w;

				memcpy(&w, u, 8);
				if(w & 0x8080808080808080ULL)
					break;
				u += 8;
			}
			while(u < e && *u < 0x80)
				u++;
			continue;
		}
		c->lo = 0x80;
		c->hi = 0xBF;
		if(b >= 0xC2 && b <= 0xDF)
			c->need = 1;
		else if(b >= 0xE0 && b <= 0xEF) {
			c->need = 2;
			if(b == 0xE0)
				c->lo = 0xA0;	/* not overlong */
			else if(b == 0xED)
				c->hi = 0x9F;	/* not a surrogate */
		} else if(b >= 0xF0 && b <= 0xF4) {
			c->need = 3;
			if(b == 0xF0)
				c->lo = 0x90;
			else if(b == 0xF4)
				c->hi = 0x8F;	/* up to 10FFFFh */
		} else
			return 0;
		u++;
	}
	return !(last && c->need);
}

/* --- decoding ------------------------------------------------------------ */

static int
put8(char *s, unsigned long c)
{
	if(c < 0x80) {
		s[0] = (char)c;
		return 1;
	}
	if(c < 0x800) {
		s[0] = (char)(0xC0 | c >> 6);
		s[1] = (char)(0x80 | (c & 0x3F));
		return 2;
	}
	if(c < 0x10000) {
		s[0] = (char)(0xE0 | c >> 12);
		s[1] = (char)(0x80 | (c >> 6 & 0x3F));
		s[2] = (char)(0x80 | (c & 0x3F));
		return 3;
	}
	s[0] = (char)(0xF0 | c >> 18);
	s[1] = (char)(0x80 | (c >> 12 & 0x3F));
	s[2] = (char)(0x80 | (c >> 6 & 0x3F));
	s[3] = (char)(0x80 | (c & 0x3F));
	return 4;
}

static unsigned long
byte_char(int e, unsigned b)
{
	int i;

	if(b < 0x80)
		return b;
	if(e == ENC_CP1252 && b < 0xA0)
		return cp1252[b - 0x80];
	if(e == ENC_LATIN9)
		for(i = 0; i < 8; i++)
			if(latin9[i][0] == b)
				return latin9[i][1];
	return b;
}

/* a UTF-16 unit: a character out, or a surrogate kept */
static pos_t
unit16(struct dec *d, unsigned long u, char *out)
{
	pos_t o = 0;

	if(d->hi) {
		if(u >= 0xDC00 && u <= 0xDFFF) {
			o = put8(out, 0x10000 + ((d->hi - 0xD800) << 10) + (u - 0xDC00));
			d->hi = 0;
			return o;
		}
		o = put8(out, 0xFFFD);	/* a high surrogate alone */
		d->hi = 0;
	}
	if(u >= 0xD800 && u <= 0xDBFF)
		d->hi = u;
	else if(u >= 0xDC00 && u <= 0xDFFF)
		o += put8(out + o, 0xFFFD);
	else
		o += put8(out + o, u);
	return o;
}

pos_t
dec_run(struct dec *d, const char *in, pos_t n, char *out, int last)
{
	const unsigned char *u = (const unsigned char *)in, *e = u + n;
	pos_t o = 0;
	int be = d->enc == ENC_UTF16BE;

	if(!ENC_UTF16(d->enc)) {
		for(; u < e; u++) {
			if(*u < 0x80)
				out[o++] = (char)*u;
			else {
				if(*u < 0xA0)
					d->c1 = 1;
				o += put8(out + o, byte_char(d->enc, *u));
			}
		}
		return o;
	}
	if(d->npend && u < e) {
		d->pend[1] = *u++;
		o += unit16(d, be ? (unsigned long)d->pend[0] << 8 | d->pend[1] :
		    (unsigned long)d->pend[1] << 8 | d->pend[0], out + o);
		d->npend = 0;
	}
	for(; e - u >= 2; u += 2)
		o += unit16(d, be ? (unsigned long)u[0] << 8 | u[1] :
		    (unsigned long)u[1] << 8 | u[0], out + o);
	if(u < e) {
		d->pend[0] = *u;
		d->npend = 1;
	}
	if(last) {
		if(d->hi)
			o += put8(out + o, 0xFFFD);
		if(d->npend)
			o += put8(out + o, 0xFFFD);
		d->hi = 0;
		d->npend = 0;
	}
	return o;
}

/* --- encoding ------------------------------------------------------------ */

int
enc_has(int e, unsigned long c)
{
	int i;

	if(!ENC_8BIT(e) || c < 0x80)
		return 1;
	if(e == ENC_CP1252) {
		if(c >= 0xA0 && c <= 0xFF)
			return 1;
		for(i = 0; i < 32; i++)
			if(cp1252[i] == c)
				return 1;
		return 0;
	}
	if(e == ENC_LATIN9) {
		for(i = 0; i < 8; i++) {
			if(latin9[i][1] == c)
				return 1;
			if(latin9[i][0] == c)
				return 0;	/* its byte is another's */
		}
	}
	return c <= 0xFF;
}

static unsigned
char_byte(int e, unsigned long c)
{
	int i;

	if(e == ENC_CP1252 && c > 0xFF)
		for(i = 0; i < 32; i++)
			if(cp1252[i] == c)
				return 0x80 + i;
	if(e == ENC_LATIN9 && c > 0xFF)
		for(i = 0; i < 8; i++)
			if(latin9[i][1] == c)
				return latin9[i][0];
	return (unsigned)c & 0xFF;
}

/* the length of the character a lead byte starts */
static int
seqlen(unsigned b)
{
	return b < 0xC0 ? 1 : b < 0xE0 ? 2 : b < 0xF0 ? 3 : 4;
}

static pos_t
put_char(struct encd *e, unsigned long c, char *out)
{
	unsigned long h, l;

	if(ENC_8BIT(e->enc)) {
		out[0] = (char)char_byte(e->enc, c);
		return 1;
	}
	if(c >= 0x10000) {
		h = 0xD800 + ((c - 0x10000) >> 10);
		l = 0xDC00 + ((c - 0x10000) & 0x3FF);
		put_char(e, h, out);
		put_char(e, l, out + 2);
		return 4;
	}
	if(e->enc == ENC_UTF16BE) {
		out[0] = (char)(c >> 8);
		out[1] = (char)(c & 0xFF);
	} else {
		out[0] = (char)(c & 0xFF);
		out[1] = (char)(c >> 8);
	}
	return 2;
}

static unsigned long
get_char(const unsigned char *s, int n)
{
	unsigned long c;
	int i;

	if(n == 1)
		return s[0];
	c = s[0] & (0x7F >> n);
	for(i = 1; i < n; i++)
		c = c << 6 | (s[i] & 0x3F);
	return c;
}

pos_t
enc_run(struct encd *e, const char *in, pos_t n, char *out)
{
	const unsigned char *u = (const unsigned char *)in, *end = u + n;
	pos_t o = 0;
	int k;

	if(!ENC_8BIT(e->enc) && !ENC_UTF16(e->enc)) {
		memcpy(out, in, (size_t)n);
		return n;
	}
	while(e->npend && u < end) {	/* the end of a character cut short */
		e->pend[e->npend++] = *u++;
		if(e->npend == seqlen(e->pend[0])) {
			o += put_char(e, get_char(e->pend, e->npend), out + o);
			e->npend = 0;
		}
	}
	while(u < end) {
		if(*u < 0x80 && ENC_8BIT(e->enc)) {
			out[o++] = (char)*u++;
			continue;
		}
		k = seqlen(*u);
		if(end - u < k) {
			memcpy(e->pend, u, (size_t)(end - u));
			e->npend = (int)(end - u);
			break;
		}
		o += put_char(e, get_char(u, k), out + o);
		u += k;
	}
	return o;
}
