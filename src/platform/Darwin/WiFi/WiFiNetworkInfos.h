/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
