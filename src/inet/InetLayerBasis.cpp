/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *      This file contains the basis class for reference counting
 *      objects by the Inet layer as well as a class for representing
 *      the pending or resulting I/O events on a socket.
 */

#include "InetLayerBasis.h"

namespace chip {
namespace Inet {

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
/**
 *  Sets the bit for the specified file descriptor in the given sets of file descriptors.
 *
 *  @param[in]    socket    The file descriptor for which the bit is being set.
 *
 *  @param[out]   nfds      A reference to the range of file descriptors in the set.
 *
 *  @param[in]    readfds   A pointer to the set of readable file descriptors.
 *
 *  @param[in]    writefds  A pointer to the set of writable file descriptors.
 *
 *  @param[in]    exceptfds  A pointer to the set of file descriptors with errors.
 *
 */
void SocketEvents::SetFDs(int socket, int & nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds)
{
    if (socket != INET_INVALID_SOCKET_FD)
    {
        if (IsReadable())
            FD_SET(socket, readfds);
        if (IsWriteable())
            FD_SET(socket, writefds);
        if (IsError())
            FD_SET(socket, exceptfds);
        if (IsSet() && (socket + 1) > nfds)
            nfds = socket + 1;
    }
}

/**
 *  Set the read, write or exception bit flags for the specified socket based on its status in
 *  the corresponding file descriptor sets.
 *
 *  @param[in]    socket    The file descriptor for which the bit flags are being set.
 *
 *  @param[in]    readfds   A pointer to the set of readable file descriptors.
 *
 *  @param[in]    writefds  A pointer to the set of writable file descriptors.
 *
 *  @param[in]    exceptfds  A pointer to the set of file descriptors with errors.
 *
 */
SocketEvents SocketEvents::FromFDs(int socket, fd_set * readfds, fd_set * writefds, fd_set * exceptfds)
{
    SocketEvents res;

    if (socket != INET_INVALID_SOCKET_FD)
    {
        if (FD_ISSET(socket, readfds))
            res.SetRead();
        if (FD_ISSET(socket, writefds))
            res.SetWrite();
        if (FD_ISSET(socket, exceptfds))
            res.SetError();
    }

    return res;
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace chip
