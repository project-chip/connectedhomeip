/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "WifiConnect.h"
#include <lib/support/Span.h>
#include <platform/ConnectivityManager.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace NXP {
namespace App {

CHIP_ERROR WifiConnectAtboot(chip::DeviceLayer::NetworkCommissioning::WiFiDriver * wifiDriver)
{
    VerifyOrReturnError(wifiDriver != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    /* In case WiFi connect at boot is enabled try to set SSID to the predefined value */
    ByteSpan ssidSpan     = ByteSpan(Uint8::from_const_char(CONFIG_CHIP_APP_WIFI_SSID), strlen(CONFIG_CHIP_APP_WIFI_SSID));
    ByteSpan passwordSpan = ByteSpan(Uint8::from_const_char(CONFIG_CHIP_APP_WIFI_PASSWORD), strlen(CONFIG_CHIP_APP_WIFI_PASSWORD));
    VerifyOrReturnError(IsSpanUsable(ssidSpan) && IsSpanUsable(passwordSpan), CHIP_ERROR_INVALID_ARGUMENT);

    chip::DeviceLayer::NetworkCommissioning::NetworkIterator * networks = wifiDriver->GetNetworks();
    /* In case Wi-Fi driver has already Wi-Fi network information, skip the connect stage */
    if (networks == nullptr || networks->Count() == 0)
    {
        uint8_t networkIndex;
        chip::MutableCharSpan debugText;
        chip::DeviceLayer::NetworkCommissioning::Status status =
            wifiDriver->AddOrUpdateNetwork(ssidSpan, passwordSpan, debugText, networkIndex);
        VerifyOrReturnError(status == chip::DeviceLayer::NetworkCommissioning::Status::kSuccess, CHIP_ERROR_CONNECTION_ABORTED);
        wifiDriver->ConnectNetwork(ssidSpan, nullptr);
    }
    return CHIP_NO_ERROR;
}

} // namespace App
} // namespace NXP
} // namespace chip
