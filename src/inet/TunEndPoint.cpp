/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file implements the TunnelEndPoint abstraction APIs in the Inet
 *      Layer for creation and management of tunnel interfaces instantiated
 *      within either Linux Sockets or LwIP.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include "TunEndPoint.h"

#include <core/CHIPTunnelConfig.h>

#include <inet/InetLayer.h>

#include <core/CHIPEncoding.h>
#include <support/CodeUtils.h>

#include "arpa-inet-compatibility.h"
#include <stdio.h>
#include <string.h>

#if INET_WANT_ENDPOINT_TUN

namespace chip {
namespace Inet {

using chip::System::PacketBuffer;

chip::System::ObjectPool<TunEndPoint, INET_CONFIG_NUM_TUN_ENDPOINTS> TunEndPoint::sPool;

using namespace chip::Encoding;

/**
 * Initialize the Tunnel EndPoint object.
 *
 * @note
 *  By convention, the \c Init method on \c EndPointBasis
 *  subclasses is \c private. It should not be used outside \c InetLayer.
 *
 * @param[in] inetLayer       A pointer to the Inet layer object that
 *                            created the Tunnel EndPoint.
 *
 */
void TunEndPoint::Init(InetLayer * inetLayer)
{
    InitEndPointBasis(*inetLayer);
}

/**
 * Open a tunnel pseudo interface and create a handle to it.
 *
 * @note
 *  This method has different signatures on LwIP systems and
 *  POSIX systems.  On LwIP, there is an argument for specifying the name
 *  of the tunnel interface.  On POSIX, the method has no arguments and the
 *  name of the tunnel device is implied.
 *
 * @return INET_NO_ERROR on success, else a corresponding INET mapped OS error.
 */
#if CHIP_SYSTEM_CONFIG_USE_LWIP
INET_ERROR TunEndPoint::Open(void)
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    INET_ERROR TunEndPoint::Open(const char * intfName)
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
{
    INET_ERROR err = INET_NO_ERROR;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    struct netif * tNetif = NULL;
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    tNetif = netif_add(&mTunNetIf, NULL, NULL, NULL, this, TunInterfaceNetifInit, tcpip_input);

    // UnLock LwIP stack
    UNLOCK_TCPIP_CORE();

    VerifyOrExit(tNetif != NULL, err = INET_ERROR_INTERFACE_INIT_FAILURE);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    // Create the tunnel device
    err = TunDevOpen(intfName);
    SuccessOrExit(err);

    printf("Opened tunnel device: %s\n", intfName);

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (err == INET_NO_ERROR)
        mState = kState_Open;

exit:

    return err;
}

/**
 * Close the tunnel pseudo interface device.
 *
 */
void TunEndPoint::Close(void)
{
    if (mState != kState_Closed)
    {

        // For LwIP, we do not remove the netif as it would have
        // an impact on the interface iterator in CHIP which
        // might lose reference to a particular netif index that
        // it might be holding on to.
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        if (mSocket >= 0)
        {
            chip::System::Layer & lSystemLayer = SystemLayer();

            // Wake the thread calling select so that it recognizes the socket is closed.
            lSystemLayer.WakeSelect();
            TunDevClose();
        }

        // Clear any results from select() that indicate pending I/O for the socket.
        mPendingIO.Clear();

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
        mState = kState_Closed;
    }
}

/**
 * Close the tunnel pseudo interface device and decrement the reference count
 * of the InetLayer object.
 *
 */
void TunEndPoint::Free()
{
    Close();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    DeferredFree(kReleaseDeferralErrorTactic_Release);
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
    Release();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 * Send an IPv6 packet to the tun device to be sent out.
 *
 * @note
 *  This method performs a couple of minimal sanity checks on the packet to
 *  be sure it is IP version 6 then dispatches it for encapsulation in a
 *  CHIP tunneling message.
 *
 * @param[in]   message     the IPv6 packet to send.
 *
 * @retval  INET_NO_ERROR   success: packet encapsulated and queued to send
 * @retval  INET_ERROR_NOT_SUPPORTED    packet not IP version 6
 * @retval  INET_ERROR_BAD_ARGS         \c message is a \c NULL pointer
 *
 */
INET_ERROR TunEndPoint::Send(PacketBuffer * msg)
{
    INET_ERROR ret = INET_NO_ERROR;

    ret = CheckV6Sanity(msg);

    if (ret == INET_NO_ERROR)
    {
        ret = TunDevSendMessage(msg);
    }

    return ret;
}

/**
 * Extract the activation state of the tunnel interface.
 *
 * @returns \c true if the tunnel interface is active,
 *          otherwise \c false.
 */
bool TunEndPoint::IsInterfaceUp(void) const
{
    bool ret = false;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    ret = netif_is_up(&mTunNetIf);

    // UnLock LwIP stack
    UNLOCK_TCPIP_CORE();

    ExitNow();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    int sockfd = INET_INVALID_SOCKET_FD;
    struct ::ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    // Get interface
    if (TunGetInterface(mSocket, &ifr) < 0)
    {
        ExitNow();
    }

    sockfd = socket(AF_INET6, SOCK_DGRAM | NL_SOCK_CLOEXEC, IPPROTO_IP);
    if (sockfd < 0)
    {
        ExitNow();
    }

    // Get interface flags
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        ExitNow();
    }

    ret = ((ifr.ifr_flags & IFF_UP) == IFF_UP);

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

exit:
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (sockfd >= 0)
    {
        close(sockfd);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return ret;
}

/**
 * Activate the tunnel interface.
 *
 * @retval  INET_NO_ERROR           success: tunnel interface is activated.
 * @retval  other                   another system or platform error
 */
INET_ERROR TunEndPoint::InterfaceUp(void)
{
    INET_ERROR err = INET_NO_ERROR;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    netif_set_up(&mTunNetIf);

    // UnLock LwIP stack
    UNLOCK_TCPIP_CORE();

    ExitNow();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    int sockfd = INET_INVALID_SOCKET_FD;
    struct ::ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    // Get interface
    if (TunGetInterface(mSocket, &ifr) < 0)
    {
        ExitNow(err = chip::System::MapErrorPOSIX(errno));
    }

    sockfd = socket(AF_INET6, SOCK_DGRAM | NL_SOCK_CLOEXEC, IPPROTO_IP);
    if (sockfd < 0)
    {
        ExitNow(err = chip::System::MapErrorPOSIX(errno));
    }

    // Get interface flags
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        ExitNow(err = chip::System::MapErrorPOSIX(errno));
    }

    // Set flag to activate interface
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        err = chip::System::MapErrorPOSIX(errno);
    }

    // Set the MTU
    ifr.ifr_mtu = CHIP_CONFIG_TUNNEL_INTERFACE_MTU;
    if (ioctl(sockfd, SIOCSIFMTU, &ifr) < 0)
    {
        ExitNow(err = chip::System::MapErrorPOSIX(errno));
    }

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

exit:
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (sockfd >= 0)
    {
        close(sockfd);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return err;
}

/**
 * @brief   Deactivate the tunnel interface.
 *
 * @retval  INET_NO_ERROR           success: tunnel interface is deactivated.
 * @retval  other                   another system or platform error
 */
INET_ERROR TunEndPoint::InterfaceDown(void)
{
    INET_ERROR err = INET_NO_ERROR;
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    int sockfd = INET_INVALID_SOCKET_FD;
    struct ::ifreq ifr;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Remove the link local address from the netif
    memset(&(mTunNetIf.ip6_addr[0]), 0, sizeof(ip6_addr_t));

    netif_set_down(&mTunNetIf);

    // UnLock LwIP stack
    UNLOCK_TCPIP_CORE();

    ExitNow();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    memset(&ifr, 0, sizeof(ifr));

    // Get interface
    if (TunGetInterface(mSocket, &ifr) < 0)
    {
        ExitNow(err = chip::System::MapErrorPOSIX(errno));
    }

    sockfd = socket(AF_INET6, SOCK_DGRAM | NL_SOCK_CLOEXEC, IPPROTO_IP);
    if (sockfd < 0)
    {
        ExitNow(err = chip::System::MapErrorPOSIX(errno));
    }

    // Get interface flags
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        ExitNow(err = chip::System::MapErrorPOSIX(errno));
    }

    // Set flag to deactivate interface
    ifr.ifr_flags &= ~(IFF_UP);
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        err = chip::System::MapErrorPOSIX(errno);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

exit:
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    if (sockfd >= 0)
    {
        close(sockfd);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return err;
}

/**
 * @brief   Get the tunnel interface identifier.
 *
 * @return  The tunnel interface identifier.
 */
InterfaceId TunEndPoint::GetTunnelInterfaceId(void)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    return &mTunNetIf;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    INET_ERROR err          = INET_NO_ERROR;
    InterfaceId tunIntfId   = INET_NULL_INTERFACEID;
    const char * tunIntfPtr = &tunIntfName[0];

    err = InterfaceNameToId(tunIntfPtr, tunIntfId);
    if (err != INET_NO_ERROR)
    {
        tunIntfId = INET_NULL_INTERFACEID;
    }

    return tunIntfId;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP
/* Function for sending the IPv6 packets over LwIP */
INET_ERROR TunEndPoint::TunDevSendMessage(PacketBuffer * msg)
{
    INET_ERROR ret  = INET_NO_ERROR;
    struct pbuf * p = NULL;
    err_t err       = ERR_OK;

    // no packet could be read, silently ignore this
    VerifyOrExit(msg != NULL, ret = INET_ERROR_BAD_ARGS);

    p = (struct pbuf *) msg;

    // Call the input function for the netif object in LWIP.
    // This essentially creates a TCP_IP msg and puts into
    // the mbox message queue for processing by the TCP/IP
    // stack.

    if ((err = tcpip_input(p, &mTunNetIf)) != ERR_OK)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("tunNetif_input: IP input error\n"));
        ExitNow(ret = chip::System::MapErrorLwIP(err));
    }

exit:
    return (ret);
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
/* Function for sending the IPv6 packets over Linux sockets */
INET_ERROR TunEndPoint::TunDevSendMessage(PacketBuffer * msg)
{
    INET_ERROR ret  = INET_NO_ERROR;
    ssize_t lenSent = 0;
    uint8_t * p     = NULL;

    // no packet could be read, silently ignore this
    VerifyOrExit(msg != NULL, ret = INET_ERROR_BAD_ARGS);

    p = msg->Start();

    lenSent = write(mSocket, p, msg->DataLength());
    if (lenSent < 0)
    {
        ExitNow(ret = chip::System::MapErrorPOSIX(errno));
    }
    else if (lenSent < msg->DataLength())
    {
        ExitNow(ret = INET_ERROR_OUTBOUND_MESSAGE_TRUNCATED);
    }

exit:
    if (msg != NULL)
    {
        PacketBuffer::Free(msg);
    }

    return (ret);
}
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

/* Function that performs some basic sanity checks for IPv6 packets */
INET_ERROR TunEndPoint::CheckV6Sanity(PacketBuffer * msg)
{
    INET_ERROR err          = INET_NO_ERROR;
    uint8_t * p             = NULL;
    struct ip6_hdr * ip6hdr = NULL;

    p = msg->Start();

    ip6hdr = (struct ip6_hdr *) p;

    VerifyOrExit(ip6hdr != NULL, err = INET_ERROR_BAD_ARGS);

    // Do some IPv6 sanity checks
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    if (IP6H_V(ip6hdr) != 6)
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        if ((ip6hdr->ip6_vfc >> 4) != 6)
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
        {
            ExitNow(err = INET_ERROR_NOT_SUPPORTED);
        }

exit:

    return err;
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP
/* Handler to send received packet to upper layer callback */
void TunEndPoint::HandleDataReceived(PacketBuffer * msg)
{
    INET_ERROR err = INET_NO_ERROR;
    if (mState == kState_Open && OnPacketReceived != NULL)
    {
        err = CheckV6Sanity(msg);
        if (err == INET_NO_ERROR)
        {
            OnPacketReceived(this, msg);
        }
        else
        {
            if (OnReceiveError != NULL)
            {
                OnReceiveError(this, err);
            }

            PacketBuffer::Free(msg);
        }
    }
    else
    {
        PacketBuffer::Free(msg);
    }
}

/* Post an event to the Inet layer event queue from LwIP */
err_t TunEndPoint::LwIPPostToInetEventQ(struct netif * netif, struct pbuf * p)
{
    err_t lwipErr                      = ERR_OK;
    INET_ERROR err                     = INET_NO_ERROR;
    TunEndPoint * ep                   = static_cast<TunEndPoint *>(netif->state);
    chip::System::Layer & lSystemLayer = ep->SystemLayer();
    PacketBuffer * buf                 = PacketBuffer::NewWithAvailableSize(p->tot_len);

    // Starting off with a reserved size of the default CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE
    // which allows for adding the CHIP header and the underlying transport and IP headers
    // encapsulating this tunneled packet.

    VerifyOrExit(buf != NULL, lwipErr = ERR_MEM);

    buf->SetDataLength(p->tot_len);

    // Make a pbuf alloc and copy to post to Inetlayer queue because LwIP would free the
    // passed pbuf as it made a down-call to send it out the tunnel netif.

    lwipErr = pbuf_copy((struct pbuf *) buf, p);
    VerifyOrExit(lwipErr == ERR_OK, (void) lwipErr);

    err = lSystemLayer.PostEvent(*ep, kInetEvent_TunDataReceived, (uintptr_t) buf);
    VerifyOrExit(err == INET_NO_ERROR, lwipErr = ERR_MEM);

    buf = NULL;

exit:
    if (buf != NULL)
    {
        PacketBuffer::Free(buf);
    }

    return lwipErr;
}

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#if LWIP_IPV4
/* Output handler for netif */
err_t TunEndPoint::LwIPOutputIPv4(struct netif * netif, struct pbuf * p, const ip4_addr_t * addr)
{
    LWIP_UNUSED_ARG(addr);

    return LwIPPostToInetEventQ(netif, p);
}
#endif // LWIP_IPV4

#if LWIP_IPV6
/* Output handler for netif */
err_t TunEndPoint::LwIPOutputIPv6(struct netif * netif, struct pbuf * p, const ip6_addr_t * addr)
{
    LWIP_UNUSED_ARG(addr);

    return LwIPPostToInetEventQ(netif, p);
}
#endif // LWIP_IPV4
#else  // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5
/* Receive message in LwIP */
err_t TunEndPoint::LwIPReceiveTunMessage(struct netif * netif, struct pbuf * p, ip4_addr_t * addr)
{
    LWIP_UNUSED_ARG(addr);

    return LwIPPostToInetEventQ(netif, p);
}

#if LWIP_IPV6
err_t TunEndPoint::LwIPReceiveTunV6Message(struct netif * netif, struct pbuf * p, ip6_addr_t * addr)
{
    LWIP_UNUSED_ARG(addr);

    return LwIPPostToInetEventQ(netif, p);
}
#endif // LWIP_IPV6
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5

/* Initialize the LwIP tunnel netif interface */
err_t TunEndPoint::TunInterfaceNetifInit(struct netif * netif)
{
    netif->name[0] = 't';
    netif->name[1] = 'n';
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#if LWIP_IPV4
    netif->output = LwIPOutputIPv4;
#endif /* LWIP_IPV6 */
#if LWIP_IPV6
    netif->output_ip6 = LwIPOutputIPv6;
#endif /* LWIP_IPV6 */
#else  // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5
    netif->output     = LwIPReceiveTunMessage;
#if LWIP_IPV6
    netif->output_ip6 = LwIPReceiveTunV6Message;
#endif /* LWIP_IPV6 */
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5
    netif->linkoutput = NULL;

    netif->mtu = CHIP_CONFIG_TUNNEL_INTERFACE_MTU;

    netif->hwaddr_len = 6;
    memset(netif->hwaddr, 0, NETIF_MAX_HWADDR_LEN);
    netif->hwaddr[5] = 1;

#if LWIP_VERSION_MAJOR == 1 && LWIP_VERSION_MINOR < 5
    /* device capabilities */
    netif->flags |= NETIF_FLAG_POINTTOPOINT;
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    return ERR_OK;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
/* Open a tun device in linux */
INET_ERROR TunEndPoint::TunDevOpen(const char * intfName)
{
    struct ::ifreq ifr;
    int fd         = INET_INVALID_SOCKET_FD;
    INET_ERROR ret = INET_NO_ERROR;

    if ((fd = open(INET_CONFIG_TUNNEL_DEVICE_NAME, O_RDWR | NL_O_CLOEXEC)) < 0)
    {
        ExitNow(ret = chip::System::MapErrorPOSIX(errno));
    }

    // Keep copy of open device fd
    mSocket = fd;

    memset(&ifr, 0, sizeof(ifr));

#if HAVE_LINUX_IF_TUN_H
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
#endif

    if (*intfName)
    {
        strncpy(ifr.ifr_name, intfName, sizeof(ifr.ifr_name) - 1);
        ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';
    }

#if HAVE_LINUX_IF_TUN_H
    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0)
    {
        ExitNow(ret = chip::System::MapErrorPOSIX(errno));
    }
#endif

    // Verify name
    memset(&ifr, 0, sizeof(ifr));
    if (TunGetInterface(fd, &ifr) < 0)
    {
        ExitNow(ret = chip::System::MapErrorPOSIX(errno));
    }

    if (ifr.ifr_name[0] != '\0')
    {
        // Keep member copy of interface name and Id
        strncpy(tunIntfName, ifr.ifr_name, sizeof(tunIntfName) - 1);
        tunIntfName[sizeof(tunIntfName) - 1] = '\0';
    }
    else
    {
        ExitNow(ret = chip::System::MapErrorPOSIX(errno));
    }

exit:

    if (ret != INET_NO_ERROR)
    {
        TunDevClose();
    }

    return ret;
}

/* Close a tun device */
void TunEndPoint::TunDevClose(void)
{
    if (mSocket >= 0)
    {
        close(mSocket);
    }
    mSocket = INET_INVALID_SOCKET_FD;
}

/* Get the tun device interface in Linux */
int TunEndPoint::TunGetInterface(int fd, struct ::ifreq * ifr)
{
#if HAVE_LINUX_IF_TUN_H
    return ioctl(fd, TUNGETIFF, (void *) ifr);
#else
    return -1;
#endif
}

/* Read packets from TUN device in Linux */
INET_ERROR TunEndPoint::TunDevRead(PacketBuffer * msg)
{
    ssize_t rcvLen;
    INET_ERROR err = INET_NO_ERROR;
    uint8_t * p    = NULL;
    p              = msg->Start();

    rcvLen = read(mSocket, p, msg->AvailableDataLength());
    if (rcvLen < 0)
    {
        err = chip::System::MapErrorPOSIX(errno);
    }
    else if (rcvLen > msg->AvailableDataLength())
    {
        err = INET_ERROR_INBOUND_MESSAGE_TOO_BIG;
    }
    else
    {
        msg->SetDataLength((uint16_t) rcvLen);
    }

    return err;
}

/* Prepare socket for reading */
SocketEvents TunEndPoint::PrepareIO()
{
    SocketEvents res;

    if (mState == kState_Open && OnPacketReceived != NULL)
    {
        res.SetRead();
    }

    return res;
}

/* Read from the Tun device in Linux and pass up to upper layer callback */
void TunEndPoint::HandlePendingIO()
{
    INET_ERROR err = INET_NO_ERROR;

    if (mState == kState_Open && OnPacketReceived != NULL && mPendingIO.IsReadable())
    {

        PacketBuffer * buf = PacketBuffer::New(0);

        if (buf != NULL)
        {
            // Read data from Tun Device
            err = TunDevRead(buf);
            if (err == INET_NO_ERROR)
            {
                err = CheckV6Sanity(buf);
            }
        }
        else
        {
            err = INET_ERROR_NO_MEMORY;
        }

        if (err == INET_NO_ERROR)
        {
            OnPacketReceived(this, buf);
        }
        else
        {
            PacketBuffer::Free(buf);
            if (OnReceiveError != NULL)
            {
                OnReceiveError(this, err);
            }
        }
    }

    mPendingIO.Clear();
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace chip

#endif // INET_WANT_ENDPOINT_TUN
