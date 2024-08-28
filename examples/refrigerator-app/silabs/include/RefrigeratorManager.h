/*
 *
 *    Copyright (c) 2019 Google LLC.
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
#include <app/clusters/refrigerator-alarm-server/refrigerator-alarm-server.h>

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>

using namespace chip;

// AppCluster Spec Table 85.
enum RefrigeratorMode
{
    OFF = 0,
    NORMAL,
    RAPID_COOL,
    RAPID_FREEZE,
    ENERGY_SAVE,
    HEAT,
};

class RefrigeratorManager
{
public:
    CHIP_ERROR Init();
    void AttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size);
    uint8_t GetMode();
    int8_t GetCurrentTemp();
    int8_t SetMode();

private:
    friend RefrigeratorManager & RefrigeratorMgr();

    int8_t mCurrentTempCelsius;
    uint8_t mRefrigeratorMode;

    int8_t ConvertToPrintableTemp(int16_t temperature);
    static RefrigeratorManager sRefrigeratorMgr;
};

inline RefrigeratorManager & RefrigeratorMgr()
{
    return RefrigeratorManager::sRefrigeratorMgr;
}
