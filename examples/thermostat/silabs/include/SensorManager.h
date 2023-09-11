/*
 *
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/CHIPError.h>

class SensorManager
{
public:
    CHIP_ERROR Init();

private:
    friend SensorManager & SensorMgr();

    // Reads new generated sensor value, stores it, and updates local temperature attribute
    static void SensorTimerEventHandler(TimerHandle_t xTimer);

    static SensorManager sSensorManager;
};

inline SensorManager & SensorMgr()
{
    return SensorManager::sSensorManager;
}
