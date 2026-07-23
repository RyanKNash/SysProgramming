#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include "cc.h"

static int
read_line(char *buffer, size_t size)
{
	int ch;
	size_t length;

	if(fgets(buffer, size, stdin) == NULL)
		return -1;
	length = strlen(buffer);
	if(length > 0 && buffer[length - 1] == '\n') {
		buffer[length - 1] = '\0';
		return 0;
	}

	while((ch = getchar()) != '\n' && ch != EOF)
		;
	return 0;
}

static int
prompt_string(const char *label, char *value, size_t size)
{
	char input[Descsz];

	printf("%s [%s]: ", label, value);
	fflush(stdout);
	if(read_line(input, sizeof(input)) == -1)
		return -1;
	if(input[0] != '\0') {
		strncpy(value, input, size - 1);
		value[size - 1] = '\0';
	}
	return 0;
}

static int
prompt_int(const char *label, int *value)
{
	char input[128];
	char *end;
	long number;

	for(;;) {
		printf("%s [%d]: ", label, *value);
		fflush(stdout);
		if(read_line(input, sizeof(input)) == -1)
			return -1;
		if(input[0] == '\0')
			return 0;

		errno = 0;
		number = strtol(input, &end, 10);
		if(errno == 0 && *end == '\0' &&
		    number >= INT_MIN && number <= INT_MAX) {
			*value = (int)number;
			return 0;
		}
		fprintf(stderr, "Please enter a valid integer, or press Enter "
		    "to keep the current value.\n");
	}
}

static int
parse_item(const char *text, long *item)
{
	char *end;

	errno = 0;
	*item = strtol(text, &end, 10);
	if(errno != 0 || *text == '\0' || *end != '\0' ||
	    *item <= 0 || *item > INT_MAX)
		return -1;
	return 0;
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int fd;
	long item;
	long offset;
	CComp comp;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s ID\n", argv[0]);
		return 1;
	}
	if(parse_item(argv[1], &item) == -1) {
		fprintf(stderr, "Invalid item number: %s\n", argv[1]);
		return 1;
	}
	offset = item * (long)sizeof(CComp);

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

	if(fseek(fp, offset, SEEK_SET) != 0 ||
	    fread(&comp, sizeof(comp), 1, fp) != 1 ||
	    comp.id == 0) {
		fprintf(stderr, "Item not found\n");
		flock(fd, LOCK_UN);
		fclose(fp);
		return 4;
	}

	printf("Press Enter without typing a value to keep the current value.\n");
	if(prompt_string("Name", comp.name, sizeof(comp.name)) == -1 ||
	    prompt_int("Year", &comp.year) == -1 ||
	    prompt_string("Maker", comp.maker, sizeof(comp.maker)) == -1 ||
	    prompt_string("CPU", comp.cpu, sizeof(comp.cpu)) == -1 ||
	    prompt_int("Memory", &comp.memory) == -1 ||
	    prompt_string("Description", comp.desc, sizeof(comp.desc)) == -1) {
		fprintf(stderr, "\nInput ended; item was not changed\n");
		flock(fd, LOCK_UN);
		fclose(fp);
		return 5;
	}

	if(fseek(fp, offset, SEEK_SET) != 0 ||
	    fwrite(&comp, sizeof(comp), 1, fp) != 1 ||
	    fflush(fp) == EOF) {
		perror("ccdb");
		flock(fd, LOCK_UN);
		fclose(fp);
		return 6;
	}

	if(flock(fd, LOCK_UN) == -1) {
		perror("flock");
		fclose(fp);
		return 7;
	}
	if(fclose(fp) == EOF) {
		perror("ccdb");
		return 6;
	}
	return 0;
}
