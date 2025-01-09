/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 
#include <stdlib.h>
#include "CHIPDeviceManager.h"
#include "AppTask.h"

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

extern "C" void ChipTestShutdown(void)
{
    ChipLogProgress(DeviceLayer, "Light switch APP Demo! Shutdown Now!");
    CHIPDeviceManager::GetInstance().Shutdown();
}

extern "C" void InitGPIO(void)
{
    GetAppTask().InitGpio();
}

extern "C" void ChipTest(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "GetAppTask().StartAppTask() failed");
        return;
    }
}

extern "C" void InitiateSingleSwitch(uint8_t index, uint8_t action)
{
    if(index >= MAX_SUPPORTED_LIGHT_NUM)
    {
        ChipLogError(DeviceLayer, "Endpoint index exceed!");
        return;
    }

    LightSwitch::GetInstance().InitiateActionSwitch(kLightSwitchEndpointId[index], action);
}

extern "C" void InitiateGroupSwitch(uint8_t action)
{
    LightSwitch::GetInstance().InitiateActionSwitch(kLightSwitchForGroupEndpointId, action);
}

extern "C" void InitiateGroupSwitch2(uint8_t index, uint8_t action)
{
    LightSwitch::GetInstance().InitiateActionSwitch(kLightSwitchEndpointId[index], action);
}


#if CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE

extern "C" void ShutdownSubscribeRequestForOneNode(uint8_t index)
{
    if(index >= MAX_SUPPORTED_LIGHT_NUM)
    {
        ChipLogError(DeviceLayer, "Endpoint index exceed!");
        return;
    }   
    
     LightSwitch::GetInstance().ShutdownSubscribeRequestForOneNode(kLightSwitchEndpointId[index]);
}

extern "C" void SubscribeRequestForOneNode(uint8_t index)
{
    if(index >= MAX_SUPPORTED_LIGHT_NUM)
    {
        ChipLogError(DeviceLayer, "Endpoint index exceed!");
        return;
    }
    LightSwitch::GetInstance().SubscribeRequestForOneNode(kLightSwitchEndpointId[index]);
}

typedef void (*P_ProcessCommandCallback)(uint8_t status, uint8_t index);
static P_ProcessCommandCallback g_NotifyUpperStatusChange = NULL;

void UpdateLightingStatetoGUI(EndpointId endpointId, uint8_t status)
{
    uint8_t index;

    for(index = 0; index < MAX_SUPPORTED_LIGHT_NUM; index++)
    {
        if(endpointId == kLightSwitchEndpointId[index])
        {
            break;
        }
    }
    
    if(g_NotifyUpperStatusChange)
    {
        g_NotifyUpperStatusChange(status,index);
    }
}

extern "C" void RegisterSwitchCommandCallback(P_ProcessCommandCallback cback)
{
    g_NotifyUpperStatusChange = cback;
}
#endif