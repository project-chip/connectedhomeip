#include <net_if.h>
#include <sys/eventfd.h>

#include "EventFileHandle.h"
#include "OpenFileHandleAsFileDescriptor.h"

int eventfd(unsigned int initval, int flags)
{
    if (initval || flags)
    {
        return -1;
    }
    return mbed::open_fh_as_fd<mbed::EventFileHandle>();
}

int eventfd_read(int fd, eventfd_t * value)
{
    return read(fd, value, sizeof(*value));
}

int eventfd_write(int fd, eventfd_t value)
{
    return write(fd, &value, sizeof(value));
}