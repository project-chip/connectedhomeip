/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <app-common/zap-generated/attributes/Accessors.h>

#include <lib/core/CHIPError.h>

using namespace chip;

// AppCluster Spec Table 85.
enum ThermMode
{
    OFF = 0,
    AUTO,
    NOT_USED,
    COOL,
    HEAT,
};

class TemperatureManager
{
public:
    CHIP_ERROR Init();
    void AttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size);
    uint8_t GetMode();
    int8_t GetCurrentTemp();
    int8_t GetHeatingSetPoint();
    int8_t GetCoolingSetPoint();

private:
    friend TemperatureManager & TempMgr();

    int8_t mCurrentTempCelsius;
    int8_t mCoolingCelsiusSetPoint;
    int8_t mHeatingCelsiusSetPoint;
    uint8_t mThermMode;

    int8_t ConvertToPrintableTemp(int16_t temperature);
    static TemperatureManager sTempMgr;
};

inline TemperatureManager & TempMgr()
{
    return TemperatureManager::sTempMgr;
}
