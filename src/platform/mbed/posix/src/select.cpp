#include <net_socket.h>
#include <sys/select.h>

int select(int fd, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout)
{
    return mbed_select(fd, readfds, writefds, exceptfds, timeout);
}