/*
 *    Copyright (c) 2022 Project CHIP Authors
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
 * This file implements Inet::TCPEndPoint using sockets.
 */

#include <inet/TCPEndPointImplIoTSocket.h>

#include <inet/InetFaultInjection.h>
#include <inet/arpa-inet-compatibility.h>

#include <iot_socket.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#include <stdio.h>
#include <string.h>
#include <utility>

namespace chip {
namespace System {
CHIP_ERROR MapErrorIoTSocket(int32_t errorCode);
}
namespace Inet {

using System::MapErrorIoTSocket;

CHIP_ERROR TCPEndPointImplIoTSocket::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr)
{
    CHIP_ERROR res = GetSocket(addrType);

    if (res == CHIP_NO_ERROR && reuseAddr)
    {
        // Enable SO_REUSEPORT.  This permits coexistence between an
        // untargetted CHIP client and other services that listen on
        // a CHIP port on a specific address (such as a CHIP client
        // with TARGETED_LISTEN or TCP proxying services).  Note that
        // one of the costs of this implementation is the
        // non-deterministic connection dispatch when multple clients
        // listen on the address with the same degreee of selectivity,
        // e.g. two untargetted-listen CHIP clients, or two
        // targeted-listen CHIP clients with the same node id.

        int32_t n       = 1;
        int32_t retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_REUSEADDR, &n, sizeof(n));
        if (retcode)
        {
            ChipLogError(Inet, "SO_REUSEPORT: %ld", retcode);
        }
    }

    if (res == CHIP_NO_ERROR)
    {
        if (addrType == IPAddressType::kIPv6)
        {
            struct iot_in6_addr iot_address;
            iot_address     = addr.ToIPv6();
            int32_t retcode = iotSocketBind(mSocket, (uint8_t *) &iot_address, sizeof(iot_address), port);
            if (retcode)
            {
                res = MapErrorIoTSocket(retcode);
            }
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == IPAddressType::kIPv4)
        {
            struct iot_in_addr iot_address;
            iot_address     = addr.ToIPv4();
            int32_t retcode = iotSocketBind(mSocket, (uint8_t *) &iot_address, sizeof(iot_address), port);
            if (retcode)
            {
                res = MapErrorIoTSocket(retcode);
            }
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
        {
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
        }
    }

    return res;
}

CHIP_ERROR TCPEndPointImplIoTSocket::ListenImpl(uint16_t backlog)
{
    int32_t retcode = iotSocketListen(mSocket, backlog);
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    // Enable non-blocking mode for the socket.
    uint32_t opt = 1;
    iotSocketSetOpt(mSocket, IOT_SOCKET_IO_FIONBIO, &opt, sizeof(uint32_t));

    // Wait for ability to read on this endpoint.
    CHIP_ERROR res = static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).EnableSelectCallback(this, true, false);

    return res;
}

CHIP_ERROR TCPEndPointImplIoTSocket::ConnectImpl(const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    IPAddressType addrType = addr.Type();

    ReturnErrorOnFailure(GetSocket(addrType));

    if (!intfId.IsPresent())
    {
        // The behavior when connecting to an IPv6 link-local address without specifying an outbound
        // interface is ambiguous. So prevent it in all cases.
        if (addr.IsIPv6LinkLocal())
        {
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }
    }
    else
    {
        // Try binding to the interface

        // If destination is link-local then there is no need to iotSocketBind to
        // interface or address on the interface.

        if (!addr.IsIPv6LinkLocal())
        {
            char interfaceName[NETIF_NAMESIZE];
            ReturnErrorOnFailure(intfId.GetInterfaceName(interfaceName, NETIF_NAMESIZE));

            // Attempt to iotSocketBind to the interface using SO_BINDTODEVICE which requires privileged access.
            // If the permission is denied(EACCES) because CHIP is running in a context
            // that does not have privileged access, choose a source address on the
            // interface to iotSocketBind the connetion to.
            int32_t retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_BINDTODEVICE, &interfaceName, strlen(interfaceName));

            if (retcode == 0)
            {
                mBoundInterface = intfId;
            }
            else
            {
                // Attempting to initiate a connection via a specific interface is not allowed.
                // The only way to do this is to iotSocketBind the local to an address on the desired
                // interface.
                ReturnErrorOnFailure(BindSrcAddrFromIntf(addrType, intfId));
            }
        }
    }

    // Enable non-blocking mode for the socket.
    uint32_t opt = 1;
    iotSocketSetOpt(mSocket, IOT_SOCKET_IO_FIONBIO, &opt, sizeof(uint32_t));

    int conRes;

    if (addrType == IPAddressType::kIPv6)
    {
        iot_in6_addr iot_address;
        iot_address = addr.ToIPv6();

        conRes = iotSocketConnect(mSocket, (uint8_t *) &iot_address, sizeof(iot_address), port);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (addrType == IPAddressType::kIPv4)
    {
        iot_in_addr iot_address;
        iot_address = addr.ToIPv4();

        conRes = iotSocketConnect(mSocket, (uint8_t *) &iot_address, sizeof(iot_address), port);
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    if (conRes < 0 && conRes != IOT_SOCKET_EINPROGRESS)
    {
        CHIP_ERROR res = MapErrorIoTSocket(conRes);
        DoClose(res, true);
        return res;
    }

    // Once Connecting or Connected, bump the reference count.  The corresponding Release() will happen in DoClose().
    Retain();

    if (conRes == 0)
    {
        mState = State::kConnected;
        // Wait for ability to read on this endpoint.
        ReturnErrorOnFailure(static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).EnableSelectCallback(this, true, false));
        if (OnConnectComplete != nullptr)
        {
            OnConnectComplete(this, CHIP_NO_ERROR);
        }
    }
    else
    {
        mState = State::kConnecting;
        // Wait for ability to write on this endpoint.
        ReturnErrorOnFailure(static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).EnableSelectCallback(this, false, true));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplIoTSocket::GetInfo(IPAddress * retAddr, uint16_t * retPort, bool local) const
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    union
    {
        iot_in6_addr addr6;
        iot_in_addr addr4;
    } addr;
    uint32_t iplen = sizeof(addr);

    int32_t ret;

    if (local)
    {
        ret = iotSocketGetSockName(mSocket, (uint8_t *) &addr, &iplen, retPort);
    }
    else
    {

        ret = iotSocketGetPeerName(mSocket, (uint8_t *) &addr, &iplen, retPort);
    }

    if (iplen == sizeof(iot_in6_addr))
    {
        *retAddr = IPAddress(addr.addr6);
    }
    else
    {
        *retAddr = IPAddress(addr.addr4);
    }

    if (ret == 0)
    {
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR TCPEndPointImplIoTSocket::GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    return GetInfo(retAddr, retPort, false);
}

CHIP_ERROR TCPEndPointImplIoTSocket::GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    return GetInfo(retAddr, retPort, true);
}

CHIP_ERROR TCPEndPointImplIoTSocket::GetInterfaceId(InterfaceId * retInterface)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    if (retInterface)
    {
        *retInterface = mBoundInterface;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR TCPEndPointImplIoTSocket::SendQueuedImpl(bool queueWasEmpty)
{
    if (queueWasEmpty)
    {
        // Wait for ability to write on this endpoint.
        return static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).EnableSelectCallback(this, false, true);
        ;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplIoTSocket::EnableNoDelay()
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // Disable TCP Nagle buffering by setting TCP_NODELAY socket option to true
    int32_t val     = 1;
    int32_t retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_TCP_NODELAY, &val, sizeof(val));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplIoTSocket::EnableKeepAlive(uint16_t interval, uint16_t timeoutCount)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // Set the idle interval
    int32_t val     = interval;
    int32_t retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_TCP_KEEPIDLE, &val, sizeof(val));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    // Set the probe retransmission interval.
    val     = interval;
    retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_TCP_KEEPINTVL, &val, sizeof(val));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    // Set the probe timeout count
    val     = timeoutCount;
    retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_TCP_KEEPCNT, &val, sizeof(val));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    // Enable keepalives for the connection.
    val     = 1; // enable
    retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_KEEPALIVE, &val, sizeof(val));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplIoTSocket::DisableKeepAlive()
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // Disable keepalives on the connection.
    int32_t val     = 0; // disable
    int32_t retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_KEEPALIVE, &val, sizeof(val));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplIoTSocket::AckReceive(uint16_t len)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // nothing to do for sockets case
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplIoTSocket::SetUserTimeoutImpl(uint32_t userTimeoutMillis)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR TCPEndPointImplIoTSocket::DriveSendingImpl()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Pretend send() fails in the while loop below
    INET_FAULT_INJECT(FaultInjection::kFault_Send, {
        err = MapErrorIoTSocket(EIO);
        DoClose(err, false);
        return err;
    });

    while (!mSendQueue.IsNull())
    {
        uint16_t bufLen = mSendQueue->DataLength();

        int32_t lenSentRaw = iotSocketSend(mSocket, mSendQueue->Start(), bufLen);

        if (lenSentRaw < 0)
        {
            if (lenSentRaw != IOT_SOCKET_EAGAIN)
            {
                err = MapErrorIoTSocket(lenSentRaw);
            }
            break;
        }

        if (lenSentRaw < 0 || lenSentRaw > bufLen)
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            break;
        }

        // Cast is safe because bufLen is uint16_t.
        uint16_t lenSent = static_cast<uint16_t>(lenSentRaw);

        // Mark the connection as being active.
        MarkActive();

        if (lenSent < bufLen)
        {
            mSendQueue->ConsumeHead(lenSent);
        }
        else
        {
            mSendQueue.FreeHead();
            if (mSendQueue.IsNull())
            {
                // Do not wait for ability to write on this endpoint.
                err = static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).DisableSelectCallback(this, false, true);
                if (err != CHIP_NO_ERROR)
                {
                    break;
                }
            }
        }

        if (OnDataSent != nullptr)
        {
            OnDataSent(this, lenSent);
        }

        if (lenSent < bufLen)
        {
            break;
        }
    }

    if (err == CHIP_NO_ERROR)
    {
        // If we're in the SendShutdown state and the send queue is now empty, shutdown writing on the socket.
        if (mState == State::kSendShutdown && mSendQueue.IsNull())
        {
            int32_t retcode = iotSocketShutdown(mSocket, IOT_SOCKET_SHUTDOWN_WR);
            if (retcode)
            {
                err = MapErrorIoTSocket(retcode);
            }
        }
    }

    return err;
}

void TCPEndPointImplIoTSocket::HandleConnectCompleteImpl()
{
    // Wait for ability to read or write on this endpoint.
    CHIP_ERROR err = static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).EnableSelectCallback(this, true, false);
    if (err == CHIP_NO_ERROR)
    {
        err = static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).EnableSelectCallback(this, false, true);
    }
    if (err != CHIP_NO_ERROR)
    {
        DoClose(err, false);
        return;
    }
}

void TCPEndPointImplIoTSocket::DoCloseImpl(CHIP_ERROR err, State oldState)
{
    // If the socket hasn't been closed already...
    if (mSocket != kInvalidSocketFd)
    {
        // If entering the Closed state
        // OR if entering the Closing state, and there's no unsent data in the send queue
        // THEN close the socket.
        if (mState == State::kClosed || (mState == State::kClosing && mSendQueue.IsNull()))
        {
            // If aborting the connection, ensure we send a TCP RST.
            if (IsConnected(oldState) && err != CHIP_NO_ERROR)
            {
                iot_opt_linger lingerStruct;
                lingerStruct.l_onoff  = 1;
                lingerStruct.l_linger = 0;

                int32_t retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_LINGER, &lingerStruct, sizeof(lingerStruct));
                if (retcode)
                {
                    ChipLogError(Inet, "SO_LINGER: %ld", retcode);
                }
            }

            static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).DisableSelectCallback(this, true, true);
            iotSocketClose(mSocket);
            mSocket = kInvalidSocketFd;
        }
    }
}

CHIP_ERROR TCPEndPointImplIoTSocket::BindSrcAddrFromIntf(IPAddressType addrType, InterfaceId intfId)
{
    // If we are trying to make a TCP connection over a 'specified target interface',
    // then we bind the TCPEndPoint to an IP address on that target interface
    // and use that address as the source address for that connection. This is
    // done in the event that directly binding the connection to the target
    // interface is not allowed due to insufficient privileges.
    VerifyOrReturnError(mState != State::kBound, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    bool ipAddrFound = false;
    for (InterfaceAddressIterator addrIter; addrIter.HasCurrent(); addrIter.Next())
    {
        IPAddress curAddr;
        if ((addrIter.GetInterfaceId() == intfId) && (addrIter.GetAddress(curAddr) == CHIP_NO_ERROR))
        {
            // Search for an IPv4 address on the TargetInterface

#if INET_CONFIG_ENABLE_IPV4
            if (addrType == IPAddressType::kIPv4)
            {
                if (curAddr.IsIPv4())
                {
                    // Bind to the IPv4 address of the TargetInterface
                    ipAddrFound = true;
                    ReturnErrorOnFailure(Bind(IPAddressType::kIPv4, curAddr, 0, true));

                    break;
                }
            }
#endif // INET_CONFIG_ENABLE_IPV4
            if (addrType == IPAddressType::kIPv6)
            {
                // Select an IPv6 address on the interface that is not
                // a link local or a multicast address.
                // TODO: Define a proper IPv6GlobalUnicast address checker.
                if (!curAddr.IsIPv4() && !curAddr.IsIPv6LinkLocal() && !curAddr.IsMulticast())
                {
                    // Bind to the IPv6 address of the TargetInterface
                    ipAddrFound = true;
                    ReturnErrorOnFailure(Bind(IPAddressType::kIPv6, curAddr, 0, true));

                    break;
                }
            }
        }
    }

    VerifyOrReturnError(ipAddrFound, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    mBoundInterface = intfId;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplIoTSocket::GetSocket(IPAddressType addrType)
{
    if (mSocket == kInvalidSocketFd)
    {
        if (addrType == IPAddressType::kIPv6)
        {
            mSocketFamily = IOT_SOCKET_AF_INET6;
#if INET_CONFIG_ENABLE_IPV4
        }
        else if (addrType == IPAddressType::kIPv4)
        {
            mSocketFamily = IOT_SOCKET_AF_INET;
#endif // INET_CONFIG_ENABLE_IPV4
        }
        else
        {
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }
        mSocket = iotSocketCreate(mSocketFamily, IOT_SOCKET_SOCK_STREAM, IOT_SOCKET_IPPROTO_TCP);
        if (mSocket < 0)
        {
            return MapErrorIoTSocket(mSocket);
        }

        // If creating an IPv6 socket, tell the kernel that it will be IPv6 only.  This makes it
        // posible to iotSocketBind two sockets to the same port, one for IPv4 and one for IPv6.
        if (mSocketFamily == IOT_SOCKET_AF_INET6)
        {
            int one = 1;
            iotSocketSetOpt(mSocket, IOT_SOCKET_IPV6_V6ONLY, &one, sizeof(one));
        }
    }
    else
    {
        if (addrType == IPAddressType::kIPv6 && mSocketFamily != IOT_SOCKET_AF_INET6)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        if (addrType == IPAddressType::kIPv4 && mSocketFamily != IOT_SOCKET_AF_INET)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    return CHIP_NO_ERROR;
}

void TCPEndPointImplIoTSocket::SelectCallback(void * readMask, void * writeMask, void * exceptionMask)
{
    bool readEvent  = iotSocketMaskIsSet(mSocket, readMask);
    bool writeEvent = iotSocketMaskIsSet(mSocket, writeMask);

    if (!readEvent && !writeEvent)
    {
        return;
    }

    // Prevent the end point from being freed while in the middle of a callback.
    Retain();

    // If in the Listening state, and the app is ready to receive a connection, and there is a connection
    // ready to be received on the socket, process the incoming connection.
    if (mState == State::kListening)
    {
        if (OnConnectionReceived != nullptr && readEvent)
        {
            HandleIncomingConnection();
        }
    }

    // If in the processes of initiating a connection...
    else if (mState == State::kConnecting)
    {
        // The socket being writable indicates the connection has completed (successfully or otherwise).
        if (writeEvent)
        {
            // connect blocks and never returns EINPROGRESS
            // The socket option SO_ERROR is not available.

            // Process the connection result.
            HandleConnectComplete(CHIP_NO_ERROR);
        }
    }

    else
    {
        // If in a state where sending is allowed, and there is data to be sent, and the socket is ready for
        // writing, drive outbound data into the connection.
        if (writeEvent && IsConnected() && !mSendQueue.IsNull())
        {
            DriveSending();
        }

        // If in a state were receiving is allowed, and the app is ready to receive data, and data is ready
        // on the socket, receive inbound data from the connection.
        if (readEvent)
        {
            if (mState == State::kConnected || mState == State::kSendShutdown)
            {
                if (mReceiveEnabled && OnDataReceived != nullptr)
                {
                    ReceiveData();
                }
            }
        }
    }

    Release();
}

void TCPEndPointImplIoTSocket::ReceiveData()
{
    System::PacketBufferHandle rcvBuf;
    bool isNewBuf = true;

    if (mRcvQueue.IsNull())
    {
        rcvBuf = System::PacketBufferHandle::New(kMaxReceiveMessageSize, 0);
    }
    else
    {
        rcvBuf = mRcvQueue->Last();
        if (rcvBuf->AvailableDataLength() == 0)
        {
            rcvBuf = System::PacketBufferHandle::New(kMaxReceiveMessageSize, 0);
        }
        else
        {
            isNewBuf = false;
            rcvBuf->CompactHead();
        }
    }

    if (rcvBuf.IsNull())
    {
        DoClose(CHIP_ERROR_NO_MEMORY, false);
        return;
    }

    // Attempt to receive data from the socket.
    int32_t rcvLen = iotSocketRecv(mSocket, rcvBuf->Start() + rcvBuf->DataLength(), rcvBuf->AvailableDataLength());

    // If an error occurred, abort the connection.
    if (rcvLen < 0)
    {
        CHIP_ERROR err = MapErrorIoTSocket(rcvLen);
        if (err == CHIP_ERROR_BUSY)
        {
            // Note: in this case, we opt to not retry the recv call,
            // and instead we expect that the read flags will get
            // reset correctly upon a subsequent return from the
            // select call.
            ChipLogError(Inet, "recv: EAGAIN, will retry");

            return;
        }

        DoClose(err, false);
    }

    else
    {
        // Mark the connection as being active.
        MarkActive();

        // If the peer closed their end of the connection...
        if (rcvLen == 0)
        {
            // If in the Connected state and the app has provided an OnPeerClose callback,
            // enter the ReceiveShutdown state.  Providing an OnPeerClose callback allows
            // the app to decide whether to keep the send side of the connection open after
            // the peer has closed. If no OnPeerClose is provided, we assume that the app
            // wants to close both directions and automatically enter the Closing state.
            if (mState == State::kConnected && OnPeerClose != nullptr)
            {
                mState = State::kReceiveShutdown;
            }
            else
            {
                mState = State::kClosing;
            }
            // Do not wait for ability to read on this endpoint.
            (void) static_cast<System::LayerOpenIoTSDK &>(GetSystemLayer()).DisableSelectCallback(this, true, false);
            // Call the app's OnPeerClose.
            if (OnPeerClose != nullptr)
            {
                OnPeerClose(this);
            }
        }

        // Otherwise, add the new data onto the receive queue.
        else
        {
            VerifyOrDie(rcvLen > 0);
            size_t newDataLength = rcvBuf->DataLength() + static_cast<size_t>(rcvLen);
            VerifyOrDie(CanCastTo<uint16_t>(newDataLength));
            if (isNewBuf)
            {
                rcvBuf->SetDataLength(static_cast<uint16_t>(newDataLength));
                rcvBuf.RightSize();
                if (mRcvQueue.IsNull())
                {
                    mRcvQueue = std::move(rcvBuf);
                }
                else
                {
                    mRcvQueue->AddToEnd(std::move(rcvBuf));
                }
            }
            else
            {
                rcvBuf->SetDataLength(static_cast<uint16_t>(newDataLength), mRcvQueue);
            }
        }
    }

    // Drive any received data into the app.
    DriveReceiving();
}

void TCPEndPointImplIoTSocket::HandleIncomingConnection()
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    TCPEndPointImplIoTSocket * conEP = nullptr;
    IPAddress peerAddr;
    uint16_t peerPort;
    int32_t conSocket;

    if (mSocketFamily == IOT_SOCKET_AF_INET6)
    {
        struct iot_in6_addr iot_address;
        memset(&iot_address, 0, sizeof(iot_address));
        uint32_t address_length = sizeof(iot_address);

        conSocket = iotSocketAccept(mSocket, (uint8_t *) &iot_address, &address_length, &peerPort);
        peerAddr  = IPAddress(iot_address);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (mSocketFamily == IOT_SOCKET_AF_INET)
    {
        struct iot_in_addr iot_address;
        memset(&iot_address, 0, sizeof(iot_address));
        uint32_t address_length = sizeof(iot_address);

        conSocket = iotSocketAccept(mSocket, (uint8_t *) &iot_address, &address_length, &peerPort);
        peerAddr  = IPAddress(iot_address);
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    // Accept the new connection.

    if (conSocket < 0)
    {
        err = MapErrorIoTSocket(conSocket);
        if (err == CHIP_ERROR_BUSY)
        {
            return;
        }
    }

    // If there's no callback available, fail with an error.
    if (err == CHIP_NO_ERROR && OnConnectionReceived == nullptr)
    {
        err = CHIP_ERROR_NO_CONNECTION_HANDLER;
    }

    // Attempt to allocate an end point object.
    if (err == CHIP_NO_ERROR)
    {
        TCPEndPoint * connectEndPoint = nullptr;
        err                           = GetEndPointManager().NewEndPoint(&connectEndPoint);
        conEP                         = static_cast<TCPEndPointImplIoTSocket *>(connectEndPoint);
    }

    // If all went well...
    if (err == CHIP_NO_ERROR)
    {
        // Put the new end point into the Connected state.
        conEP->mSocket = conSocket;

        conEP->mState = State::kConnected;
        conEP->Retain();

        // Wait for ability to read on this endpoint.
        auto & conEPLayer = static_cast<System::LayerOpenIoTSDK &>(conEP->GetSystemLayer());
        err               = conEPLayer.EnableSelectCallback(conEP, true, false);

        if (err == CHIP_NO_ERROR)
        {
            // Call the app's callback function.
            OnConnectionReceived(this, conEP, peerAddr, peerPort);
            return;
        }
    }

    // Otherwise immediately close the connection, clean up and call the app's error callback.
    if (conSocket != -1)
    {
        iotSocketClose(conSocket);
    }
    if (conEP != nullptr)
    {
        if (conEP->mState == State::kConnected)
        {
            conEP->Release();
        }
        conEP->Release();
    }
    if (OnAcceptError != nullptr)
    {
        OnAcceptError(this, err);
    }
} // namespace Inet

} // namespace Inet
} // namespace chip
