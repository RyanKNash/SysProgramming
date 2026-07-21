#include <stdio.h>
#include <stdlib.h>
#include "cc.h"
#include <sys/file.h>

int
main(int argc, char *argv[]) {
	int id;
	int fd;
	FILE *fp;
	CComp comp;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s ID\n", argv[0]);
		exit(1);
	}
	
	id = atoi(argv[1]);

	fp = fopen("ccdb", "r");
	if(fp == NULL) {
		exit(2);
	}

	fd = fileno(fp);

	flock(fd, LOCK_SH);

	fseek(fp, id * sizeof(CComp), SEEK_SET);
	if(fread(&comp, sizeof(CComp), 1, fp) != 1) {
		fprintf(stderr, "No record\n");
		exit(3);
	}
	if(comp.id == 0) {
		fprintf(stderr, "Item not found\n");
		exit(4);
	}
	printf("ID: %d\n", comp.id);
	printf("Name: %s\n", comp.name);
	printf("Year: %d\n", comp.year);
	printf("Maker: %s\n", comp.maker);
	printf("CPU: %s\n", comp.cpu);
	printf("Memory: %d\n", comp.memory);
	printf("Description: %s\n", comp.desc);

	flock(fd, LOCK_UN);
	fclose(fp);
	return 0;
}