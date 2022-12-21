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
 * This file declares an implementation of Inet::TCPEndPoint using sockets.
 */

#pragma once

#include <inet/EndPointStateIoTSocket.h>
#include <inet/TCPEndPoint.h>

namespace chip {
namespace Inet {

class TCPEndPointImplIoTSocket : public TCPEndPoint, public EndPointStateIoTSocket
{
public:
    TCPEndPointImplIoTSocket(EndPointManager<TCPEndPoint> & endPointManager) : TCPEndPoint(endPointManager) {}

    void SelectCallback(void * readMask, void * writeMask, void * exceptionMask) override;

    // TCPEndPoint overrides.
    CHIP_ERROR GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const override;
    CHIP_ERROR GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const override;
    CHIP_ERROR GetInterfaceId(InterfaceId * retInterface) override;
    CHIP_ERROR EnableNoDelay() override;
    CHIP_ERROR EnableKeepAlive(uint16_t interval, uint16_t timeoutCount) override;
    CHIP_ERROR DisableKeepAlive() override;
    CHIP_ERROR AckReceive(uint16_t len) override;

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

    CHIP_ERROR GetSocket(IPAddressType addrType);

    void ReceiveData();
    void HandleIncomingConnection();
    CHIP_ERROR BindSrcAddrFromIntf(IPAddressType addrType, InterfaceId intfId);

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    void TCPUserTimeoutHandler() override{};
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

private:
    CHIP_ERROR GetInfo(IPAddress * retAddr, uint16_t * retPort, bool local) const;
    InterfaceId mBoundInterface;
};

using TCPEndPointImpl = TCPEndPointImplIoTSocket;

} // namespace Inet
} // namespace chip
