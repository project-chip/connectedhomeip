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

#include "network-commissioning.h"

#include <cstring>

#include <app/CommandHandler.h>
#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/callback.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>

using namespace chip;

bool emberAfNetworkCommissioningClusterAddThreadNetworkCallback(chip::app::CommandHandler * commandObj, ByteSpan operationalDataset,
                                                                uint64_t breadcrumb, uint32_t timeoutMs)
{
    EmberAfNetworkCommissioningError err = chip::app::clusters::NetworkCommissioning::OnAddThreadNetworkCommandCallbackInternal(
        nullptr, emberAfCurrentEndpoint(), operationalDataset, breadcrumb, timeoutMs);
    emberAfSendImmediateDefaultResponse(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS ? EMBER_ZCL_STATUS_SUCCESS
                                                                                             : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfNetworkCommissioningClusterAddWiFiNetworkCallback(chip::app::CommandHandler * commandObj, ByteSpan ssid,
                                                              ByteSpan credentials, uint64_t breadcrumb, uint32_t timeoutMs)
{
    EmberAfNetworkCommissioningError err = chip::app::clusters::NetworkCommissioning::OnAddWiFiNetworkCommandCallbackInternal(
        nullptr, emberAfCurrentEndpoint(), ssid, credentials, breadcrumb, timeoutMs);
    emberAfSendImmediateDefaultResponse(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS ? EMBER_ZCL_STATUS_SUCCESS
                                                                                             : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfNetworkCommissioningClusterEnableNetworkCallback(chip::app::CommandHandler * commandObj, ByteSpan networkID,
                                                             uint64_t breadcrumb, uint32_t timeoutMs)
{
    EmberAfNetworkCommissioningError err = chip::app::clusters::NetworkCommissioning::OnEnableNetworkCommandCallbackInternal(
        nullptr, emberAfCurrentEndpoint(), networkID, breadcrumb, timeoutMs);
    emberAfSendImmediateDefaultResponse(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS ? EMBER_ZCL_STATUS_SUCCESS
                                                                                             : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

// TODO: The following commands needed to be implemented.
// These commands are not implemented thus not handled yet, return false so ember will return a error.

bool emberAfNetworkCommissioningClusterDisableNetworkCallback(chip::app::CommandHandler * commandObj, ByteSpan networkID,
                                                              uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterGetLastNetworkCommissioningResultCallback(chip::app::CommandHandler * commandObj,
                                                                                 uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterRemoveNetworkCallback(chip::app::CommandHandler * commandObj, ByteSpan NetworkID,
                                                             uint64_t Breadcrumb, uint32_t TimeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterScanNetworksCallback(chip::app::CommandHandler * commandObj, ByteSpan ssid,
                                                            uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}
bool emberAfNetworkCommissioningClusterUpdateThreadNetworkCallback(chip::app::CommandHandler * commandObj,
                                                                   ByteSpan operationalDataset, uint64_t breadcrumb,
                                                                   uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterUpdateWiFiNetworkCallback(chip::app::CommandHandler * commandObj, ByteSpan ssid,
                                                                 ByteSpan credentials, uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}
