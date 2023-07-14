/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#pragma once
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app/icd/IcdManager.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace app {

/**
 * @brief IcdEventManager class is responsible of processing Platform Events that affect an ICD's behaviour
 * The class registers an Event Handler with the Platform Manager and dispatches the processing to the IcdManager class.
 */
class IcdEventManager
{

public:
    IcdEventManager() = default;

    /**
     * @brief Initialisation function of the IcdEventManager.
     *        Init function MUST be called before using the object
     */
    CHIP_ERROR Init(IcdManager * icdManager);
    CHIP_ERROR Shutdown();

private:
    /**
     * @brief Event Handler callback given to the PlatformManager
     *        Function dispatchs the event to the IcdManager member
     */
    static void ICDEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static uint8_t expectedMsgCount;
    IcdManager * mIcdManager;
};

} // namespace app
} // namespace chip
