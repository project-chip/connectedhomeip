/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *    @file
 *      This file defines the Matter Connection object that maintains a Wi-Fi PAF connection.
 *
 */

#pragma once

#include <lib/core/CHIPCore.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>
#include <utility>
#include <wifipaf/WiFiPAFLayer.h>

namespace chip {
namespace Transport {
class WiFiPAFListenParameters;

/**
 * Implements a transport using Wi-Fi-PAF
 */
class DLL_EXPORT WiFiPAFBase : public Base, public WiFiPAF::WiFiPAFLayerDelegate
{
public:
    WiFiPAFBase() = default;
    WiFiPAFBase(System::PacketBufferHandle * packetBuffers, size_t packetBuffersSize) :
        mPendingPackets(packetBuffers), mPendingPacketsSize(packetBuffersSize)
    {}
    ~WiFiPAFBase() override {}

    /**
     * Initialize a Wi-Fi-PAF transport
     *
     * @param param        Wi-Fi-PAF configuration parameters for this transport
     */
    CHIP_ERROR Init(const WiFiPAFListenParameters & param);
    bool CanSendToPeer(const Transport::PeerAddress & address) override;
    void SetWiFiPAFLayerTransportToSelf() { mWiFiPAFLayer->mWiFiPAFTransport = this; }
    bool IsWiFiPAFLayerTransportSetToSelf() { return mWiFiPAFLayer->mWiFiPAFTransport == this; }
    /**
     * Interface of Base
     */
    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override;
    /**
     * Interfaces of WiFiPAFLayerDelegate
     */
    CHIP_ERROR WiFiPAFMessageReceived(WiFiPAF::WiFiPAFSession & RxInfo, System::PacketBufferHandle && buffer) override;
    CHIP_ERROR WiFiPAFMessageSend(WiFiPAF::WiFiPAFSession & TxInfo, System::PacketBufferHandle && msg) override;
    CHIP_ERROR WiFiPAFCloseSession(WiFiPAF::WiFiPAFSession & SessionInfo) override;
    bool WiFiPAFResourceAvailable(void) override;

private:
    /**
     * Sends the specified message once a connection has been established.
     * @param msg - what buffer to send once a connection has been established.
     */
    CHIP_ERROR SendAfterConnect(System::PacketBufferHandle && msg);

    WiFiPAF::WiFiPAFLayer * mWiFiPAFLayer = nullptr; ///< Associated wifipaf layer

    System::PacketBufferHandle * mPendingPackets;
    size_t mPendingPacketsSize;
};

template <size_t kPendingPacketSize>
class WiFiPAF : public WiFiPAFBase
{
public:
    WiFiPAF() : WiFiPAFBase(mPendingPackets, kPendingPacketSize) {}

private:
    System::PacketBufferHandle mPendingPackets[kPendingPacketSize];
};

/** Defines parameters for setting up the Wi-Fi PAF transport */
class WiFiPAFListenParameters
{
public:
    WiFiPAFListenParameters() = default;
    explicit WiFiPAFListenParameters(WiFiPAFBase * layer) : mWiFiPAF(layer) {}
    WiFiPAFBase * GetWiFiPAFTransport() const { return mWiFiPAF; }

private:
    WiFiPAFBase * mWiFiPAF;
};

} // namespace Transport
} // namespace chip
