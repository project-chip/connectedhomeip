/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppEvent.h"
#include "lega_at_api.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/CHIPError.h>
#include <stdbool.h>
#include <stdint.h>

class SensorManager
{
public:
    CHIP_ERROR Init();

private:
    friend SensorManager & SensorMgr();

    // Reads new generated sensor value, stores it, and updates local temperature attribute
    static void TimerEventHandler(lega_timer_t * timer);
    static void SensorTimerEventHandler(AppEvent * aEvent);

    static SensorManager sSensorManager;
};

inline SensorManager & SensorMgr()
{
    return SensorManager::sSensorManager;
}
