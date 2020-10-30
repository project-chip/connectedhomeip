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
#include <AndroidDeviceControllerWrapper.h>

#include <memory>

using chip::DeviceController::ChipDeviceController;

AndroidDeviceControllerWrapper::~AndroidDeviceControllerWrapper()
{
    mController->Shutdown();

    delete mController;
    delete mDelegate;
}

static AndroidDeviceControllerWrapper * allocateNew(chip::NodeId nodeId, chip::System::Layer * systemLayer,
                                                    chip::Inet::InetLayer * inetLayer, CHIP_ERROR * errInfoOnFailure)
{
    errInfoOnFailure = CHIP_NO_ERROR;

    std::unique_ptr<ChipDeviceController> controller       = std::make_uniqe<ChipDeviceController>();
    std::unique_ptr<AndroidDevicePairingDelegate> delegate = std::make_uniqe<AndroidDevicePairingDelegate>();

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

    return new AndroidDeviceControllerWrapper(controller, delegate);
}
