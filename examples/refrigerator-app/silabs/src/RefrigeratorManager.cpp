/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/support/logging/CHIPLogging.h>
#include <static-supported-temperature-levels.h>

/**********************************************************
 * Defines and Constants
 *********************************************************/

using namespace chip;
using namespace ::chip::DeviceLayer;

namespace RefAndTempAttr = chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Attributes;
namespace RefAlarmAttr   = chip::app::Clusters::RefrigeratorAlarm::Attributes;
namespace TempCtrlAttr   = chip::app::Clusters::TemperatureControl::Attributes;

// set Parent Endpoint and Composition Type for an Endpoint
EndpointId kRefEndpointId           = 1;
EndpointId kColdCabinetEndpointId   = 2;
EndpointId kFreezeCabinetEndpointId = 3;

RefrigeratorManager RefrigeratorManager::sRefrigeratorMgr;

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
    return CHIP_NO_ERROR;
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

void RefrigeratorManager::TempCtrlAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value,
                                                         uint16_t size)
{
    switch (attributeId)
    {
    case TempCtrlAttr::TemperatureSetpoint::Id: {
        int16_t temperatureSetpoint = ConvertToPrintableTemp(static_cast<int16_t>(*value));
        mTemperatureSetpoint        = temperatureSetpoint;
        TempCtrlAttr::TemperatureSetpoint::Set(endpointId, temperatureSetpoint);
    }
    break;
    default: {
        ChipLogError(AppServer, "Unhandled Temperature controlled attribute %ld", attributeId);
        return;
    }
    break;
    }
}

void RefrigeratorManager::RefAlaramAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId, uint8_t * value,
                                                          uint16_t size)
{
    switch (attributeId)
    {
    case RefAlarmAttr::Mask::Id: {
        auto mask = static_cast<uint32_t>(*value);
        mState    = static_cast<chip::app::Clusters::RefrigeratorAlarm::AlarmBitmap>(mask);
        RefAlarmAttr::Mask::Set(endpointId, mMask);
    }
    break;

    case RefAlarmAttr::State::Id: {
        auto state = static_cast<uint32_t>(*value);
        mState     = static_cast<chip::app::Clusters::RefrigeratorAlarm::AlarmBitmap>(state);
        RefAlarmAttr::State::Set(endpointId, mState);
    }
    break;

    default: {
        ChipLogError(AppServer, "Unhandled Refrigerator Alarm attribute %ld", attributeId);
        return;
    }
    break;
    }
}
