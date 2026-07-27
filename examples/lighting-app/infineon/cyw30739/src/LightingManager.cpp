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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <stdio.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

LightingManager LightingManager::sLight;

void LightingManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightingManager::IsActionInProgress()
{
    return (mState == kState_OffInitiated || mState == kState_OnInitiated);
}

bool LightingManager::IsLightOn(void)
{
    bool on                                          = true;
    const Protocols::InteractionModel::Status status = OnOff::Attributes::OnOff::Get(1, &on);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        printf("Error ReadServerAttribute 0x%02x\n", to_underlying(status));
    }

    return on != false;
}

bool LightingManager::InitiateAction(Actor_t aActor, Action_t aAction, uint8_t value)
{
    if (mActionInitiated_CB)
    {
        mActionInitiated_CB(aActor, aAction, value);
    }

    switch (aAction)
    {
    case ON_ACTION:
        mState = kState_OnInitiated;
        if (aActor != ACTOR_ZCL_CMD)
        {
            WriteClusterState(1);
        }
        mState = kState_OnCompleted;
        break;
    case OFF_ACTION:
        mState = kState_OffInitiated;
        if (aActor != ACTOR_ZCL_CMD)
        {
            WriteClusterState(0);
        }
        mState = kState_OffCompleted;
        break;
    default:
        return false;
    }

    if (mActionCompleted_CB)
    {
        mActionCompleted_CB(aAction);
    }

    return true;
}

void LightingManager::WriteClusterState(uint8_t value)
{
    const Protocols::InteractionModel::Status status = OnOff::Attributes::OnOff::Set(1, value);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        printf("Error WriteServerAttribute 0x%02x\n", to_underlying(status));
    }
}

void LightingManager::WriteClusterLevel(uint8_t value)
{
    const Protocols::InteractionModel::Status status = LevelControl::Attributes::CurrentLevel::Set(1, value);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        printf("Error WriteServerAttribute 0x%02x\n", to_underlying(status));
    }
}
