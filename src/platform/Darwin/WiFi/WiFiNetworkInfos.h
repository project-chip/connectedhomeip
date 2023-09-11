/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace DeviceLayer {

struct WiFiNetworkInfos
{
    uint8_t ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    size_t ssidLen;
    uint8_t credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
    size_t credentialsLen;

    WiFiNetworkInfos() { Reset(); }

    void Reset()
    {
        memset(ssid, 0, sizeof(ssid));
        ssidLen = 0;
        memset(credentials, 0, sizeof(credentials));
        credentialsLen = 0;
    }

    bool Match(const WiFiNetworkInfos & infos) { return ssidLen == infos.ssidLen && memcmp(ssid, infos.ssid, ssidLen) == 0; }

    bool Match(const ByteSpan & infos) { return ssidLen == infos.size() && memcmp(ssid, infos.data(), ssidLen) == 0; }

    CHIP_ERROR Update(const ByteSpan & anOtherSSID, const ByteSpan & anOtherCredentials)
    {
        VerifyOrReturnError(anOtherSSID.size() <= sizeof(ssid), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(anOtherCredentials.size() <= sizeof(credentials), CHIP_ERROR_INVALID_ARGUMENT);

        memcpy(credentials, anOtherCredentials.data(), anOtherCredentials.size());
        credentialsLen = anOtherCredentials.size();
        memcpy(ssid, anOtherSSID.data(), anOtherSSID.size());
        ssidLen = anOtherSSID.size();
        return CHIP_NO_ERROR;
    }
};

} // namespace DeviceLayer
} // namespace chip
