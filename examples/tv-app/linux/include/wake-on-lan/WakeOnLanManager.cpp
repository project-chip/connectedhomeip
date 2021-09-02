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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>
#include <lib/support/ZclString.h>

#include <inipp/inipp.h>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace chip;

CHIP_ERROR WakeOnLanManager::Init()
{
    CHIP_ERROR err                                       = CHIP_NO_ERROR;
    EndpointConfigurationStorage & endpointConfiguration = EndpointConfigurationStorage::GetInstance();
    err                                                  = endpointConfiguration.Init();
    SuccessOrExit(err);
    es = &endpointConfiguration;
exit:
    return err;
}

void WakeOnLanManager::store(chip::EndpointId endpoint, char macAddress[32])
{
    uint8_t bufferMemory[32];
    MutableByteSpan zclString(bufferMemory);
    MakeZclCharString(zclString, macAddress);
    EmberAfStatus macAddressStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_WAKE_ON_LAN_CLUSTER_ID, ZCL_WAKE_ON_LAN_MAC_ADDRESS_ATTRIBUTE_ID,
                                    zclString.data(), ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    if (macAddressStatus != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to store mac address attribute.");
    }
}

void WakeOnLanManager::setMacAddress(chip::EndpointId endpoint, char * macAddress)
{
    char address[18];
    uint16_t size = static_cast<uint16_t>(sizeof(address));

    std::string section = "endpoint" + std::to_string(endpoint);
    CHIP_ERROR err      = es->get(section, "macAddress", macAddress, size);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to get mac address. Error:%s", chip::ErrorStr(err));
    }
}
