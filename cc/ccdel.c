#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include "cc.h"

int
main(int argc, char *argv[])
{
	char *end;
	long item;
	FILE *fp;
	int fd;
	CComp comp;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s ID\n", argv[0]);
		return 1;
	}

	errno = 0;
	item = strtol(argv[1], &end, 10);
	if(errno != 0 || *argv[1] == '\0' || *end != '\0' ||
	    item <= 0 || item > INT_MAX) {
		fprintf(stderr, "Invalid item number: %s\n", argv[1]);
		return 1;
	}

	fp = fopen("ccdb", "r+");
	if(fp == NULL) {
		perror("ccdb");
		return 2;
	}
	fd = fileno(fp);

	if(flock(fd, LOCK_EX) == -1) {
		perror("flock");
		fclose(fp);
		return 3;
	}

	if(fseek(fp, item * (long)sizeof(CComp), SEEK_SET) != 0 ||
	    fread(&comp, sizeof(CComp), 1, fp) != 1 ||
	    comp.id == 0) {
		fprintf(stderr, "Item not found\n");
		flock(fd, LOCK_UN);
		fclose(fp);
		return 4;
	}

	memset(&comp, 0, sizeof(comp));
	if(fseek(fp, item * (long)sizeof(CComp), SEEK_SET) != 0 ||
	    fwrite(&comp, sizeof(CComp), 1, fp) != 1 ||
	    fflush(fp) == EOF) {
		perror("ccdb");
		flock(fd, LOCK_UN);
		fclose(fp);
		return 5;
	}

	if(flock(fd, LOCK_UN) == -1) {
		perror("flock");
		fclose(fp);
		return 6;
	}
	if(fclose(fp) == EOF) {
		perror("ccdb");
		return 5;
	}
	return 0;
}
