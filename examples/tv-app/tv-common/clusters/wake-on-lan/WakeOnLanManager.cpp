/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "WakeOnLanManager.h"
#include <fstream>
#include <iostream>
#include <lib/support/BytesToHex.h>
#include <platform/ConfigurationManager.h>
#include <string>

using namespace chip;
using namespace chip::app::Clusters::WakeOnLan;

std::string getMacAddress()
{
    uint8_t macBuffer[chip::DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength];
    MutableByteSpan mac(macBuffer);
    if (chip::DeviceLayer::ConfigurationMgr().GetPrimaryMACAddress(mac) != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "WakeOnLanManager::getMacAddress no primary MAC configured by DeviceLayer");
        return "0000000000";
    }

    char macStr[chip::DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength * 2 + 1] = { 0 }; // added null char
    if (BytesToHex(&macBuffer[0], sizeof(macBuffer), &macStr[0], sizeof(macBuffer) * 2u, chip::Encoding::HexFlags::kUppercase) !=
        CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "WakeOnLanManager::getMacAddress hex conversion failed");
        return "0000000000";
    }

    return std::string(macStr);
}

CHIP_ERROR WakeOnLanManager::HandleGetMacAddress(chip::app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "WakeOnLanManager::HandleGetMacAddress");

    return aEncoder.Encode(CharSpan::fromCharString(getMacAddress().c_str()));
}
