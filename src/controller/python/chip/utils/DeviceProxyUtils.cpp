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
#include <controller/python/chip/native/PyChipError.h>
#include <stdio.h>
#include <system/SystemError.h>

using namespace chip;

namespace python {

struct __attribute__((packed)) SessionParametersStruct
{
    uint32_t sessionIdleInterval      = 0;
    uint32_t sessionActiveInterval    = 0;
    uint16_t sessionActiveThreshold   = 0;
    uint16_t dataModelRevision        = 0;
    uint16_t interactionModelRevision = 0;
    uint32_t specificationVersion     = 0;
    uint16_t maxPathsPerInvoke        = 0;
};

} // namespace python

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

/**
 * @brief This gets the Session Parameters reported by remote node.
 *
 * A valid DeviceProxy pointer with a valid established session is required for this method.
 */
PyChipError pychip_DeviceProxy_GetRemoteSessionParameters(DeviceProxy * device, void * sessionParametersStructPointer)
{
    VerifyOrReturnError(device != nullptr || sessionParametersStructPointer, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    auto * deviceProxy = static_cast<DeviceProxy *>(device);
    VerifyOrReturnError(deviceProxy->GetSecureSession().HasValue(), ToPyChipError(CHIP_ERROR_INCORRECT_STATE));

    auto remoteSessionParameters = deviceProxy->GetSecureSession().Value()->GetRemoteSessionParameters();
    auto remoteMrpConfig         = remoteSessionParameters.GetMRPConfig();

    python::SessionParametersStruct * sessionParam = static_cast<python::SessionParametersStruct *>(sessionParametersStructPointer);

    sessionParam->sessionIdleInterval      = remoteMrpConfig.mIdleRetransTimeout.count();
    sessionParam->sessionActiveInterval    = remoteMrpConfig.mActiveRetransTimeout.count();
    sessionParam->sessionActiveThreshold   = remoteMrpConfig.mActiveThresholdTime.count();
    sessionParam->dataModelRevision        = remoteSessionParameters.GetDataModelRevision().ValueOr(0);
    sessionParam->interactionModelRevision = remoteSessionParameters.GetInteractionModelRevision().ValueOr(0);
    sessionParam->specificationVersion     = remoteSessionParameters.GetSpecificationVersion().ValueOr(0);
    sessionParam->maxPathsPerInvoke        = remoteSessionParameters.GetMaxPathsPerInvoke();
    return ToPyChipError(CHIP_NO_ERROR);
}
}
