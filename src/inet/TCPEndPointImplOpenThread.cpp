/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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

#include <inet/TCPEndPointImplOpenThread.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/OpenThread/OpenThreadUtils.h>

#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Inet {

CHIP_ERROR TCPEndPointImplOT::GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::GetInterfaceId(InterfaceId * retInterface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::EnableNoDelay()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::EnableKeepAlive(uint16_t interval, uint16_t timeoutCount)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::DisableKeepAlive()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::AckReceive(uint16_t len)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR TCPEndPointImplOT::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::ListenImpl(uint16_t backlog)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::ConnectImpl(const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::SendQueuedImpl(bool queueWasEmpty)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::SetUserTimeoutImpl(uint32_t userTimeoutMillis)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
CHIP_ERROR TCPEndPointImplOT::DriveSendingImpl()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
void TCPEndPointImplOT::HandleConnectCompleteImpl()
{
    // Not implemented
}
void TCPEndPointImplOT::DoCloseImpl(CHIP_ERROR err, State oldState)
{
    // Not implemented
}

} // namespace Inet
} // namespace chip
