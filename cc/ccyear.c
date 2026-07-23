#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include "cc.h"

static int
parse_year(const char *text, int *year)
{
	char *end;
	long value;

	errno = 0;
	value = strtol(text, &end, 10);
	if(errno != 0 || *text == '\0' || *end != '\0' ||
	    value < INT_MIN || value > INT_MAX)
		return -1;
	*year = (int)value;
	return 0;
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int fd;
	int first;
	int last;
	CComp comp;

	if(argc != 3) {
		fprintf(stderr, "Usage: %s first-year last-year\n", argv[0]);
		return 1;
	}
	if(parse_year(argv[1], &first) == -1 ||
	    parse_year(argv[2], &last) == -1) {
		fprintf(stderr, "Invalid year\n");
		return 1;
	}
	if(first > last) {
		fprintf(stderr, "First year must not be after last year\n");
		return 1;
	}

	fp = fopen("ccdb", "r");
	if(fp == NULL) {
		perror("ccdb");
		return 2;
	}
	fd = fileno(fp);

	if(flock(fd, LOCK_SH) == -1) {
		perror("flock");
		fclose(fp);
		return 3;
	}

	if(fseek(fp, sizeof(CComp), SEEK_SET) != 0) {
		perror("ccdb");
		flock(fd, LOCK_UN);
		fclose(fp);
		return 4;
	}

	while(fread(&comp, sizeof(CComp), 1, fp) == 1) {
		if(comp.id != 0 && comp.year >= first && comp.year <= last) {
			printf("ID: %d\n", comp.id);
			printf("Name: %s\n", comp.name);
			printf("Year: %d\n", comp.year);
			printf("Maker: %s\n", comp.maker);
			printf("CPU: %s\n", comp.cpu);
			printf("Memory: %d\n", comp.memory);
			printf("Description: %s\n", comp.desc);
			putchar('\n');
		}
	}

	if(ferror(fp)) {
		perror("ccdb");
		flock(fd, LOCK_UN);
		fclose(fp);
		return 4;
	}
	if(flock(fd, LOCK_UN) == -1) {
		perror("flock");
		fclose(fp);
		return 5;
	}
	if(fclose(fp) == EOF) {
		perror("ccdb");
		return 4;
	}
	return 0;
}
