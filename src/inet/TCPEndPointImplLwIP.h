/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This header file defines the <tt>Inet::TCPEndPoint</tt>
 *      class, where the CHIP Inet Layer encapsulates methods for
 *      interacting with TCP transport endpoints (SOCK_DGRAM sockets
 *      on Linux and BSD-derived systems) or LwIP TCP protocol
 *      control blocks, as the system is configured accordingly.
 */

/**
 * This file declares an implementation of Inet::TCPEndPoint using LwIP.
 */

#pragma once

#include <inet/EndPointStateLwIP.h>
#include <inet/TCPEndPoint.h>

namespace chip {
namespace Inet {

class TCPEndPointImplLwIP : public TCPEndPoint, public EndPointStateLwIP
{
public:
    TCPEndPointImplLwIP(EndPointManager<TCPEndPoint> & endPointManager) :
        TCPEndPoint(endPointManager), mUnackedLength(0), mTCP(nullptr), mPreAllocatedConnectEP(nullptr)
    {}

    // TCPEndPoint overrides.
    CHIP_ERROR GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const override;
    CHIP_ERROR GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const override;
    CHIP_ERROR GetInterfaceId(InterfaceId * retInterface) override;
    CHIP_ERROR EnableNoDelay() override;
    CHIP_ERROR EnableKeepAlive(uint16_t interval, uint16_t timeoutCount) override;
    CHIP_ERROR DisableKeepAlive() override;
    CHIP_ERROR AckReceive(size_t len) override;
#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    void TCPUserTimeoutHandler() override;
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

private:
    // TCPEndPoint overrides.
    CHIP_ERROR BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr) override;
    CHIP_ERROR ListenImpl(uint16_t backlog) override;
    CHIP_ERROR ConnectImpl(const IPAddress & addr, uint16_t port, InterfaceId intfId) override;
    CHIP_ERROR SendQueuedImpl(bool queueWasEmpty) override;
    CHIP_ERROR SetUserTimeoutImpl(uint32_t userTimeoutMillis) override;
    CHIP_ERROR DriveSendingImpl() override;
    void HandleConnectCompleteImpl() override;
    void DoCloseImpl(CHIP_ERROR err, State oldState) override;

    struct BufferOffset
    {
        BufferOffset(System::PacketBufferHandle && aBuffer) : buffer(std::move(aBuffer)), offset(0) {}
        BufferOffset(BufferOffset && aOther)
        {
            buffer = std::move(aOther.buffer);
            offset = aOther.offset;
        }
        chip::System::PacketBufferHandle buffer;
        uint16_t offset;
    };

    uint16_t mUnackedLength; // Amount sent but awaiting ACK. Used as a form of reference count
                             // to hang-on to backing packet buffers until they are no longer needed.
    tcp_pcb * mTCP;          // LwIP Transmission control protocol (TCP) control block.
    // For TCP Listen endpoint, we will pre-allocate a connection endpoint to assign the incoming connection to it.
    // when there is a new TCP connection established.
    TCPEndPoint * mPreAllocatedConnectEP;

    uint16_t RemainingToSend();
    BufferOffset FindStartOfUnsent();
    CHIP_ERROR GetPCB(IPAddressType addrType);
    void HandleDataSent(uint16_t len);
    void HandleDataReceived(chip::System::PacketBufferHandle && buf);
    void HandleIncomingConnection(TCPEndPoint * pcb);
    void HandleError(CHIP_ERROR err);

    static err_t LwIPHandleConnectComplete(void * arg, struct tcp_pcb * tpcb, err_t lwipErr);
    static err_t LwIPHandleIncomingConnection(void * arg, struct tcp_pcb * tcpConPCB, err_t lwipErr);
    static err_t LwIPHandleDataReceived(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err);
    static err_t LwIPHandleDataSent(void * arg, struct tcp_pcb * tpcb, u16_t len);
    static void LwIPHandleError(void * arg, err_t err);
};

using TCPEndPointImpl = TCPEndPointImplLwIP;

} // namespace Inet
} // namespace chip
