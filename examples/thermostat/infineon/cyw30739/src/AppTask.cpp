/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AppTask.h"
#include "SensorManager.h"
#include "TemperatureManager.h"
#ifdef BOARD_ENABLE_DISPLAY
#include "ThermostatUI.h"
#endif
#include <app/clusters/identify-server/identify-server.h>
#include <matter_config.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::app;

static void SensorInterruptHandler(intptr_t aArg);

void AppTaskMain(intptr_t args)
{
    SensorMgr().Init(SensorInterruptHandler);
    if (TempMgr().Init() != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TempMgr Init fail");
    }

    CYW30739MatterConfig::InitApp();
}

void UpdateThermoStatUI(intptr_t aArg)
{
#ifdef BOARD_ENABLE_DISPLAY
    ThermostatUI::SetCurrentTemp(TempMgr().GetCurrentTemp());
    ThermostatUI::SetCurrentHumidity(TempMgr().GetCurrentHumidity());
    ThermostatUI::DrawUI();
#endif
    ChipLogDetail(Zcl, "Thermostat Status - M:%d Temp:%d'C Humidity:%d'C H:%d'C C:%d'C", to_underlying(TempMgr().GetMode()),
                  TempMgr().GetCurrentTemp(), TempMgr().GetCurrentHumidity(), TempMgr().GetHeatingSetPoint(),
                  TempMgr().GetCoolingSetPoint());
}

void SensorInterruptHandler(intptr_t aArg)
{
    TempMgr().SensorInterruptHandler();
    UpdateThermoStatUI(aArg);
}

void OnIdentifyTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        break;
    }
}

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnIdentifyTriggerEffect,
};
