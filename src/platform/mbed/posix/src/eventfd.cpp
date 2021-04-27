#include <net_socket.h>
#include <sys/eventfd.h>

int eventfd(unsigned int initval, int flags)
{
    return mbed_eventfd(initval, flags);
}

int eventfd_read(int fd, eventfd_t * value)
{
    return mbed_eventfd_read(fd, value);
}

int eventfd_write(int fd, eventfd_t value)
{
    return mbed_eventfd_write(fd, value);
}
