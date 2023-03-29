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

#include "WakeOnLanManager.h"
#include <fstream>
#include <iostream>
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

    uint8_t j = 0;
    for (uint8_t i = 0; i < sizeof(macBuffer); i++)
    {
        j         = i * 2;
        macStr[j] = (((macBuffer[i] & 0xF0) >> 4) & 0xF);
        macStr[j] += (macStr[j] <= 9) ? '0' : ('A' - 10);
        j++;
        macStr[j] = (macBuffer[i] & 0x0F);
        macStr[j] += (macStr[j] <= 9) ? '0' : ('A' - 10);
    }

    return std::string(macStr);
}

CHIP_ERROR WakeOnLanManager::HandleGetMacAddress(chip::app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "WakeOnLanManager::HandleGetMacAddress");

    return aEncoder.Encode(CharSpan::fromCharString(getMacAddress().c_str()));
}
