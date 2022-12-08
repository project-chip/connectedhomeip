#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char* cmdline_magic = "cmdline";

extern char __cmdline__[];

void openiotsdk_get_cmdline(int * argc, char *** argv)
{
    assert(argc != NULL);
    assert(argv != NULL);

    *argc = 0;

    size_t magic_length = strlen(cmdline_magic);
    char* p = &__cmdline__[0];
    if (strcmp(p, cmdline_magic) != 0) {
        return;
    }

    p += magic_length + 1;

    // Count NUL-terminated strings in __cmdline__ until double-NUL is encountered.
    int i     = 1;
    char prev = p[0];
    while (true)
    {
        char c = p[i];
        if (c == 0)
        {
            if (prev == 0)
            {
                break;
            }

            (*argc)++;
        }

        prev = c;
        i++;
    }

    // Allocate space for string pointers.
    *argv      = calloc(*argc, sizeof(char *));
    (*argv)[0] = p;

    // Assign each element in argc to the start of its string in p.
    for (int cmdline_idx = 0, argv_idx = 1; argv_idx < *argc; cmdline_idx++)
    {
        if (p[cmdline_idx] == 0)
        {
            cmdline_idx++;
            (*argv)[argv_idx++] = &p[cmdline_idx];
        }
    }
}
