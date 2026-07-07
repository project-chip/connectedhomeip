/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <devices/interface/SingleEndpointDevice.h>

using namespace chip::app::Clusters;

namespace chip::app {

CHIP_ERROR SingleEndpointDevice::RegisterDescriptor(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                    EndpointComposition composition)
{
    VerifyOrReturnError(endpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    mEndpointId    = endpoint;
    CHIP_ERROR err = DeviceInterface::RegisterDescriptor(endpoint, provider, composition);
    if (err != CHIP_NO_ERROR)
    {
        mEndpointId = kInvalidEndpointId;
    }
    return err;
}

void SingleEndpointDevice::UnregisterDescriptor(CodeDrivenDataModelProvider & provider)
{
    DeviceInterface::UnregisterDescriptor(mEndpointId, provider);
    mEndpointId = kInvalidEndpointId;
}

} // namespace chip::app
