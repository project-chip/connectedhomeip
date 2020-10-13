/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/GenericNetworkProvisioningServerImpl.cpp>
#include <platform/internal/NetworkProvisioningServer.h>

#include <core/CHIPTLV.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include "esp_event.h"
#include "esp_wifi.h"

using namespace ::chip;
using namespace ::chip::TLV;

using Profiles::kChipProfile_Common;

namespace chip {
namespace DeviceLayer {
namespace Internal {

NetworkProvisioningServerImpl NetworkProvisioningServerImpl::sInstance;

CHIP_ERROR NetworkProvisioningServerImpl::_Init(void)
{
    return GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>::DoInit();
}

void NetworkProvisioningServerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    CHIP_ERROR err;

    // Handle ESP system events...
    if (event->Type == DeviceEventType::kESPSystemEvent)
    {
        switch (event->Platform.ESPSystemEvent.event_id)
        {
        case SYSTEM_EVENT_SCAN_DONE:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_SCAN_DONE");
            HandleScanDone();
            break;
        default:
            break;
        }
    }

    // Handle a change in WiFi connectivity...
    else if (event->Type == DeviceEventType::kWiFiConnectivityChange)
    {
        // Whenever WiFi connectivity is established, update the persisted WiFi
        // station security type to match that used by the connected AP.
        if (event->WiFiConnectivityChange.Result == kConnectivity_Established)
        {
            wifi_ap_record_t ap_info;

            err = esp_wifi_sta_get_ap_info(&ap_info);
            SuccessOrExit(err);

            WiFiSecurityType secType = ESP32Utils::WiFiAuthModeToChipWiFiSecurityType(ap_info.authmode);

            err = ConfigurationMgrImpl().UpdateWiFiStationSecurityType(secType);
            SuccessOrExit(err);
        }
    }

    // Propagate the event to the GenericNetworkProvisioningServerImpl<> base class so
    // that it can take action on specific events.
    GenericImplClass::_OnPlatformEvent(event);

exit:
    return;
}

CHIP_ERROR NetworkProvisioningServerImpl::GetWiFiStationProvision(NetworkInfo & netInfo, bool includeCredentials)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t stationConfig;

    netInfo.Reset();

    err = esp_wifi_get_config(ESP_IF_WIFI_STA, &stationConfig);
    SuccessOrExit(err);

    VerifyOrExit(stationConfig.sta.ssid[0] != 0, err = CHIP_ERROR_INCORRECT_STATE);

    netInfo.NetworkId              = kWiFiStationNetworkId;
    netInfo.FieldPresent.NetworkId = true;
    netInfo.NetworkType            = kNetworkType_WiFi;
    memcpy(netInfo.WiFiSSID, stationConfig.sta.ssid, min(strlen((char *) stationConfig.sta.ssid) + 1, sizeof(netInfo.WiFiSSID)));
    netInfo.WiFiMode = kWiFiMode_Managed;
    netInfo.WiFiRole = kWiFiRole_Station;

    err = ConfigurationMgrImpl().GetWiFiStationSecurityType(netInfo.WiFiSecurityType);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    if (includeCredentials)
    {
        netInfo.WiFiKeyLen = min(strlen((char *) stationConfig.sta.password), sizeof(netInfo.WiFiKey));
        memcpy(netInfo.WiFiKey, stationConfig.sta.password, netInfo.WiFiKeyLen);
    }

exit:
    return err;
}

CHIP_ERROR NetworkProvisioningServerImpl::SetWiFiStationProvision(const NetworkInfo & netInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t wifiConfig;

    // Ensure that ESP station mode is enabled.  This is required before esp_wifi_set_config(ESP_IF_WIFI_STA,...)
    // can be called.
    err = ESP32Utils::EnableStationMode();
    SuccessOrExit(err);

    // Initialize an ESP wifi_config_t structure based on the new provision information.
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.sta.ssid, netInfo.WiFiSSID, min(strlen(netInfo.WiFiSSID) + 1, sizeof(wifiConfig.sta.ssid)));
    memcpy(wifiConfig.sta.password, netInfo.WiFiKey, min((size_t) netInfo.WiFiKeyLen, sizeof(wifiConfig.sta.password)));
    if (netInfo.WiFiSecurityType == kWiFiSecurityType_NotSpecified)
    {
        wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    }
    else
    {
        wifiConfig.sta.scan_method    = WIFI_FAST_SCAN;
        wifiConfig.sta.threshold.rssi = 0;
        switch (netInfo.WiFiSecurityType)
        {
        case kWiFiSecurityType_None:
            wifiConfig.sta.threshold.authmode = WIFI_AUTH_OPEN;
            break;
        case kWiFiSecurityType_WEP:
            wifiConfig.sta.threshold.authmode = WIFI_AUTH_WEP;
            break;
        case kWiFiSecurityType_WPAPersonal:
            wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;
            break;
        case kWiFiSecurityType_WPA2Personal:
            wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            break;
        case kWiFiSecurityType_WPA2Enterprise:
            wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE;
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
        }
    }
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;

    // Configure the ESP WiFi interface.
    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", chip::ErrorStr(err));
    }
    SuccessOrExit(err);

    // Store the WiFi Station security type separately in NVS.  This is necessary because the ESP wifi API
    // does not provide a way to query the configured WiFi auth mode.
    err = ConfigurationMgrImpl().UpdateWiFiStationSecurityType(netInfo.WiFiSecurityType);
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "WiFi station provision set (SSID: %s)", netInfo.WiFiSSID);

exit:
    return err;
}

CHIP_ERROR NetworkProvisioningServerImpl::ClearWiFiStationProvision(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t stationConfig;

    // Clear the ESP WiFi station configuration.
    memset(&stationConfig, 0, sizeof(stationConfig));
    esp_wifi_set_config(ESP_IF_WIFI_STA, &stationConfig);

    // Clear the persisted WiFi station security type.
    ConfigurationMgrImpl().UpdateWiFiStationSecurityType(kWiFiSecurityType_NotSpecified);

    return err;
}

CHIP_ERROR NetworkProvisioningServerImpl::InitiateWiFiScan(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_scan_config_t scanConfig;

    // Initiate an active scan using the default dwell times.  Configure the scan to return hidden networks.
    memset(&scanConfig, 0, sizeof(scanConfig));
    scanConfig.show_hidden = 1;
    scanConfig.scan_type   = WIFI_SCAN_TYPE_ACTIVE;
    err                    = esp_wifi_scan_start(&scanConfig, false);
    SuccessOrExit(err);

#if CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT
    // Arm timer in case we never get the scan done event.
    SystemLayer.StartTimer(CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT, HandleScanTimeOut, NULL);
#endif // CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT

exit:
    return err;
}

void NetworkProvisioningServerImpl::HandleScanDone()
{
    CHIP_ERROR err;
    wifi_ap_record_t * scanResults = NULL;
    uint16_t scanResultCount;
    uint16_t encodedResultCount;
    PacketBuffer * respBuf = NULL;

    // If we receive a SCAN DONE event for a scan that we didn't initiate, simply ignore it.
    VerifyOrExit(mState == kState_ScanNetworks_InProgress, err = CHIP_NO_ERROR);

    mState = kState_Idle;

#if CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT
    // Cancel the scan timeout timer.
    SystemLayer.CancelTimer(HandleScanTimeOut, NULL);
#endif // CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT

    // Determine the number of scan results found.
    err = esp_wifi_scan_get_ap_num(&scanResultCount);
    SuccessOrExit(err);

    // Only return up to CHIP_DEVICE_CONFIG_MAX_SCAN_NETWORKS_RESULTS.
    scanResultCount = min(scanResultCount, (uint16_t) CHIP_DEVICE_CONFIG_MAX_SCAN_NETWORKS_RESULTS);

    // Allocate a buffer to hold the scan results array.
    scanResults = (wifi_ap_record_t *) chip::Platform::MemoryAlloc(scanResultCount * sizeof(wifi_ap_record_t));
    VerifyOrExit(scanResults != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Collect the scan results from the ESP WiFi driver.  Note that this also *frees*
    // the internal copy of the results.
    err = esp_wifi_scan_get_ap_records(&scanResultCount, scanResults);
    SuccessOrExit(err);

    // If the ScanNetworks request is still outstanding...
    if (GetCurrentOp() == kMsgType_ScanNetworks)
    {
        chip::TLV::TLVWriter writer;
        TLVType outerContainerType;

        // Sort results by rssi.
        qsort(scanResults, scanResultCount, sizeof(*scanResults), ESP32Utils::OrderScanResultsByRSSI);

        // Allocate a packet buffer to hold the encoded scan results.
        respBuf = PacketBuffer::New(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE + 1);
        VerifyOrExit(respBuf != NULL, err = CHIP_ERROR_NO_MEMORY);

        // Encode the list of scan results into the response buffer.  If the encoded size of all
        // the results exceeds the size of the buffer, encode only what will fit.
        writer.Init(respBuf, respBuf->AvailableDataLength() - 1);
        err = writer.StartContainer(AnonymousTag, kTLVType_Array, outerContainerType);
        SuccessOrExit(err);
        for (encodedResultCount = 0; encodedResultCount < scanResultCount; encodedResultCount++)
        {
            NetworkInfo netInfo;
            const wifi_ap_record_t & scanResult = scanResults[encodedResultCount];

            netInfo.Reset();
            netInfo.NetworkType = kNetworkType_WiFi;
            memcpy(netInfo.WiFiSSID, scanResult.ssid,
                   min(strlen((char *) scanResult.ssid) + 1, (size_t) NetworkInfo::kMaxWiFiSSIDLength));
            netInfo.WiFiSSID[NetworkInfo::kMaxWiFiSSIDLength] = 0;
            netInfo.WiFiMode                                  = kWiFiMode_Managed;
            netInfo.WiFiRole                                  = kWiFiRole_Station;
            netInfo.WiFiSecurityType                          = ESP32Utils::WiFiAuthModeToChipWiFiSecurityType(scanResult.authmode);
            netInfo.WirelessSignalStrength                    = scanResult.rssi;

            {
                chip::TLV::TLVWriter savePoint = writer;
                err                            = netInfo.Encode(writer);
                if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
                {
                    writer = savePoint;
                    break;
                }
            }
            SuccessOrExit(err);
        }
        err = writer.EndContainer(outerContainerType);
        SuccessOrExit(err);
        err = writer.Finalize();
        SuccessOrExit(err);

        // Send the scan results to the requestor.  Note that this method takes ownership of the
        // buffer, success or fail.
        err     = SendNetworkScanComplete(encodedResultCount, respBuf);
        respBuf = NULL;
        SuccessOrExit(err);
    }

exit:
    PacketBuffer::Free(respBuf);

    // If an error occurred and we haven't yet responded, send a Internal Error back to the
    // requestor.
    if (err != CHIP_NO_ERROR && GetCurrentOp() == kMsgType_ScanNetworks)
    {
        SendStatusReport(kChipProfile_Common, kStatus_InternalError, err);
    }

    // Tell the ConnectivityManager that the WiFi scan is now done.  This allows it to continue
    // any activities that were deferred while the scan was in progress.
    ConnectivityMgr().OnWiFiScanDone();
}

#if CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT

void NetworkProvisioningServerImpl::HandleScanTimeOut(::chip::System::Layer * aLayer, void * aAppState,
                                                      ::chip::System::Error aError)
{
    ChipLogError(DeviceLayer, "WiFi scan timed out");

    // Reset the state.
    sInstance.mState = kState_Idle;

    // Verify that the ScanNetworks request is still outstanding; if so, send a
    // Common:InternalError StatusReport to the client.
    if (sInstance.GetCurrentOp() == kMsgType_ScanNetworks)
    {
        sInstance.SendStatusReport(kChipProfile_Common, kStatus_InternalError, CHIP_ERROR_TIMEOUT);
    }

    // Tell the ConnectivityManager that the WiFi scan is now done.
    ConnectivityMgr().OnWiFiScanDone();
}

#endif // CHIP_DEVICE_CONFIG_WIFI_SCAN_COMPLETION_TIMEOUT

bool NetworkProvisioningServerImpl::IsSupportedWiFiSecurityType(WiFiSecurityType_t wifiSecType)
{
    return (wifiSecType == kWiFiSecurityType_None || wifiSecType == kWiFiSecurityType_WEP ||
            wifiSecType == kWiFiSecurityType_WPAPersonal || wifiSecType == kWiFiSecurityType_WPA2Personal ||
            wifiSecType == kWiFiSecurityType_WPA2Enterprise);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
