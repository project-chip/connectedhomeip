#include <net_if.h>
#include <sys/ioctl.h>

int ioctl(int fd, unsigned long request, void * param)
{
    return mbed_ioctl(fd, request, param);
}
