/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**********************************************************
 * Includes
 *********************************************************/

#include "RefrigeratorManager.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"

/**********************************************************
 * Defines and Constants
 *********************************************************/

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
using namespace ::chip::app::Clusters::RefrigeratorAlarm;
using namespace ::chip::app::Clusters::TemperatureControl;

namespace RefAndTempAttr = chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Attributes;
namespace RefAlarmAttr = chip::app::Clusters::RefrigeratorAlarm::Attributes;
namespace TempCtrlAttr = chip::app::Clusters::TemperatureControl::Attributes;

// set Parent Endpoint and Composition Type for an Endpoint
EndpointId kRefEndpointId           = 1;
EndpointId kColdCabinetEndpointId   = 2;
EndpointId kFreezeCabinetEndpointId = 3;

namespace {
    app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;

    // Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
    constexpr const uint8_t kNamespaceRefrigerator = 0x41;
    // Refrigerator Namespace: 0x41, tag 0x00 (Refrigerator)
    constexpr const uint8_t kTagRefrigerator = 0x00;
    // Refrigerator Namespace: 0x41, tag 0x01 (Freezer)
    constexpr const uint8_t kTagFreezer                                                = 0x01;
    const Clusters::Descriptor::Structs::SemanticTagStruct::Type refrigeratorTagList[] = { { .namespaceID = kNamespaceRefrigerator,
                                                                                            .tag         = kTagRefrigerator } };
    const Clusters::Descriptor::Structs::SemanticTagStruct::Type freezerTagList[]      = { { .namespaceID = kNamespaceRefrigerator,
                                                                                            .tag         = kTagFreezer } };
} // namespace

CHIP_ERROR RefrigeratorManager::Init()
{
    SetTreeCompositionForEndpoint(kRefEndpointId);
    SetParentEndpointForEndpoint(kColdCabinetEndpointId, kRefEndpointId);
    SetParentEndpointForEndpoint(kFreezeCabinetEndpointId, kRefEndpointId);

    // set TagList
    SetTagList(kColdCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(refrigeratorTagList));
    SetTagList(kFreezeCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(freezerTagList));

    app::Clusters::TemperatureControl::SetInstance(&sAppSupportedTemperatureLevelsDelegate);
}

int8_t RefrigeratorManager::ConvertToPrintableTemp(int16_t temperature)
{
    constexpr uint8_t kRoundUpValue = 50;

    // Round up the temperature as we won't print decimals on LCD
    // Is it a negative temperature
    if (temperature < 0)
    {
        temperature -= kRoundUpValue;
    }
    else
    {
        temperature += kRoundUpValue;
    }

    return static_cast<int8_t>(temperature / 100);
}


void RefrigeratorManager::RefAndTempCtrlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size)
{
    switch (attributeId)
    {
        case RefAndTempAttr::CurrentMode::Id: {
            int8_t Temp = ConvertToPrintableTemp(*((int16_t *) value));
            mCurrentMode = Temp;
        }
        break;

        case RefAndTempAttr::StartUpMode::Id: {
            int8_t startUpMode = ConvertToPrintableTemp(*((int16_t *) value));
            mStartUpMode = startUpMode;
        }
        break;

        case RefAndTempAttr::OnMode::Id: {
            int8_t onMode = ConvertToPrintableTemp(*((int16_t *) value));
            mOnMode = onMode;
        }
        break;

        default: {
            SILABS_LOG("Unhandled Refrigerator and Temprature attribute %x", attributeId);
            return;
        }
        break;
    }
}

void RefrigeratorManager::TempCtrlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size)
{
    switch (attributeId)
    {
        case TempCtrlAttr::TemperatureSetpoint::Id: {
            int8_t temperatureSetpoint = ConvertToPrintableTemp(*((int16_t *) value));
            mTemperatureSetpoint = temperatureSetpoint;
        }
        break;

        case TempCtrlAttr::MinTemperature::Id: {
            int8_t minTemperature = ConvertToPrintableTemp(*((int16_t *) value));
            mMinTemperature = minTemperature;
        }
        break;

        case TempCtrlAttr::MaxTemperature::Id: {
            int8_t maxTemperature = ConvertToPrintableTemp(*((int16_t *) value));
            mMaxTemperature = maxTemperature;
        }
        break;

        case TempCtrlAttr::Step::Id: {
            int8_t step = ConvertToPrintableTemp(*((int16_t *) value));
            mStep = step;
        }
        break;

        case TempCtrlAttr::SelectedTemperatureLevel::Id: {
            int8_t selectedTemperatureLevel = ConvertToPrintableTemp(*((int16_t *) value));
            mSelectedTemperatureLevel = selectedTemperatureLevel;
        }
        break;

        case TempCtrlAttr::SupportedTemperatureLevels::Id: {
            int8_t supportedTemperatureLevels = ConvertToPrintableTemp(*((int16_t *) value));
            mSupportedTemperatureLevels = supportedTemperatureLevels;
        }
        break;

        default: {
            SILABS_LOG("Unhandled Temprature controlled attribute %x", attributeId);
            return;
        }
        break;
    }

    AppTask::GetAppTask().UpdateThermoStatUI();
}

void RefrigeratorManager::RefAlaramAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value, uint16_t size)
{
    switch (attributeId)
    {
        case RefAlarmAttr::Mask::Id: {
            int8_t Temp = ConvertToPrintableTemp(*((int16_t *) value));
            mCurrentMode = Temp;
        }
        break;

        case RefAlarmAttr::State::Id: {
            int8_t startUpMode = ConvertToPrintableTemp(*((int16_t *) value));
            mStartUpMode = startUpMode;
        }
        break;

        case RefAlarmAttr::Supported::Id: {
            int8_t heatingTemp = ConvertToPrintableTemp(*((int16_t *) value));
            mHeatingCelsiusSetPoint = heatingTemp;
        }
        break;

        default: {
            SILABS_LOG("Unhandled Refrigerator Alarm attribute %x", attributeId);
            return;
        }
        break;
    }
}