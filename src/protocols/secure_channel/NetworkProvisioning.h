/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines the CHIP Device Network Provisioning object.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <protocols/Protocols.h>
#include <support/BufferWriter.h>
#include <system/SystemPacketBuffer.h>
#include <transport/RendezvousSessionDelegate.h>
#include <transport/SecureSessionMgr.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

namespace chip {

class DLL_EXPORT NetworkProvisioningDelegate
{
public:
    /**
     * @brief
     *   Called when network provisioning generates a new message that should be sent to peer.
     *
     * @param protocol Protocol ID for the message
     * @param msgType Message type
     * @param msgBuf the new message that should be sent to the peer
     * @return CHIP_ERROR Error thrown when sending the message
     */
    virtual CHIP_ERROR SendSecureMessage(Protocols::Id protocol, uint8_t msgType, System::PacketBufferHandle msgBuf)
    {
        return CHIP_NO_ERROR;
    }

    /**
     * @brief
     *   Called when network provisioning fails with an error
     *
     * @param error error code
     */
    virtual void OnNetworkProvisioningError(CHIP_ERROR error) {}

    /**
     * @brief
     *   Called when the network provisioning is complete
     */
    virtual void OnNetworkProvisioningComplete() {}

    virtual ~NetworkProvisioningDelegate() {}
};

class DLL_EXPORT NetworkProvisioning : public Messaging::ExchangeDelegate
{
public:
    enum MsgTypes : uint8_t
    {
        kWiFiAssociationRequest   = 0,
        kIPAddressAssigned        = 1,
        kThreadAssociationRequest = 2
    };

    void Init(Messaging::ExchangeManager * exchangeMgr, SecureSessionHandle session, NetworkProvisioningDelegate * delegate);

    ~NetworkProvisioning();

    CHIP_ERROR SendNetworkCredentials(const char * ssid, const char * passwd);
    CHIP_ERROR SendThreadCredentials(ByteSpan threadData);

    CHIP_ERROR HandleNetworkProvisioningMessage(uint8_t msgType, const System::PacketBufferHandle & msgBuf);

    /**
     * @brief
     *  Get the IP address assigned to the device during network provisioning
     *  process.
     *
     * @return The IP address of the device
     */
    const Inet::IPAddress & GetIPAddress() const { return mDeviceAddress; }

    //// ExchangeDelegate Implementation ////
    void OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                           const PayloadHeader & payloadHeader, System::PacketBufferHandle payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * exchangeContext) override {}

private:
    NetworkProvisioningDelegate * mDelegate = nullptr;

    Messaging::ExchangeManager * mExchangeMgr = nullptr;

    Inet::IPAddress mDeviceAddress = Inet::IPAddress::Any;

    SecureSessionHandle mSession;

    /**
     * @brief
     *  The device can use this function to send its IP address to
     *  commissioner. This would generally be called during network
     *  provisioning of the device, after the IP address assignment.
     *
     * @param addr The IP address of the device
     */
    CHIP_ERROR SendIPAddress(const Inet::IPAddress & addr);

    /**
     * @brief
     *  The device can use this function to send its current IP address to
     *  commissioner. This would generally be called during network
     *  provisioning of the device, when the device already has an IP address.
     */
    CHIP_ERROR SendCurrentIPv4Address();

    static size_t EncodedStringSize(const char * str);
    static CHIP_ERROR EncodeString(const char * str, Encoding::LittleEndian::BufferWriter & bbuf);
    static CHIP_ERROR DecodeString(const uint8_t * input, size_t input_len, Encoding::LittleEndian::BufferWriter & bbuf,
                                   size_t & consumed);

    CHIP_ERROR DecodeThreadAssociationRequest(const System::PacketBufferHandle & msgBuf);

    CHIP_ERROR SendMessageUsingExchange(uint8_t msgType, System::PacketBufferHandle msgPayload);

#if CONFIG_DEVICE_LAYER
    static void ConnectivityHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
#endif // CONFIG_DEVICE_LAYER
};

} // namespace chip
