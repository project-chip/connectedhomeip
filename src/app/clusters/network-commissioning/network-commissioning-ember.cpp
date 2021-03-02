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

#include <gen/att-storage.h>
#include <gen/attribute-id.h>
#include <gen/attribute-type.h>
#include <gen/callback.h>
#include <gen/cluster-id.h>
#include <gen/command-id.h>
#include <gen/enums.h>
#include <util/af.h>

namespace {
// TODO: The network commissioning clusters uses lots of OCTET_STRING type arguments, we need to fix this by some byte buffer
// containers.
size_t ConvertStringToNullTerminated(uint8_t * str)
{
    uint8_t len = str[0];
    memmove(str, str + 1, len);
    str[len] = 0;
    return len;
}
} // namespace

bool emberAfNetworkCommissioningClusterAddThreadNetworkCallback(uint8_t * operationalDataset, uint64_t breadcrumb,
                                                                uint32_t timeoutMs)
{
    ConvertStringToNullTerminated(operationalDataset);

    EmberAfNetworkCommissioningError err = chip::app::clusters::NetworkCommissioning::OnAddThreadNetworkCommandCallbackInternal(
        nullptr, emberAfCurrentEndpoint(), operationalDataset, breadcrumb, timeoutMs);
    emberAfSendImmediateDefaultResponse(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS ? EMBER_ZCL_STATUS_SUCCESS
                                                                                             : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfNetworkCommissioningClusterAddWiFiNetworkCallback(uint8_t * ssid, uint8_t * credentials, uint64_t breadcrumb,
                                                              uint32_t timeoutMs)
{
    ConvertStringToNullTerminated(ssid);
    ConvertStringToNullTerminated(credentials);

    EmberAfNetworkCommissioningError err = chip::app::clusters::NetworkCommissioning::OnAddWiFiNetworkCommandCallbackInternal(
        nullptr, emberAfCurrentEndpoint(), ssid, credentials, breadcrumb, timeoutMs);
    emberAfSendImmediateDefaultResponse(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS ? EMBER_ZCL_STATUS_SUCCESS
                                                                                             : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfNetworkCommissioningClusterEnableNetworkCallback(uint8_t * networkID, uint64_t breadcrumb, uint32_t timeoutMs)
{
    ConvertStringToNullTerminated(networkID);

    EmberAfNetworkCommissioningError err = chip::app::clusters::NetworkCommissioning::OnEnableNetworkCommandCallbackInternal(
        nullptr, emberAfCurrentEndpoint(), networkID, breadcrumb, timeoutMs);
    emberAfSendImmediateDefaultResponse(err == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS ? EMBER_ZCL_STATUS_SUCCESS
                                                                                             : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

// TODO: The following commands needed to be implemented.
// These commands are not implemented thus not handled yet, return false so ember will return a error.

bool emberAfNetworkCommissioningClusterDisableNetworkCallback(uint8_t * networkID, uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterGetLastNetworkCommissioningResultCallback(uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterRemoveNetworkCallback(uint8_t * NetworkID, uint64_t Breadcrumb, uint32_t TimeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterScanNetworksCallback(uint8_t * ssid, uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}
bool emberAfNetworkCommissioningClusterUpdateThreadNetworkCallback(uint8_t * operationalDataset, uint64_t breadcrumb,
                                                                   uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterUpdateWiFiNetworkCallback(uint8_t * ssid, uint8_t * credentials, uint64_t breadcrumb,
                                                                 uint32_t timeoutMs)
{
    return false;
}
