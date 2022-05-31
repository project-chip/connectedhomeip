/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *    @file
 *          Provides the implementation of the Device Layer Network Provisioining Delegate class
 *          for Open IOT SDK platform.
 */

#include <lib/support/logging/CHIPLogging.h>
#include <platform/openiotsdk/DeviceNetworkProvisioningDelegateImpl.h>
#include <platform/openiotsdk/NetworkCommissioningDriver.h>

using namespace ::chip::DeviceLayer::NetworkCommissioning;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionThreadNetwork(ByteSpan threadData)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace chip
