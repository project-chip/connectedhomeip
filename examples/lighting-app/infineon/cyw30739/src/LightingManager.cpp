/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LightingManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/att-storage.h>
#include <app/util/af.h>
#include <stdio.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

LightingManager LightingManager::sLight;

CHIP_ERROR LightingManager::Init()
{
    mState = kState_OffCompleted;

    return CHIP_NO_ERROR;
}

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
    bool on                    = true;
    const EmberAfStatus status = OnOff::Attributes::OnOff::Get(1, &on);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("Error ReadServerAttribute 0x%02x\n", status);
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
    const EmberAfStatus status = OnOff::Attributes::OnOff::Set(1, value);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("Error WriteServerAttribute 0x%02x\n", status);
    }
}

void LightingManager::WriteClusterLevel(uint8_t value)
{
    const EmberAfStatus status = LevelControl::Attributes::CurrentLevel::Set(1, value);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("Error WriteServerAttribute 0x%02x\n", status);
    }
}
