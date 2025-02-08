/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <zephyr/device.h>
#include <zephyr/pm/device.h>

namespace chip {
namespace DeviceLayer {

class ExternalFlashManager
{
public:
    enum class Action : uint8_t
    {
        WAKE_UP,
        SLEEP
    };

    // Not copyable or movable
    ExternalFlashManager(ExternalFlashManager &&)                  = delete;
    ExternalFlashManager & operator=(const ExternalFlashManager &) = delete;
    ExternalFlashManager(const ExternalFlashManager &)             = delete;
    ExternalFlashManager & operator=(ExternalFlashManager &&)      = delete;

    static ExternalFlashManager & GetInstance()
    {
        static ExternalFlashManager sExternalFlashManager;
        return sExternalFlashManager;
    }

    void DoAction(Action aAction)
    {
#if defined(CONFIG_PM_DEVICE) && defined(CONFIG_NORDIC_QSPI_NOR)
        // utilize the QSPI driver sleep power mode
        const auto * qspi_dev = DEVICE_DT_GET(DT_INST(0, nordic_qspi_nor));
        if (device_is_ready(qspi_dev))
        {
            const auto requestedAction = Action::WAKE_UP == aAction ? PM_DEVICE_ACTION_RESUME : PM_DEVICE_ACTION_SUSPEND;
            (void) pm_device_action_run(qspi_dev, requestedAction); // not much can be done in case of a failure
        }
#endif
    }

private:
    // Singleton Object
    ExternalFlashManager() = default;
};

} // namespace DeviceLayer
} // namespace chip
