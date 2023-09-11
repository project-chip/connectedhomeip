/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEventCommon.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/CHIPError.h>

class SensorManager
{
public:
    CHIP_ERROR Init();

    int16_t GetMeasuredValue();
    int16_t GetMinMeasuredValue();
    int16_t GetMaxMeasuredValue();

private:
    friend SensorManager & SensorMgr();

    // Reads new generated sensor value, stores it, and updates local temperature attribute
    static int16_t SensorEventHandler();

    int16_t mMeasuredTempCelsius;
    int16_t mMinMeasuredTempCelsius = -40;
    int16_t mMaxMeasuredTempCelsius = 120;

    static SensorManager sSensorManager;
};

inline SensorManager & SensorMgr()
{
    return SensorManager::sSensorManager;
}
