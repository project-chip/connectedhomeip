/*
 *
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

#ifndef DEVICE_NETWORK_INFO_H
#define DEVICE_NETWORK_INFO_H

namespace chip {
namespace DeviceLayer {

/**
 * Network Types.
 */
enum NetworkType
{
    kNetworkType_NotSpecified                   = -1,

    kNetworkType_WiFi                           = 1,
};

/**
 * WiFi Security Modes.
 */
enum WiFiSecurityType
{
    kWiFiSecurityType_NotSpecified              = -1,

    kWiFiSecurityType_None                      = 1,
    kWiFiSecurityType_WEP                       = 2,
    kWiFiSecurityType_WPAPersonal               = 3,
    kWiFiSecurityType_WPA2Personal              = 4,
    kWiFiSecurityType_WPA2MixedPersonal         = 5,
    kWiFiSecurityType_WPAEnterprise             = 6,
    kWiFiSecurityType_WPA2Enterprise            = 7,
    kWiFiSecurityType_WPA2MixedEnterprise       = 8,
    kWiFiSecurityType_WPA3Personal              = 9,
    kWiFiSecurityType_WPA3MixedPersonal         = 10,
    kWiFiSecurityType_WPA3Enterprise            = 11,
    kWiFiSecurityType_WPA3MixedEnterprise       = 12,
};

/**
 * WiFi Operating Modes.
 */
enum WiFiMode
{
    kWiFiMode_NotSpecified                      = -1,

    kWiFiMode_AdHoc                             = 1,
    kWiFiMode_Managed                           = 2
};

/**
 * Device WiFi Role.
 */
enum WiFiRole
{
    kWiFiRole_NotSpecified                      = -1,

    kWiFiRole_Station                           = 1,
    kWiFiRole_AccessPoint                       = 2
};

class DeviceNetworkInfo
{
public:
    enum
    {
        // ---- WiFi-specific Limits ----
        kMaxWiFiSSIDLength                  = 32,
        kMaxWiFiKeyLength                   = 64,

        // ---- Thread-specific Limits ----
        kMaxThreadNetworkNameLength         = 16,
        kThreadExtendedPANIdLength          = 8,
        kThreadMeshPrefixLength             = 8,
        kThreadNetworkKeyLength             = 16,
        kThreadPSKcLength                   = 16,
    };

    NetworkType mNetworkType;              /**< The type of network. */
    uint32_t mNetworkId;                     /**< The network id assigned to the network by the device. */

    // ---- WiFi-specific Fields ----
    char mWiFiSSID[kMaxWiFiSSIDLength + 1];  /**< The WiFi SSID as a NULL-terminated string. */
    WiFiMode mWiFiMode;                    /**< The operating mode of the WiFi network.*/
    WiFiRole mWiFiRole;                    /**< The role played by the device on the WiFi network. */
    WiFiSecurityType mWiFiSecurityType;    /**< The WiFi security type. */
    uint8_t mWiFiKey[kMaxWiFiKeyLength];     /**< The WiFi key (NOT NULL-terminated). */
    uint8_t mWiFiKeyLen;                     /**< The length in bytes of the WiFi key. */
};

} // namespace DeviceLayer
} // namespace chip

#endif // DEVICE_NETWORK_INFO_H
