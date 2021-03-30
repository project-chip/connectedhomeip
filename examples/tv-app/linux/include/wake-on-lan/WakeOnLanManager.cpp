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

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <app/util/af.h>
#include <app/util/basic-types.h>

#include <inipp/inipp.h>

#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

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

void WakeOnLanManager::store(chip::EndpointId endpoint, char macAddress[17])
{
    EmberAfStatus macAddressStatus =
        emberAfWriteServerAttribute(endpoint, ZCL_WAKE_ON_LAN_CLUSTER_ID, ZCL_WAKE_ON_LAN_MAC_ADDRESS_ATTRIBUTE_ID,
                                    (uint8_t *) &macAddress, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
    assert(macAddressStatus == EMBER_ZCL_STATUS_SUCCESS);
}

void WakeOnLanManager::setMacAddress(chip::EndpointId endpoint, char * macAddress)
{
    char address[17];
    uint16_t size = static_cast<uint16_t>(sizeof(address));

    string section = "endpoint" + std::to_string(endpoint);
    CHIP_ERROR err = es->get(section, "macAddress", macAddress, size);
    if (err != CHIP_NO_ERROR)
    {
        emberAfWakeOnLanClusterPrintln("Failed to get app catalog mac address. ERR:%s", chip::ErrorStr(err));
    }
}