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

#ifndef DEVICE_NETWORK_INFO_H
#define DEVICE_NETWORK_INFO_H

#include <stddef.h>
#include <stdint.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Constants for common network metadata entries
 */
// ---- WiFi-specific Limits ----
constexpr size_t kMaxWiFiSSIDLength = 32;
constexpr size_t kMaxWiFiKeyLength  = 64;

// ---- Thread-specific Limits ----
constexpr size_t kMaxThreadNetworkNameLength = 16;
constexpr size_t kThreadExtendedPANIdLength  = 8;
constexpr size_t kThreadMeshPrefixLength     = 8;
constexpr size_t kThreadNetworkKeyLength     = 16;
constexpr size_t kThreadPSKcLength           = 16;
constexpr size_t kThreadChannel_NotSpecified = UINT8_MAX;
constexpr size_t kThreadPANId_NotSpecified   = UINT16_MAX;

/**
 * Ids for well-known network provision types.
 */
enum
{
    kThreadNetworkId      = 1,
    kWiFiStationNetworkId = 2,
};

/**
 * WiFi Security Modes.
 */
enum WiFiAuthSecurityType
{
    kWiFiSecurityType_NotSpecified = -1,

    kWiFiSecurityType_None                = 1,
    kWiFiSecurityType_WEP                 = 2,
    kWiFiSecurityType_WPAPersonal         = 3,
    kWiFiSecurityType_WPA2Personal        = 4,
    kWiFiSecurityType_WPA2MixedPersonal   = 5,
    kWiFiSecurityType_WPAEnterprise       = 6,
    kWiFiSecurityType_WPA2Enterprise      = 7,
    kWiFiSecurityType_WPA2MixedEnterprise = 8,
    kWiFiSecurityType_WPA3Personal        = 9,
    kWiFiSecurityType_WPA3MixedPersonal   = 10,
    kWiFiSecurityType_WPA3Enterprise      = 11,
    kWiFiSecurityType_WPA3MixedEnterprise = 12,
};

class DeviceNetworkInfo
{
public:
    uint32_t NetworkId; /**< The network id assigned to the network by the device. */

    // ---- WiFi-specific Fields ----
    char WiFiSSID[kMaxWiFiSSIDLength + 1]; /**< The WiFi SSID as a NULL-terminated string. */
    uint8_t WiFiKey[kMaxWiFiKeyLength];    /**< The WiFi key (NOT NULL-terminated). */
    uint8_t WiFiKeyLen;                    /**< The length in bytes of the WiFi key. */
    WiFiAuthSecurityType WiFiSecurityType; /**< The WiFi security type. */

    // ---- Thread-specific Fields ----
    char ThreadNetworkName[kMaxThreadNetworkNameLength + 1];
    /**< The Thread network name as a NULL-terminated string. */
    uint8_t ThreadExtendedPANId[kThreadExtendedPANIdLength];
    /**< The Thread extended PAN ID. */
    uint8_t ThreadMeshPrefix[kThreadMeshPrefixLength];
    /**< The Thread mesh prefix. */
    uint8_t ThreadNetworkKey[kThreadNetworkKeyLength];
    /**< The Thread master network key (NOT NULL-terminated). */
    uint8_t ThreadPSKc[kThreadPSKcLength];
    /**< The Thread pre-shared commissioner key (NOT NULL-terminated). */
    uint16_t ThreadPANId;  /**< The 16-bit Thread PAN ID, or kThreadPANId_NotSpecified */
    uint8_t ThreadChannel; /**< The Thread channel (currently [11..26]), or kThreadChannel_NotSpecified */

    struct
    {
        bool NetworkId : 1;           /**< True if the NetworkId field is present. */
        bool ThreadExtendedPANId : 1; /**< True if the ThreadExtendedPANId field is present. */
        bool ThreadMeshPrefix : 1;    /**< True if the ThreadMeshPrefix field is present. */
        bool ThreadPSKc : 1;          /**< True if the ThreadPSKc field is present. */
    } FieldPresent;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // DEVICE_NETWORK_INFO_H
