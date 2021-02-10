/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *      This file contains the external implementations of methods in
 *      the basis class for all the various transport endpoint classes
 *      in the Inet layer, i.e. TCP, UDP, Raw and Tun.
 */

#include <inet/EndPointBasis.h>

#include <inet/InetLayer.h>

namespace chip {
namespace Inet {

void EndPointBasis::InitEndPointBasis(InetLayer & aInetLayer, void * aAppState)
{
    InitInetLayerBasis(aInetLayer, aAppState);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    mLwIPEndPointType = kLwIPEndPointType_Unknown;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    mSocket = INET_INVALID_SOCKET_FD;
    mPendingIO.Clear();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
}

} // namespace Inet
} // namespace chip
