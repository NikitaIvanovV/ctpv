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

    int c, i = 0;
    for (; s[i] != 0; i++) {
        c = s[i];
        if (!isalnum(c))
            c = '_';

        res[i] = c;
    }

    res[i] = '\0';
}

void print_char(char c)
{
    printf("0x%x, ", c);
}

void print_file(char *f)
{
    int c;
    FILE *file = fopen(f, "r");

    if (!file) {
        fprintf(stderr, "failed to open %s: %s\n", f, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((c = fgetc(file)) != EOF)
        print_char(c);

    fclose(file);
}

void embed_file(char *prefix, char *filename, char *helpers)
{
    static char varname[FILENAME_MAX];

    getvarname(varname, prefix, filename);

    printf("char %s[] = { ", varname);

    if (helpers) {
        print_file(helpers);
        print_char('\n');
    }

    print_file(filename);

    puts("0 };");
}

int main(int argc, char *argv[])
{
    char *prefix = NULL, *helpers = NULL;

    int c;
    while ((c = getopt(argc, argv, "p:h:")) != -1) {
        switch (c) {
        case 'p':
            prefix = optarg;
            break;
        case 'h':
            helpers = optarg;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    for (int i = optind; i < argc; i++)
        embed_file(prefix, argv[i], helpers);

    return EXIT_SUCCESS;
}
