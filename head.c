#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    int c;
    int lines = 10;
    int printed = 0;
    int args = 1;

    if (argc > args && argv[args][0] == '-') {
        char *num_str = &argv[args][1];

        if (num_str[0] == '\0' || !isdigit((unsigned char)num_str[0])) {
            fprintf(stderr, "Usage: %s [-n] [file]", argv[0]);
            return EXIT_FAILURE;
        }

        lines = atoi(num_str);
        args++;
    } 

    if (argc > args){
        fp = fopen(argv[args], "r");
        if (fp == NULL) {
            perror("Could not open file");
            return EXIT_FAILURE;
        }
        args++;
    } else{
        fp = stdin;
    }

    if (argc > args) {
        fprintf(stderr, "Usage: %s [-n] [file]\n", argv[0]);
        if (fp != stdin) fclose(fp);
        return EXIT_FAILURE;
    }

    while (printed < lines && (c = getc(fp)) != EOF) {
        putc(c, stdout);
        if (c == '\n') {
            printed++;
        }
    }

    if (fp != stdin) {
        fclose(fp);
    }

    return EXIT_SUCCESS;
}