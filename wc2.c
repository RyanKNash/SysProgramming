#include <stdio.h>

void dowc(FILE *);

int
main(int argc, char *argv[]) {
	FILE *fp;

	fp = stdin;
	if(argc > 1)
		fp = fopen(argv[1], "r");
	dowc(fp);
}

void
dowc(FILE *fp) {
	int nlines, nwords, nchars;
	int c;
	int inword;

	nlines = 0;
	nwords = 0;
	nchars = 0;
	inword = 0;
	while(1) {
		c = getc(fp);
		if(c == EOF) {
			break;
		}
		if(c == '\n') {
			++nlines;
		}
		++nchars;
		if(inword) {
			if(c == ' ' || c == '\t' || c == '\n') {
				inword = 0;
			}
		}
		else {
			if(c != ' ' && c != '\t' && c != '\n') {
				inword = 1;
				++nwords;
			}
		}
	}
	printf("%d\t%d\t%d\n", nlines, nwords, nchars);
}