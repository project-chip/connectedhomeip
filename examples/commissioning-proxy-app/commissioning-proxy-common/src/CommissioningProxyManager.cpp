/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

// stdlib
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// POSIX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Local includes
#include "CommissioningProxyManager.h"

#include <lib/support/logging/CHIPLogging.h>

// Location of the executable to use by default to power dumb USB light up and
// down
#define UHUBCTL_PATHNAME "/usr/sbin/uhubctl"

// Name of the environment variable where additional UHUBCTL arguments may be
// specified
#define UHUBCTL_ARGUMENT_ENV_NAME "MORSE_MICRO_MATTER_UHUBCTL_COMMAND"


CommissioningProxyManager CommissioningProxyManager::sCommissioningProxy;

CHIP_ERROR CommissioningProxyManager::Init()
{
    mMainCommissioningProxyState = kState_CPDisconnected;
    return CHIP_NO_ERROR;
}

bool CommissioningProxyManager::IsCPConnected()
{
    return mMainCommissioningProxyState == kState_CPConnected;
}

#if 0 
void CommissioningProxyManager::SetCallbacks(CommissioningProxyCallback_fn aActionInitiated_CB, CommissioningProxyCallback_fn aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}
#endif

void CommissioningProxyManager::SetCPState(uint8_t level)
{
   mMainCommissioningProxyState = kState_CPConnected;
    ChipLogDetail(AppServer, "%s: CP Connected", __func__);
} 

bool CommissioningProxyManager::InitiateAction(Action_t aAction)
{
    // this function is called InitiateAction because we may want to implement some features such as ramping up here.
    bool action_initiated = false;
    //State_t new_state;

    switch (aAction)
    {
    case CONNECT_ACTION:
        ChipLogProgress(AppServer, "CommissioningProxyManager::InitiateAction(CONNECT_ACTION)");
        break;
    case DISCONNECT_ACTION:
        ChipLogProgress(AppServer, "CommissioningProxyManager::InitiateAction(DISCONNECT_ACTION)");
        break;
    default:
        ChipLogProgress(AppServer, "CommissioningProxyManager::InitiateAction(unknown)");
        break;
    }

    // Initiate On/Off Action only when the previous one is complete.
    if (mMainCommissioningProxyState == kState_CPDisconnected && aAction == CONNECT_ACTION)
    {
        action_initiated = true;
        //new_state        = kState_CPConnected;
    }
    else if (mMainCommissioningProxyState == kState_CPConnected && aAction == DISCONNECT_ACTION)
    {
        action_initiated = true;
        //new_state        = kState_CPDisconnected;
    }

    if (action_initiated)
    {
        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction);
        }

        //SetMainLight(new_state == kState_CPConnected);

        if (mActionCompleted_CB)
        {
            mActionCompleted_CB(aAction);
        }
    }

    return action_initiated;
}

#if 0
void CommissioningProxyManager::SetMainLight(bool aOn)
{
    // Set the main light to the default color if on or the 'off' color if off
    SetMainLight(aOn ? LIGHT_DEFAULT_ON_COLOR_MAIN : LIGHT_OFF_COLOR);
}

void CommissioningProxyManager::RestoreMainLight(void)
{
    // Synchronize the physical representation of the main light to the logical
    // representation
    SetMainLight(mMainCommissioningProxyState == kState_CPConnected ? mMainLightColor : LIGHT_OFF_COLOR);
}

void CommissioningProxyManager::SetMainLight(uint32_t color)
{
    // Set the physical main light to the requested state.
    // The default legacy behaviour may have been overridden through
    // environment variables

    mMainCommissioningProxyState = (color == LIGHT_OFF_COLOR) ? kState_CPDisconnected : kState_CPConnected
    mMainLightColor = (color == LIGHT_OFF_COLOR) ? mMainLightColor : color;
}

void CommissioningProxyManager::SetStatusLight(bool aOn)
{
    // Set the secondary visual indicator to the default color if on or the 'off' color if off
    SetStatusLight(aOn ? LIGHT_DEFAULT_ON_COLOR_STATUS : LIGHT_OFF_COLOR);
}

void CommissioningProxyManager::SetStatusLight(uint32_t color)
{
    mStatusLightState = (color == LIGHT_OFF_COLOR) ? kState_CPDisconnected : kState_CPConnected
    mStatusLightColor = (color == LIGHT_OFF_COLOR) ? mStatusLightColor : color;
}

void CommissioningProxyManager::SetIdentifyLight(bool aOn)
{
    // Set the Identify indicator to the default color if on or the 'off' color if off
    SetIdentifyLight(aOn ? LIGHT_DEFAULT_ON_COLOR_IDENTIFY : LIGHT_OFF_COLOR);
}

void CommissioningProxyManager::SetIdentifyLight(uint32_t color)
{
    // Set the physical Identify indicator to the requested state.
    // At present this directly uses the secondary visual indicator.
    mIdentifyLightState = (color == LIGHT_OFF_COLOR) ? kState_CPDisconnected : kState_CPConnected
    mIdentifyLightColor = (color == LIGHT_OFF_COLOR) ? mIdentifyLightColor : color;

    SetStatusLight(color);
}

void CommissioningProxyManager::ToggleIdentifyLight(void)
{
    ToggleIdentifyLight(mIdentifyLightColor);
}

void CommissioningProxyManager::ToggleIdentifyLight(uint32_t color)
{
    // Toggle the physical Identify indicator to provide an easy flash
    // mechanism
    mIdentifyLightState = (mIdentifyLightState == kState_CPConnected) ? kState_CPDisconnected : kState_CPConnected
    SetIdentifyLight(mIdentifyLightState == kState_CPConnected ? color : LIGHT_OFF_COLOR);
}
#endif
