/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "AndroidDeviceControllerWrapper.h"

#include <memory>

using chip::DeviceController::ChipDeviceController;

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    mController->Shutdown();
}

AndroidDeviceControllerWrapper * AndroidDeviceControllerWrapper::AllocateNew(chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                                             chip::Inet::InetLayer * inetLayer,
                                                                             CHIP_ERROR * errInfoOnFailure)
{
    if (errInfoOnFailure == nullptr)
    {
        ChipLogError(Controller, "Missing error info");
        return nullptr;
    }
    if (systemLayer == nullptr)
    {
        ChipLogError(Controller, "Missing system layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }
    if (inetLayer == nullptr)
    {
        ChipLogError(Controller, "Missing inet layer");
        *errInfoOnFailure = CHIP_ERROR_INVALID_ARGUMENT;
        return nullptr;
    }

    *errInfoOnFailure = CHIP_NO_ERROR;

    std::unique_ptr<ChipDeviceController> controller(new ChipDeviceController());
    std::unique_ptr<AndroidDevicePairingDelegate> delegate(new AndroidDevicePairingDelegate());

    if (!controller || !delegate)
    {
        *errInfoOnFailure = CHIP_ERROR_NO_MEMORY;
        return nullptr;
    }

    *errInfoOnFailure = controller->Init(nodeId, systemLayer, inetLayer, delegate.get());
    if (*errInfoOnFailure != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return new AndroidDeviceControllerWrapper(std::move(controller), std::move(delegate));
}
