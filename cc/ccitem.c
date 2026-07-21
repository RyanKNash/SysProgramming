#include <stdio.h>
#include <stdlib.h>
#include "cc.h"

int
main(int argc, char *argv[]) {
	int id;
	FILE *fp;
	CComp comp;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s ID\n", argv[0]);
		exit(1);
	}
	id = atoi(argv[1]);
	fp = fopen("ccdb", "r");
	if(fp == NULL) {
		fprintf(stderr, "No DB\n");
		exit(2);
	}
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
	fclose(fp);
}