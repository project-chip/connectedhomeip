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

/**
 * @file  Wrappers for namespaced network commissioning cluster implementation.
 */

#include "network-commissioning.h"

#include <cstring>

#include <app/Command.h>
#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>
#include <gen/callback.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;

bool emberAfNetworkCommissioningClusterAddThreadNetworkCallback(chip::app::Command * commandObj, ByteSpan operationalDataset,
                                                                uint64_t breadcrumb, uint32_t timeoutMs)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    chip::app::clusters::NetworkCommissioning::OnAddThreadNetworkCommandCallbackInternal(commandObj, emberAfCurrentEndpoint(),
                                                                                         operationalDataset, breadcrumb, timeoutMs);
    return true;
#else
    return false;
#endif
}

bool emberAfNetworkCommissioningClusterAddWiFiNetworkCallback(chip::app::Command * commandObj, ByteSpan ssid, ByteSpan credentials,
                                                              uint64_t breadcrumb, uint32_t timeoutMs)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    chip::app::clusters::NetworkCommissioning::OnAddWiFiNetworkCommandCallbackInternal(commandObj, emberAfCurrentEndpoint(), ssid,
                                                                                       credentials, breadcrumb, timeoutMs);
    return true;
#else
    return false;
#endif
}

bool emberAfNetworkCommissioningClusterEnableNetworkCallback(chip::app::Command * commandObj, ByteSpan networkID,
                                                             uint64_t breadcrumb, uint32_t timeoutMs)
{
    chip::app::clusters::NetworkCommissioning::OnEnableNetworkCommandCallbackInternal(commandObj, emberAfCurrentEndpoint(),
                                                                                      networkID, breadcrumb, timeoutMs);
    return true;
}

// TODO: The following commands needed to be implemented.
// These commands are not implemented thus not handled yet, return false so ember will return a error.

bool emberAfNetworkCommissioningClusterDisableNetworkCallback(chip::app::Command * commandObj, ByteSpan networkID,
                                                              uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterGetLastNetworkCommissioningResultCallback(chip::app::Command * commandObj,
                                                                                 uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterRemoveNetworkCallback(chip::app::Command * commandObj, ByteSpan NetworkID,
                                                             uint64_t Breadcrumb, uint32_t TimeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterScanNetworksCallback(chip::app::Command * commandObj, ByteSpan ssid, uint64_t breadcrumb,
                                                            uint32_t timeoutMs)
{
    return false;
}
bool emberAfNetworkCommissioningClusterUpdateThreadNetworkCallback(chip::app::Command * commandObj, ByteSpan operationalDataset,
                                                                   uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}

bool emberAfNetworkCommissioningClusterUpdateWiFiNetworkCallback(chip::app::Command * commandObj, ByteSpan ssid,
                                                                 ByteSpan credentials, uint64_t breadcrumb, uint32_t timeoutMs)
{
    return false;
}
