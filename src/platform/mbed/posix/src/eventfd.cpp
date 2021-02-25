#include <net_if.h>
#include <sys/eventfd.h>

int eventfd(unsigned int initval, int flags)
{
    return 0;
}

int eventfd_read(int fd, eventfd_t * value)
{
    return 0;
}

int eventfd_write(int fd, eventfd_t value)
{
    return 0;
}