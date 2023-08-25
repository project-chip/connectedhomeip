/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/support/CHIPMem.h>
#include "AirQualitySensorAppAttrUpdateDelegate.h"
#include "AirQualitySensorAttrUpdateHandler.h"
#include "UartHelper.h"

#include <platform/PlatformManager.h>

#define BUFFER_SIZE 256

using namespace chip;


AirQualitySensorAppAttrUpdateDelegate::AirQualitySensorAppAttrUpdateDelegate(void)
{
    char uartBuff[BUFFER_SIZE];
    //char *json;
    int32_t jsonStructLen = 0;
    
    while(1)
    {
        jsonStructLen = UartGetJsonStruct(uartBuff, BUFFER_SIZE);

        if(jsonStructLen > 0)
        {
            //json = Platform::MemoryAlloc((size_t)jsonStructLen);
            // memcpy(json, uartBuff, jsonStructureLen);
            //Assuming OnEventCommandReceived makes a copy of buffer
            //Otherwise need to allocate memory and set up an event to free memory once schedulework is done
            OnEventCommandReceived(uartBuff);
        }
    }
}

void AirQualitySensorAppAttrUpdateDelegate::OnEventCommandReceived(const char * json)
{
    ChipLogError(NotSpecified, "Received: %s", json);
    auto handler = AirQualitySensorAttrUpdateHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "AirQualitySensor App: Unable to instantiate a command handler");
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(AirQualitySensorAttrUpdateHandler::HandleCommand,
                                                  reinterpret_cast<intptr_t>(handler));
}
