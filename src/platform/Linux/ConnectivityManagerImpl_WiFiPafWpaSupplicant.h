/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include "ConnectivityManagerImpl_WiFiPafInterface.h"
#include "WpaSupplicantClient.h"

namespace chip {
namespace DeviceLayer {

// Forward Declarations

struct ChipDeviceEvent;

/**
 *  Provides an implementation of the ConnectionManager object
 *  for Linux platforms where wpa_supplicant provides the
 *  Wi-Fi Neighbor Awareness Networking (NAN) Unsynchronized
 *  Service Discovery (USD) / Public Action Frame (PAF)
 *  commissioning transport.
 *
 */
class ConnectivityManagerImpl_WiFiPafWpaSupplicant final : public Internal::WiFiPafInterface, public Internal::WpaSupplicantClient
{
public:
    // Destruction

    virtual ~ConnectivityManagerImpl_WiFiPafWpaSupplicant() = default;

    // Initialization

    CHIP_ERROR Init(ConnectivityManagerImpl & inConnectivityManagerImpl) override final;

    // Event Handling

    void OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) override final;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    // Wi-Fi Neighbor Awareness Networking (NAN) Unsynchronized
    // Service Discovery (USD) / Public Action Frame (PAF)
    // Commissioning Transport

    // Initialization

    CHIP_ERROR WiFiPafShutdown(const uint32_t & inId, const WiFiPAF::WiFiPafRole & inWiFiPafRole) override final;

    // Introspection

    bool IsWiFiPafResourceAvailable() const override final;

    // Mutation

    CHIP_ERROR WiFiPafSetAdvertisingEnabled(bool inEnabled, uint32_t & inOutPublishId) override final;
    void WiFiPafSetParam(const ConnectivityManager::WiFiPAFAdvertiseParam & inWiFiPafAdvertiseParams) override final;
    CHIP_ERROR WiFiPafSetResourceAvailable(const bool & inAvailable) override final;
    void WiFiPafSetSubscribeFreq(const uint16_t & inFrequency) override final;

    // Publish-and-subscribe

    CHIP_ERROR WiFiPafPublish(ConnectivityManager::WiFiPAFAdvertiseParam & inOutWiFiPafAdvertiseParams) override final;
    CHIP_ERROR WiFiPafCancelPublish(const uint32_t & inPublishId) override final;
    CHIP_ERROR WiFiPafSubscribe(const uint16_t & inConnectionDiscriminator, void * inContext,
                                ConnectivityManager::OnConnectionCompleteFunct onSuccessFunc,
                                ConnectivityManager::OnConnectionErrorFunct onErrorFunc) override final;
    CHIP_ERROR WiFiPafCancelSubscribe(const uint32_t & inSubscribeId) override final;
    CHIP_ERROR WiFiPafCancelIncompleteSubscribe() override final;

    // Data Transmission

    CHIP_ERROR WiFiPafSend(const WiFiPAF::WiFiPAFSession & inWiFiPafSession,
                           chip::System::PacketBufferHandle && inOutMessageBuffer) override final;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

private:
    void OnDiscoveryResult(GVariant * obj);
    void OnReplied(GVariant * obj);
    void OnNanReceive(GVariant * obj);
    void OnNanPublishTerminated(guint public_id, gchar * reason);
    void OnNanSubscribeTerminated(guint subscribe_id, gchar * reason);

    // wpa_supplicant Base Class Method Overrides

    void OnWiFiMediumAvailable(WpaSupplicantClient & inOutWpaSupplicantClient, bool inAvailable) override final;
    void PostNetworkConnect() override final;
    void UpdateWiFiNetworkStatus() override final;

private:
    ConnectivityManagerImpl * mConnectivityManagerImpl;
    bool mPafChannelAvailable;
    ConnectivityManager::WiFiPAFAdvertiseParam mPafAdvertiseParams;
    ConnectivityManager::OnConnectionCompleteFunct mOnPafSubscribeComplete;
    ConnectivityManager::OnConnectionErrorFunct mOnPafSubscribeError;
    void * mAppState;
    uint16_t mSubscribeFreq;
};

} // namespace DeviceLayer
} // namespace chip
