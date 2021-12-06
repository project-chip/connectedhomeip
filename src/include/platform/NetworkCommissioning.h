/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace DeviceLayer {

class DLL_EXPORT WiFiNetworkCommissioningDelegate
{
public:
    class Callback
    {
    public:
        virtual void OnConnectWiFiResult(app::Clusters::NetworkCommissioning::NetworkCommissioningStatus commissioningError,
                                         CharSpan debugText, int32_t wifiAssociationStatus) = 0;

        /**
         * Indicates the scan is finished, and accepts a list of networks discovered.
         * OnScanFinished() must be called in a thread-safe manner with CHIP stack. (e.g. using ScheduleWork or ScheduleLambda)
         */
        virtual void
        OnScanFinished(CHIP_ERROR err, CharSpan debugText,
                       const Span<app::Clusters::NetworkCommissioning::Structs::WiFiInterfaceScanResult::Type> & network) = 0;

        virtual ~Callback() = default;
    };

    /**
     * @brief
     *   Called to initialtiate a WiFi network scan, exactly one of callback->OnError or callback->OnScanFinished must be called
     * within timeout.
     *   The ScanWiFiNetworks has no return values, platforms can call callback->OnError() inside ScanWiFiNetworks.
     *
     * @param timeout The timeout of the scan, the scan MUST be finished or terminated within the given timeout.
     * @param ssid    The interested SSID, the scanning MAY be restricted to to the given SSID.
     */
    virtual void ConnectWiFiNetwork(ByteSpan ssid, ByteSpan credentials, Callback * callback) = 0;

    /**
     * @brief
     *   Called to initialtiate a WiFi network scan, exactly one of callback->OnError or callback->OnScanFinished must be called
     * within timeout.
     *   The ScanWiFiNetworks has no return values, platforms can call callback->OnError() inside ScanWiFiNetworks.
     *
     * @param timeout The timeout of the scan, the scan MUST be finished or terminated within the given timeout.
     * @param ssid    The interested SSID, the scanning MAY be restricted to to the given SSID.
     */
    virtual void ScanWiFiNetworks(ByteSpan ssid, Callback * callback) = 0;

    virtual uint8_t GetScanNetworkTimeoutSeconds()    = 0;
    virtual uint8_t GetConnectNetworkTimeoutSeconds() = 0;

    virtual ~WiFiNetworkCommissioningDelegate() = default;
};

class ThreadNetworkCommissioningDelegate
{
public:
    class Callback
    {
    public:
        virtual void OnConnectThreadResult(app::Clusters::NetworkCommissioning::NetworkCommissioningStatus commissioningError,
                                           CharSpan debugText, int32_t threadAssociationStatus) = 0;

        /**
         * Indicates the scan is finished, and accepts a list of networks discovered.
         * OnScanFinished() must be called in a thread-safe manner with CHIP stack. (e.g. using ScheduleWork or ScheduleLambda)
         */
        virtual void
        OnScanFinished(CHIP_ERROR err, CharSpan debugText,
                       const Span<app::Clusters::NetworkCommissioning::Structs::ThreadInterfaceScanResult::Type> & network) = 0;

        virtual ~Callback() = default;
    };

    virtual void ConnectThreadNetwork(ByteSpan threadData, Callback * callback) = 0;

    /**
     * @brief
     *   Called to initialtiate a Thread network scan, exactly one of callback->OnError or callback->OnScanFinished must be called.
     *   The ScanThreadNetworks has no return values, platforms can call callback->OnError() inside ScanThreadNetworks.
     *
     * @param timeout The timeout of the scan, the scan MUST be finished or terminated within the given timeout.
     */
    virtual void ScanThreadNetworks(Callback * callback) = 0;

    virtual uint8_t GetScanNetworkTimeoutSeconds()    = 0;
    virtual uint8_t GetConnectNetworkTimeoutSeconds() = 0;

    virtual ~ThreadNetworkCommissioningDelegate() {}
};

} // namespace DeviceLayer
} // namespace chip
