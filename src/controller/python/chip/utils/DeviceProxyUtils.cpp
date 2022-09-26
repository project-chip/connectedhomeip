/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      Implementation of the native methods expected by the Python
 *      version of Chip Device Manager.
 *
 */

#include "system/SystemClock.h"
#include <app/DeviceProxy.h>
#include <stdio.h>
#include <system/SystemError.h>

using namespace chip;

static_assert(std::is_same<uint32_t, ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

extern "C" {

/**
 * @brief
 *
 * This computes the value for a timeout based on the round trip time it takes for a message to be sent to a peer,
 * the message to be processed given the upperLayerProcessingTimeoutMs argument, and a response to come back.
 *
 * See Session::ComputeRoundTripTimeout for more specific details.
 *
 * A valid DeviceProxy pointer with a valid, established session is required for this method.
 *
 *
 */
uint32_t pychip_DeviceProxy_ComputeRoundTripTimeout(DeviceProxy * device, uint32_t upperLayerProcessingTimeoutMs)
{
    VerifyOrDie(device != nullptr);

    auto * deviceProxy = static_cast<DeviceProxy *>(device);
    VerifyOrDie(deviceProxy->GetSecureSession().HasValue());

    return deviceProxy->GetSecureSession()
        .Value()
        ->ComputeRoundTripTimeout(System::Clock::Milliseconds32(upperLayerProcessingTimeoutMs))
        .count();
}
}
