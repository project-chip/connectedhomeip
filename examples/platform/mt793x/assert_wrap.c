

#include <stdio.h>
#include <assert.h>


extern void platform_assert(const char *expr,
                            const char *file,
                            int        line);


void __assert_func (const char * file,
                    int          line,
                    const char * func,
                    const char * expr)
{
    fflush(NULL);
    platform_assert(expr, file, line);
    while (1);
}
