#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

void getvarname(char *res, char *prefix, char *filename)
{
    char *s = strrchr(filename, '/');
    if (s)
        s++;
    else
        s = filename;

    if (prefix) {
        size_t prefix_len = strlen(prefix);
        memcpy(res, prefix, prefix_len);
        res += prefix_len;
    }

    for (int c, i = 0; s[i] != 0; i++) {
        c = s[i];
        if (!isalnum(c))
            c = '_';

        res[i] = c;
    }
}

void embed_file(char *prefix, char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "failed to open %s: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    static char varname[FILENAME_MAX];
    getvarname(varname, prefix, filename);

    printf("char %s[] = { ", varname);

    int c;
    while ((c = fgetc(f)) != EOF)
        printf("0x%x, ", c);

    puts("0 };");

    fclose(f);
}

int main(int argc, char *argv[])
{
    char *prefix = NULL;

    int c;
    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch (c) {
        case 'p':
            prefix = optarg;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    for (int i = optind; i < argc; i++)
        embed_file(prefix, argv[i]);

    return EXIT_SUCCESS;
}
