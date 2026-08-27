/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <transport/raw/ProxyTransport.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Transport {

CHIP_ERROR ProxyTransportBase::Init(const ProxyListenParameters &)
{
    // No network resources to allocate; the transport stays inert until Activate().
    return CHIP_NO_ERROR;
}

CHIP_ERROR ProxyTransportBase::Activate(uint16_t sessionId, ProxyTransportDelegate * delegate)
{
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!mActive, CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Inet, "ProxyTransport: activating session %u", sessionId);
    mSessionId = sessionId;
    mDelegate  = delegate;
    mActive    = true;
    return CHIP_NO_ERROR;
}

void ProxyTransportBase::Deactivate()
{
    ChipLogProgress(Inet, "ProxyTransport: deactivating session %u", mSessionId);
    mActive    = false;
    mDelegate  = nullptr;
    mSessionId = 0;
}

void ProxyTransportBase::Close()
{
    Deactivate();
}

bool ProxyTransportBase::CanSendToPeer(const PeerAddress & address)
{
    // Only claim a kProxy address for the active session.  A PeerAddress built from a
    // previous session id can outlive that session in a SessionHolder or an exchange;
    // claiming it would tunnel traffic to the wrong commissionee.
    return mActive && address.GetTransportType() == Type::kProxy && address.GetProxySessionId() == mSessionId;
}

CHIP_ERROR ProxyTransportBase::SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(address.GetTransportType() == Type::kProxy, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mActive, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Same reasoning as CanSendToPeer: never forward under a session id other than the
    // active one.
    uint16_t sessionId = address.GetProxySessionId();
    VerifyOrReturnError(sessionId == mSessionId, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(Inet, "ProxyTransport: forwarding %u bytes for session %u", static_cast<unsigned>(msgBuf->DataLength()),
                  sessionId);

    return mDelegate->SendProxyMessage(sessionId, ByteSpan(msgBuf->Start(), msgBuf->DataLength()));
}

CHIP_ERROR ProxyTransportBase::OnProxyMessageReceived(uint16_t sessionId, ByteSpan message)
{
    VerifyOrReturnError(mActive && sessionId == mSessionId, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(Inet, "ProxyTransport: received message for unknown session %u (active=%d, expected=%u)",
                                     sessionId, (int) mActive, mSessionId));

    // NewWithData() memcpy()s from message.data(), which an empty span leaves null, and a
    // zero-length packet carries nothing for the stack to parse.
    VerifyOrReturnError(!message.empty(), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Inet, "ProxyTransport: empty message for session %u", sessionId));

    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(message.data(), message.size());
    VerifyOrReturnError(!buf.IsNull(), CHIP_ERROR_NO_MEMORY,
                        ChipLogError(Inet, "ProxyTransport: out of memory for received message"));

    ChipLogDetail(Inet, "ProxyTransport: injecting %u bytes for session %u into Matter stack",
                  static_cast<unsigned>(message.size()), sessionId);

    HandleMessageReceived(PeerAddress::Proxy(sessionId), std::move(buf));
    return CHIP_NO_ERROR;
}

} // namespace Transport
} // namespace chip
