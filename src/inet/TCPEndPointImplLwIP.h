/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
        TCPEndPoint(endPointManager), mUnackedLength(0), mTCP(nullptr)
    {}

    // TCPEndPoint overrides.
    CHIP_ERROR GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const override;
    CHIP_ERROR GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const override;
    CHIP_ERROR GetInterfaceId(InterfaceId * retInterface) override;
    CHIP_ERROR EnableNoDelay() override;
    CHIP_ERROR EnableKeepAlive(uint16_t interval, uint16_t timeoutCount) override;
    CHIP_ERROR DisableKeepAlive() override;
    CHIP_ERROR AckReceive(uint16_t len) override;
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
