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
    CComp newcomp;

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
}