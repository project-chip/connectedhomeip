/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "LEDWidget.h"

#include "AppConfig.h"
#include "AppTask.h"
#include "DishwasherManager.h"
#include "operational-state-delegate-impl.h"

#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
#define DW_STATE_LED 1
#else
#define DW_STATE_LED 0
#endif

namespace {
LEDWidget sDishwasherLED;
}

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::DeviceLayer;

CHIP_ERROR DishwasherManager::Init()
{
    sDishwasherLED.Init(DW_STATE_LED);
    AppTask::GetAppTask().LinkAppLed(&sDishwasherLED);

    chip::app::Clusters::DeviceEnergyManagement::DeviceEnergyManagementDelegate * dem = GetDEMDelegate();
    VerifyOrReturnLogError(dem != nullptr, CHIP_ERROR_UNINITIALIZED);

    /* For Device Energy Management we need the ESA to be Online and ready to accept commands */
    dem->SetESAState(ESAStateEnum::kOnline);
    dem->SetESAType(ESATypeEnum::kDishwasher);

    // Set the abs min and max power
    dem->SetAbsMinPower(1200000); // 1.2KW
    dem->SetAbsMaxPower(7600000); // 7.6KW

    return CHIP_NO_ERROR;
}

OperationalStateEnum DishwasherManager::GetOperationalState()
{
    return mState;
}

void DishwasherManager::UpdateDishwasherLed()
{
    OperationalStateEnum opState = GetOperationalState();
    sDishwasherLED.Set(false);

    switch (opState)
    {
    case OperationalStateEnum::kRunning:
        sDishwasherLED.Set(true);
        break;
    case OperationalStateEnum::kPaused:
        sDishwasherLED.Blink(300, 700);
        break;
    case OperationalStateEnum::kError:
        sDishwasherLED.Blink(100);
        break;
    default:
        break;
    }
}

void DishwasherManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

void DishwasherManager::CycleOperationalState()
{
    if (mActionInitiated_CB)
    {
        OperationalStateEnum action;
        switch (mState)
        {
        case OperationalStateEnum::kRunning:
            action = OperationalStateEnum::kPaused;
            break;
        case OperationalStateEnum::kPaused:
            action = OperationalStateEnum::kStopped;
            break;
        case OperationalStateEnum::kStopped:
            action = OperationalStateEnum::kRunning;
            break;
        case OperationalStateEnum::kError:
            action = OperationalStateEnum::kStopped;
            break;
        default:
            break;
        }
        mActionInitiated_CB(action);
    }
}

void DishwasherManager::UpdateOperationState(OperationalStateEnum state)
{
    mState = state;
    UpdateDishwasherLed();

    if (mActionCompleted_CB)
    {
        mActionCompleted_CB();
    }
}

int64_t DishwasherManager::GetApproxEnergyDuringSession()
{
    return kApproximateEnergyUsedByESA;
};

CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementPowerAdjustRequest(const int64_t powerMw, const uint32_t durationS,
                                                                             AdjustmentCauseEnum cause)
{
    // Currently not implemented by our dishwasher app
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementPowerAdjustCompletion()
{
    // Currently not implemented by our dishwasher app
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementCancelPowerAdjustRequest(CauseEnum cause)
{
    // Currently not implemented by our dishwasher app
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

// kNoOptOut    = 0x00,
// kLocalOptOut = 0x01,
// kGridOptOut  = 0x02,
// kOptOut      = 0x03,
CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementStartTimeAdjustRequest(const uint32_t requestedStartTimeUtc,
                                                                                 AdjustmentCauseEnum cause)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = nullptr;
    AppTask::GetAppTask().PostEvent(&event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementPauseRequest(const uint32_t durationS, AdjustmentCauseEnum cause)
{
    mActionInitiated_CB(OperationalStateEnum::kPaused);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementPauseCompletion()
{
    mActionInitiated_CB(OperationalStateEnum::kRunning);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementCancelPauseRequest(CauseEnum cause)
{
    mActionInitiated_CB(OperationalStateEnum::kRunning);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DishwasherManager::HandleDeviceEnergyManagementCancelRequest()
{
    mActionInitiated_CB(OperationalStateEnum::kStopped);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DishwasherManager::HandleModifyForecastRequest(
    const uint32_t forecastId, const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
    AdjustmentCauseEnum cause)
{
    // Currently not implemented by our dishwasher app
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DishwasherManager::RequestConstraintBasedForecast(
    const DataModel::DecodableList<DeviceEnergyManagement::Structs::ConstraintsStruct::DecodableType> & constraints,
    AdjustmentCauseEnum cause)
{
    // Currently not implemented by our dishwasher app
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
