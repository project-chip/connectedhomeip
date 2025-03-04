/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "LightingManager.h"

#include "AppTask.h"
#include "FreeRTOS.h"


LightingManager LightingManager::sLight;


CHIP_ERROR LightingManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mState                = kState_TurnOffCompleted;
    return err;
}

void LightingManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightingManager::IsActionInProgress()
{
    return (mState == kState_TurnOnInitiated || mState == kState_TurnOffInitiated) ? true : false;
}

bool LightingManager::IsTurnedOff()
{
    return (mState == kState_TurnOffCompleted) ? true : false;
}


bool LightingManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;

    // Initiate ON/OFF Action only when the previous one is complete.
    if (mState == kState_TurnOnCompleted && aAction == TURNOFF_ACTION)
    {
        action_initiated = true;
		mState = kState_TurnOffCompleted;
    }
    else if (mState == kState_TurnOffCompleted && aAction == TURNON_ACTION)
    {
        action_initiated = true;
		mState = kState_TurnOnCompleted;
    }

    if (action_initiated)
    {

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
		if (mActionCompleted_CB)
        {
            mActionCompleted_CB(aAction);
        }
    }

    return action_initiated;
}

