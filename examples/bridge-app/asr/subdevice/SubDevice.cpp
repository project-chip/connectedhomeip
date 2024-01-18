/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "SubDevice.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#ifdef CONFIG_ENABLE_ASR_APP_MESH
#include "app.h"
#include "sonata_gap_api.h"
#include "sonata_gatt_api.h"
#endif
#ifdef __cplusplus
}
#endif

#include <cstdio>
#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip::Platform;

SubDevice::SubDevice(const char * szSubDeviceName, const char * szLocation)
{
    CopyString(mName, sizeof(mName), szSubDeviceName);
    CopyString(mLocation, sizeof(mLocation), szLocation);
    mState      = kState_Off;
    mReachable  = false;
    mEndpointId = 0;
    mChanged_CB = nullptr;
}

bool SubDevice::IsOn() const
{
    return mState == kState_On;
}

bool SubDevice::IsReachable() const
{
    return mReachable;
}

void SubDevice::SetOnOff(bool aOn)
{
    bool changed;

    if (aOn)
    {
        changed = (mState != kState_On);
        mState  = kState_On;
        ChipLogProgress(DeviceLayer, "SubDevice[%s]: ON", mName);
#ifdef CONFIG_ENABLE_ASR_APP_MESH
        if (strcmp(mName, "Light 1") == 0)
        {
            app_mesh_control_fan(1);
        }
        else
        {
            app_mesh_control_light(1);
        }
#endif
    }
    else
    {
        changed = (mState != kState_Off);
        mState  = kState_Off;
        ChipLogProgress(DeviceLayer, "SubDevice[%s]: OFF", mName);
#ifdef CONFIG_ENABLE_ASR_APP_MESH
        if (strcmp(mName, "Light 1") == 0)
        {
            app_mesh_control_fan(0);
        }
        else
        {
            app_mesh_control_light(0);
        }
#endif
    }

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_State);
    }
}

void SubDevice::SetReachable(bool aReachable)
{
    bool changed = (mReachable != aReachable);

    mReachable = aReachable;

    if (aReachable)
    {
        ChipLogProgress(DeviceLayer, "SubDevice[%s]: ONLINE", mName);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "SubDevice[%s]: OFFLINE", mName);
    }

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_Reachable);
    }
}

void SubDevice::SetName(const char * szName)
{
    bool changed = (strncmp(mName, szName, sizeof(mName)) != 0);

    ChipLogProgress(DeviceLayer, "SubDevice[%s]: New Name=\"%s\"", mName, szName);

    CopyString(mName, sizeof(mName), szName);

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_Name);
    }
}

void SubDevice::SetLocation(const char * szLocation)
{
    bool changed = (strncmp(mLocation, szLocation, sizeof(mLocation)) != 0);

    CopyString(mLocation, sizeof(mLocation), szLocation);

    ChipLogProgress(DeviceLayer, "SubDevice[%s]: Location=\"%s\"", mName, mLocation);

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_Location);
    }
}

void SubDevice::SetChangeCallback(SubDeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}
