#include <stdio.h>

extern int log_write(char * buf, int len);

int __wrap__write(int file, char * ptr, int len)
{
    return log_write(ptr, len);
}
