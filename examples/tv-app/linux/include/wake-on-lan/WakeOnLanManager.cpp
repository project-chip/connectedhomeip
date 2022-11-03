/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "WakeOnLanManager.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace chip;
using namespace chip::app::Clusters::WakeOnLan;

std::string getMacAddress()
{
    std::ifstream input("/sys/class/net/eth0/address");
    std::string line;
    std::getline(input, line);
    return line;
}

CHIP_ERROR WakeOnLanManager::HandleGetMacAddress(chip::app::AttributeValueEncoder & aEncoder)
{
#if CHIP_ENABLE_WAKE_ON_LAN
    return aEncoder.Encode(CharSpan::fromCharString(getMacAddress().c_str()));
#else
    return aEncoder.Encode(CharSpan::fromCharString("00:00:00:00:00"));
#endif // CHIP_ENABLE_WAKE_ON_LAN
}
