#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include "cc.h"
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	CComp newcomp;
	FILE *fp;
	int fd;
	struct stat file_info;

	if (argc != 8)
	{
		fprintf(stderr,
				"Usage: %s id name year maker cpu memory desc\n",
				argv[0]);
		exit(1);
	}

	fp = fopen("ccdb", "r+");
	if (fp == NULL)
	{
		fp = fopen("ccdb", "w");
		if (fp == NULL)
		{
			perror("ccdb");
			exit(2);
		}
	}

	fd = fileno(fp);

	if (flock(fd, LOCK_EX) == -1)
	{
		perror("flock");
		fclose(fp);
		exit(3);
	}

	if (strcmp(argv[1], "-a") == 0)
	{
		if (fstat(fd, &file_info) == -1)
		{
			perror("fstat");
			flock(fd, LOCK_UN);
			fclose(fp);
			exit(4);
		}

		newcomp.id = file_info.st_size / sizeof(CComp);

		if (newcomp.id == 0)
		{
			newcomp.id = 1;
		}
	}
	else
	{
		newcomp.id = atoi(argv[1]);
	}

	strncpy(newcomp.name, argv[2], 15);
	newcomp.name[15] = 0;
	newcomp.year = atoi(argv[3]);
	strncpy(newcomp.maker, argv[4], 15);
	newcomp.maker[15] = 0;
	strncpy(newcomp.cpu, argv[5], 7);
	newcomp.cpu[7] = 0;
	newcomp.memory = atoi(argv[6]);
	strncpy(newcomp.desc, argv[7], Descsz - 1);
	newcomp.desc[Descsz - 1] = 0;

	sleep(5);
	fseek(fp, newcomp.id * sizeof(CComp), SEEK_SET);
	fwrite(&newcomp, sizeof(CComp), 1, fp);
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
}
