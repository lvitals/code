/* enc_data: the tables of the encodings beyond the editor's own seven
 * (enc.h), in enc_data.c, made by tools/enc-tables.c (make enc-tables)
 * from the iconv of glibc once, and kept in the sources: the editor
 * needs no library to read and write them.
 *
 * A character is 16 bits in a table (pool): ENCD_NONE, no character
 * there; ENCD_X, one beyond 16 bits, or two (Big5-HKSCS), in the
 * exceptions; ENCD_LEAD (the bytes alone of a multibyte one), the first
 * byte of a longer one.
 *
 * A byte sequence, as a key: its length << 24 | its bytes, the first the
 * highest. */
#ifndef ENC_DATA_H
#define ENC_DATA_H

#define ENCD_NONE	0xFFFF
#define ENCD_X		0xFFFE
#define ENCD_LEAD	0xFFFE

#define ENCD_KEY(n, b)	((unsigned long)(n) << 24 | (unsigned long)(b))

/* one byte a character: 256 of the pool from sb */
struct encd_sb {
	const char *label;
	unsigned sb;
	unsigned d0, nd;	/* decode only (encd_donly): not written so */
};

/* the bytes after a prefix (its first one or two), lo to hi: from off in
 * the pool */
struct encd_row {
	unsigned char npre, pre0, pre1, lo, hi;
	unsigned off;
};

/* exception: the character (and a second, or 0) of a key */
struct encd_x {
	unsigned long key, c, c2;
};

/* several bytes a character: the bytes alone (256 of the pool from sb:
 * a character, ENCD_LEAD or ENCD_NONE), the rows (by npre, pre0, pre1) */
struct encd_mb {
	const char *label;
	unsigned sb;
	unsigned r0, nr;	/* encd_rows */
	unsigned x0, nx;	/* encd_x, by key */
	unsigned d0, nd;	/* encd_donly, by key */
};

/* GB18030, four bytes: from the linear index idx (of 81 30 81 30), the
 * characters of the BMP from c, one after the other (to the next);
 * ENCD_GBNONE, none (U+FFFF is one of them: not ENCD_NONE) */
#define ENCD_GBNONE	0xFFFFFFFFUL
struct encd_gbr {
	unsigned long idx, c;
};

extern const unsigned short encd_pool[];
extern const struct encd_sb encd_sb[];
extern const int encd_nsb;
extern const struct encd_row encd_rows[];
extern const struct encd_x encd_x[];
extern const struct encd_mb encd_mb[];
extern const int encd_nmb;
extern const unsigned long encd_donly[];
extern const struct encd_gbr encd_gb18030[];
extern const int encd_ngb18030;

#endif
