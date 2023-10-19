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

#include "DishwasherManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>
#include "LEDWidget.h"

#include "operational-state-delegate-impl.h"

#include <lib/support/TypeTraits.h>

#if (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT) || defined(SIWX_917))
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
using namespace chip::DeviceLayer;

DishwasherManager DishwasherManager::sDishwasher;

TimerHandle_t sDishwasherTimer;

CHIP_ERROR DishwasherManager::Init()
{
    sDishwasherLED.Init(DW_STATE_LED);

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OperationalStateEnum state = static_cast<OperationalStateEnum>(OperationalState::GetInstance()->GetCurrentOperationalState());
    UpdateOperationState(state);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    return CHIP_NO_ERROR;
}

OperationalStateEnum DishwasherManager::GetOperationalState()
{
    return mState;
}

void DishwasherManager::UpdateDishwasherLed()
{   
    OperationalStateEnum opState = DishwasherMgr().GetOperationalState();
    sDishwasherLED.Set(false);

    switch(opState)
    {
        case OperationalStateEnum::kRunning :
            sDishwasherLED.Set(true);
        break;
        case OperationalStateEnum::kPaused :
            sDishwasherLED.Blink(300,700);
        break;
        case OperationalStateEnum::kError :
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
        switch(mState)
        {
            case OperationalStateEnum::kRunning :
                action = OperationalStateEnum::kPaused;
            break;
            case OperationalStateEnum::kPaused :
                action = OperationalStateEnum::kStopped;
            break;
            case OperationalStateEnum::kStopped :
                action = OperationalStateEnum::kRunning;
            break;
            case OperationalStateEnum::kError :
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
