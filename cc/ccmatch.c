#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include "cc.h"

int
main(int argc, char *argv[])
{
	FILE *fp;
	int fd;
	CComp comp;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s string\n", argv[0]);
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
		if(comp.id != 0 &&
		    (strstr(comp.name, argv[1]) != NULL ||
		     strstr(comp.maker, argv[1]) != NULL ||
		     strstr(comp.cpu, argv[1]) != NULL ||
		     strstr(comp.desc, argv[1]) != NULL)) {
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
