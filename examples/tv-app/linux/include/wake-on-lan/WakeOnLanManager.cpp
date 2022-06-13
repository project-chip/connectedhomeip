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
