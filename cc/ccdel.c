#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include "cc.h"
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    int fd;
    struct stat file_info;
    CComp findcomp;

    if (argc != 1)
    {
        fprintf("Usage: %s id\n", argv[0]);
        exit(1);
    }

    fp = fopen("ccdb", "r+");
    if (fp == NULL)
    {
        perror("ccdb");
        exit(2);
    }

    fd = fileno(fp);

    if (flock(fd, LOCK_EX) != 0) {
        perror("flock");
        close(fp);
        exit(3);
    }

    findcomp.id = argv[1];

    sleep(1);
    fseek(fp, findcomp.id * sizeof(CComp), SEEK_SET);
    findcomp.id = 0;
    fwrite(&findcomp, sizeof(CComp), 1, fp);


    flock(fileno(fp), LOCK_UN);
    fclose(fp);
}