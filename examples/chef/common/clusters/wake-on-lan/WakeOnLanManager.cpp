/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/config.h>
#ifdef MATTER_DM_PLUGIN_WAKE_ON_LAN_SERVER
#include "WakeOnLanManager.h"
#include <fstream>
#include <iostream>
#include <lib/support/BytesToHex.h>
#include <platform/ConfigurationManager.h>
#include <string>

constexpr char kNullHexMACAddress[] = "000000000000";

using namespace chip;
using namespace chip::app::Clusters::WakeOnLan;

std::string getMacAddress()
{
    uint8_t macBuffer[chip::DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength];
    MutableByteSpan mac(macBuffer);
    if (chip::DeviceLayer::ConfigurationMgr().GetPrimaryMACAddress(mac) != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "WakeOnLanManager::getMacAddress no primary MAC configured by DeviceLayer");
        return kNullHexMACAddress;
    }

    char macStr[chip::DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength * 2 + 1] = { 0 }; // added null char
    if (BytesToHex(mac.data(), mac.size(), &macStr[0], sizeof(macBuffer) * 2u, chip::Encoding::HexFlags::kUppercase) !=
        CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "WakeOnLanManager::getMacAddress hex conversion failed");
        return kNullHexMACAddress;
    }

    return std::string(macStr);
}

CHIP_ERROR WakeOnLanManager::HandleGetMacAddress(chip::app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "WakeOnLanManager::HandleGetMacAddress");

    // Spec REQUIRES 48-bit mac addresses. This means at least Thread devices will
    // fail here.
    if (chip::DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength != 6)
    {
        ChipLogError(Zcl, "WakeOnLanManager: primary MAC address is not 48-bit");
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return aEncoder.Encode(CharSpan::fromCharString(getMacAddress().c_str()));
}
#endif // MATTER_DM_PLUGIN_WAKE_ON_LAN_SERVER
