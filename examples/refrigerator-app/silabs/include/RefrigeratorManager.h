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
#include "AppEvent.h"
#include <app/clusters/refrigerator-alarm-server/refrigerator-alarm-server.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>
#include <stdbool.h>
#include <stdint.h>
#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAlarm::Attributes;
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureControl::Attributes;
using namespace chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

// AppCluster Spec Table 85.
enum SUPPORTED_MODES
{
    NORMAL = 0x0000,
    ENERGY_SAVE = 0x0004,
    RAPID_COOL = 0x4000,
    RAPID_FREEZE = 0x4001,
};

class RefrigeratorManager
{
public:
    CHIP_ERROR Init();
    void RefAndTempCtrlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size);
    void TempCtrlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size);
    void RefAlaramAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size);
    uint8_t GetMode();
    int8_t GetCurrentTemp();
    int8_t SetMode();

private:
    friend RefrigeratorManager & RefrigeratorMgr();

    uint8_t mCurrentMode;
    uint8_t mStartUpMode;
    uint8_t mOnMode;

    int16_t mTemperatureSetpoint;
    int16_t mMinTemperature;
    int16_t mMaxTemperature;
    int16_t mStep;
    int16_t mSelectedTemperatureLevel;
    int16_t mSupportedTemperatureLevels;

    uint8_t mMask;
    uint8_t mState;
    uint8_t mSupported;

    int8_t ConvertToPrintableTemp(int16_t temperature);
    static RefrigeratorManager sRefrigeratorMgr;
};

inline RefrigeratorManager & RefrigeratorMgr()
{
    return RefrigeratorManager::sRefrigeratorMgr;
}
