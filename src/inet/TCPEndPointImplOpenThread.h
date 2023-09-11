/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inet/EndPointStateOpenThread.h>
#include <inet/TCPEndPoint.h>

#include <openthread/error.h>
#include <openthread/icmp6.h>
#include <openthread/ip6.h>
#include <openthread/netdata.h>
#include <openthread/tcp.h>
#include <openthread/thread.h>

namespace chip {
namespace Inet {

class TCPEndPointImplOT : public TCPEndPoint, public EndPointStateOpenThread
{
public:
    TCPEndPointImplOT(EndPointManager<TCPEndPoint> & endPointManager) :
        TCPEndPoint(endPointManager), mBoundIntfId(InterfaceId::Null())
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

    InterfaceId mBoundIntfId;
    uint16_t mBoundPort;
    otInstance * mOTInstance = nullptr;
};

using TCPEndPointImpl = TCPEndPointImplOT;

} // namespace Inet
} // namespace chip
