/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <WhmDelegate.h>
#include <WhmInstance.h>

using chip::Protocols::InteractionModel::Status;
namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

/**
 * The WhmManufacturer example class
 */

class WhmManufacturer
{
public:
    WhmManufacturer(WaterHeaterManagementInstance * whmInstance)
    {
        mWhmInstance = whmInstance;
    }

    WaterHeaterManagementInstance * GetWhmInstance() { return mWhmInstance; }

    WaterHeaterManagementDelegate * GetWhmDelegate()
    {
        if (mWhmInstance)
        {
            return mWhmInstance->GetDelegate();
        }
        return nullptr;
    }

    /**
     * @brief   Called at start up to apply hardware settings
     */
    CHIP_ERROR Init();

    /**
     * @brief   Called at shutdown
     */
    CHIP_ERROR Shutdown();

private:
    WaterHeaterManagementInstance * mWhmInstance;
};

/** @brief Helper function to return the singleton WhmManufacturer instance
 *
 * This is needed by the WhmManufacturer class to support TestEventTriggers
 * which are called outside of any class context. This allows the WhmManufacturer
 * class to return the relevant Delegate instance in which to invoke the test
 * events on.
 *
 * This function is typically found in main.cpp or wherever the singleton is created.
 */
WhmManufacturer * GetWhmManufacturer();

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
