/* bin2c: a file as a C array, for the window's font to be in the program.
 *
 *   bin2c NAME FILE > out.c
 *
 * gives  const unsigned char NAME[] = { ... };
 *        const unsigned long NAME_size = ...; */
#include <stdio.h>

int
main(int argc, char **argv)
{
	FILE *f;
	unsigned long n = 0;
	int c;

	if(argc != 3) {
		fprintf(stderr, "usage: bin2c NAME FILE\n");
		return 2;
	}
	if((f = fopen(argv[2], "rb")) == 0) {
		perror(argv[2]);
		return 1;
	}
	printf("/* %s, made by tools/bin2c */\n", argv[2]);
	printf("const unsigned char %s[] = {", argv[1]);
	while((c = getc(f)) != EOF) {
		printf("%s%d,", n % 20 == 0 ? "\n" : "", c);
		n++;
	}
	printf("\n};\nconst unsigned long %s_size = %lu;\n", argv[1], n);
	fclose(f);
	return 0;
}
