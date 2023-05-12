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

#include <platform/ICDEventManager.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief Initialisation function of the ICDEventManager.
 *        Init function MUST be called before using the object
 * 
 * @param icdManager pointer to the ICDManager object
 * @return CHIP_ERROR 
 */
CHIP_ERROR ICDEventManager::Init(ICDManager * icdManager)
{
    VerifyOrReturnError(icdManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mICDManager = icdManager;

    // OTA Requestor initialization will be triggered by the connectivity events
    PlatformMgr().AddEventHandler(ICDEventHandler, reinterpret_cast<intptr_t>(nullptr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDEventManager::Shutdown()
{
    return CHIP_NO_ERROR;
}

/**
 * @brief Event Handler callback given to the PlatformManager
 *        Function dispatchs the event to the ICDManager member
 * 
 * @param event 
 * @param arg 
 */
void ICDEventManager::ICDEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    // TODO
}

} // namespace DeviceLaye
} // namespace chip
