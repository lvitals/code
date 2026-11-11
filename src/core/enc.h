/* enc: the encodings of a file.  The text is UTF-8 in the editor; a file
 * in another encoding is decoded when it is read and encoded when it is
 * written.
 *
 * What a file is in is guessed from its first bytes: a BOM (UTF-8,
 * UTF-16 LE or BE), else UTF-16 without one (a zero byte in each pair of
 * mostly ASCII text), else UTF-8 when its bytes are valid UTF-8, else an
 * 8-bit one: Windows-1252, or ISO-8859-1 when no byte is in 80h-9Fh (the
 * two differ there alone).  ISO-8859-15 is never guessed: it is chosen.
 *
 * The 8-bit decoders give each byte a character (the five bytes
 * Windows-1252 leaves out are the C1 controls), so a file read and
 * written back is the same bytes. */
#ifndef ENC_H
#define ENC_H

#include "../platform/platform.h"

enum {
	ENC_UTF8, ENC_UTF8_BOM, ENC_UTF16LE, ENC_UTF16BE,
	ENC_CP1252, ENC_LATIN1, ENC_LATIN9,
	ENC_N,
	ENC_AUTO = -1		/* buf_load: guessed */
};

extern const char *const enc_names[ENC_N];

#define ENC_8BIT(e)	((e) >= ENC_CP1252)
#define ENC_UTF16(e)	((e) == ENC_UTF16LE || (e) == ENC_UTF16BE)

/* the BOM of e (0 bytes for those without one) */
int enc_bom(int e, const char **bom);
/* the encoding a BOM at the start of p says (its length to *skip), or
 * ENC_AUTO when there is none */
int enc_sniff_bom(const char *p, pos_t n, int *skip);
/* UTF-16 without a BOM, guessed from the zero bytes: its encoding, or
 * ENC_AUTO */
int enc_sniff_utf16(const char *p, pos_t n);

/* UTF-8 checked a piece at a time: the bytes of a character may be split
 * between two pieces */
struct u8check {
	int need;		/* the continuation bytes still to come */
	int lo, hi;		/* what the next one may be */
};
/* 1 while the bytes are valid UTF-8; last: the end, where a character
 * cut short is not */
int u8_check(struct u8check *c, const char *p, pos_t n, int last);

/* a decoder: the bytes of a file to UTF-8, a piece at a time */
struct dec {
	int enc;
	unsigned char pend[2];	/* UTF-16: the byte of a pair cut short */
	int npend;
	unsigned long hi;	/* UTF-16: a high surrogate, waiting */
	int c1;			/* 8-bit: a byte in 80h-9Fh was seen */
};
/* out holds 3 * n + 8 bytes: what it got, its length; last: the end */
pos_t dec_run(struct dec *d, const char *in, pos_t n, char *out, int last);

/* an encoder: UTF-8 to the bytes of a file, a piece at a time (valid
 * UTF-8: buf_encodable checked it) */
struct encd {
	int enc;
	unsigned char pend[4];	/* a character cut short */
	int npend;
};
/* out holds 4 * n + 8 bytes: what it got, its length */
pos_t enc_run(struct encd *e, const char *in, pos_t n, char *out);
/* c can be written in e */
int enc_has(int e, unsigned long c);

#endif
