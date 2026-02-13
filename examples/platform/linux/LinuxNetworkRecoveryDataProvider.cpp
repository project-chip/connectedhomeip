/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "LinuxNetworkRecoveryDataProvider.h"
#include <app/clusters/general-commissioning-server/CodegenIntegration.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip::app::Clusters::GeneralCommissioning;
CHIP_ERROR LinuxNetworkRecoveryDataProvider::GetNetworkRecoveryIdentifier(uint64_t & recoveryIdentifier)
{
#if !CHIP_DEVICE_CONFIG_ENABLE_NETWORK_RECOVERY
    return CHIP_ERROR_INCORRECT_STATE;
#else  // CHIP_DEVICE_CONFIG_ENABLE_NETWORK_RECOVERY
    recoveryIdentifier = Instance()->GetRecoveryIdentifier();
    return CHIP_NO_ERROR;
#endif // CHIP_DEVICE_CONFIG_ENABLE_NETWORK_RECOVERY
}
CHIP_ERROR LinuxNetworkRecoveryDataProvider::GetNetworkRecoveryReason(uint8_t & reason)
{
#if !CHIP_DEVICE_CONFIG_ENABLE_NETWORK_RECOVERY
    return CHIP_ERROR_INCORRECT_STATE;
#else
    Attributes::NetworkRecoveryReason::TypeInfo::Type value;
    Instance()->GetNetworkRecoveryReasonValue(value);
    if (value.IsNull())
        return CHIP_ERROR_INCORRECT_STATE;
    reason = static_cast<uint8_t>(value.Value());
    return CHIP_NO_ERROR;
#endif // CHIP_DEVICE_CONFIG_ENABLE_NETWORK_RECOVERY
}
