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

CHIP_ERROR ProxyTransportBase::Init(const ProxyListenParameters & params)
{
    mSystemLayer = params.GetSystemLayer();
    return CHIP_NO_ERROR;
}

void ProxyTransportBase::Activate(uint16_t sessionId, ProxyTransportDelegate * delegate)
{
    ChipLogProgress(Inet, "ProxyTransport: activating session %u", sessionId);
    mSessionId = sessionId;
    mDelegate  = delegate;
    mActive    = true;
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
    return address.GetTransportType() == Type::kProxy;
}

CHIP_ERROR ProxyTransportBase::SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    VerifyOrReturnError(address.GetTransportType() == Type::kProxy, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mActive, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint16_t sessionId = address.GetProxySessionId();

    ChipLogDetail(Inet, "ProxyTransport: forwarding %u bytes for session %u",
                  static_cast<unsigned>(msgBuf->DataLength()), sessionId);

    return mDelegate->SendProxyMessage(sessionId, ByteSpan(msgBuf->Start(), msgBuf->DataLength()));
}

void ProxyTransportBase::OnProxyMessageReceived(uint16_t sessionId, const uint8_t * data, size_t length)
{
    if (!mActive || sessionId != mSessionId)
    {
        ChipLogError(Inet, "ProxyTransport: received message for unknown session %u (active=%d, expected=%u)",
                     sessionId, (int) mActive, mSessionId);
        return;
    }

    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(data, length);
    if (buf.IsNull())
    {
        ChipLogError(Inet, "ProxyTransport: out of memory for received message");
        return;
    }

    ChipLogDetail(Inet, "ProxyTransport: injecting %u bytes for session %u into Matter stack",
                  static_cast<unsigned>(length), sessionId);

    HandleMessageReceived(PeerAddress::Proxy(sessionId), std::move(buf));
}

} // namespace Transport
} // namespace chip
