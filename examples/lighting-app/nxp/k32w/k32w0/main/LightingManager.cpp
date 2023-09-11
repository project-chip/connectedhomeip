/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LightingManager.h"

#include "AppTask.h"
#include "FreeRTOS.h"

#include "app_config.h"

LightingManager LightingManager::sLight;

int LightingManager::Init()
{
    mState = kState_On;

    return 0;
}

void LightingManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

void LightingManager::SetState(bool state)
{
    mState = state ? kState_On : kState_Off;
}

bool LightingManager::IsTurnedOff()
{
    return (mState == kState_Off) ? true : false;
}

bool LightingManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;

    if (mState == kState_On && aAction == TURNOFF_ACTION)
    {
        action_initiated = true;
        mState           = kState_Off;
    }
    else if (mState == kState_Off && aAction == TURNON_ACTION)
    {
        action_initiated = true;
        mState           = kState_On;
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
