/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

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
constexpr size_t kWiFiBSSIDLength   = 6;

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
enum WiFiAuthSecurityType : int8_t
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

    struct
    {
        bool NetworkId : 1;           /**< True if the NetworkId field is present. */
        bool ThreadExtendedPANId : 1; /**< True if the ThreadExtendedPANId field is present. */
        bool ThreadMeshPrefix : 1;    /**< True if the ThreadMeshPrefix field is present. */
        bool ThreadPSKc : 1;          /**< True if the ThreadPSKc field is present. */
    } FieldPresent;

    // ---- WiFi-specific Fields ----
    char WiFiSSID[kMaxWiFiSSIDLength + 1]; /**< The WiFi SSID as a NULL-terminated string. */
    uint8_t WiFiKey[kMaxWiFiKeyLength];    /**< The WiFi key (NOT NULL-terminated). */
    uint8_t WiFiKeyLen;                    /**< The length in bytes of the WiFi key. */
    WiFiAuthSecurityType WiFiSecurityType; /**< The WiFi security type. */
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
