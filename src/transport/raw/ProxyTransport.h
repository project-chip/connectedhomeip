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

/**
 * @file
 *   Defines a virtual transport that tunnels Matter commissioning packets
 *   through ProxyMessageRequest / ProxyMessageResponse IM commands.
 *
 *   chip-tool creates a Proxy transport, activates it with the sessionId
 *   returned by ProxyConnectResponse, and then calls PairDevice() with
 *   PeerAddress::Proxy(sessionId).  All subsequent commissioning packets
 *   flow via the ProxyTransportDelegate::SendProxyMessage() callback.
 *
 *   On the commissioning-proxy-app side, packets received from the
 *   commissionee over WiFi-PAF are delivered back to chip-tool by calling
 *   OnProxyMessageReceived(), which injects them into the Matter stack as
 *   if they came directly from the device.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

/** Init parameters struct (empty — no network setup required). */
struct ProxyListenParameters
{
    explicit ProxyListenParameters(chip::System::Layer * layer) : mSystemLayer(layer) {}
    chip::System::Layer * GetSystemLayer() const { return mSystemLayer; }

private:
    chip::System::Layer * mSystemLayer = nullptr;
};

/**
 * Callback interface implemented by chip-tool (or any initiator) to forward
 * a raw Matter packet to the commissioning-proxy-app via ProxyMessageRequest.
 */
class ProxyTransportDelegate
{
public:
    virtual ~ProxyTransportDelegate() = default;

    /**
     * Forward a raw Matter packet to the proxy for the given session.
     *
     * @param sessionId  The proxy session identifier from ProxyConnectResponse.
     * @param message    The raw Matter packet bytes to forward.
     */
    virtual CHIP_ERROR SendProxyMessage(uint16_t sessionId, chip::ByteSpan message) = 0;
};

/**
 * A virtual Transport::Base that routes Matter packets over the
 * ProxyMessageRequest / ProxyMessageResponse IM command path instead of a
 * real network interface.
 *
 * Usage:
 *   1. Add ProxyTransportBase (or the Proxy<N> template alias) to the
 *      DeviceTransportMgr type.
 *   2. After receiving ProxyConnectResponse, call:
 *        GetDeviceProxyTransport(transportMgr)->Activate(sessionId, delegate);
 *   3. Call PairDevice(nodeId, PeerAddress::Proxy(sessionId)).
 *   4. When ProxyMessageResponse arrives, call:
 *        OnProxyMessageReceived(sessionId, data, length);
 */
class DLL_EXPORT ProxyTransportBase : public Base
{
public:
    ProxyTransportBase()          = default;
    ~ProxyTransportBase() override = default;

    /**
     * Initialize the transport.  Called once by the TransportMgr during
     * system startup.  No network resources are allocated here.
     */
    CHIP_ERROR Init(const ProxyListenParameters & params);

    /**
     * Activate this transport for a specific proxy session.
     * Must be called after ProxyConnectResponse is received.
     */
    void Activate(uint16_t sessionId, ProxyTransportDelegate * delegate);

    /** Deactivate the proxy session. */
    void Deactivate();

    bool IsActive() const { return mActive; }
    uint16_t GetSessionId() const { return mSessionId; }

    // ------------------------------------------------------------------
    // Transport::Base interface
    CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf) override;
    bool CanSendToPeer(const PeerAddress & address) override;
    void Close() override;

    // ------------------------------------------------------------------
    /**
     * Called by the ProxyTransportDelegate when ProxyMessageResponse arrives.
     * Injects the bytes back into the Matter stack as a received packet from
     * the proxy's virtual peer address.
     */
    void OnProxyMessageReceived(uint16_t sessionId, const uint8_t * data, size_t length);

private:
    ProxyTransportDelegate * mDelegate  = nullptr;
    chip::System::Layer *    mSystemLayer = nullptr;
    uint16_t mSessionId                  = 0;
    bool     mActive                     = false;
};

/** Typed alias to match the BLE/WiFiPAF template pattern. */
template <size_t /* unused */ = 0>
class Proxy : public ProxyTransportBase
{};

} // namespace Transport
} // namespace chip
