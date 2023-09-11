/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    virtual ~ExternalFlashManager() {}

    virtual void DoAction(Action aAction)
    {
#if CONFIG_PM_DEVICE && CONFIG_NORDIC_QSPI_NOR
        // utilize the QSPI driver sleep power mode
        const auto * qspi_dev = DEVICE_DT_GET(DT_INST(0, nordic_qspi_nor));
        if (device_is_ready(qspi_dev))
        {
            const auto requestedAction = Action::WAKE_UP == aAction ? PM_DEVICE_ACTION_RESUME : PM_DEVICE_ACTION_SUSPEND;
            (void) pm_device_action_run(qspi_dev, requestedAction); // not much can be done in case of a failure
        }
#endif
    }
};

} // namespace DeviceLayer
} // namespace chip
