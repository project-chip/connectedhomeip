/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019-2020 Google LLC.
 * SPDX-FileCopyrightText: 2013-2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
