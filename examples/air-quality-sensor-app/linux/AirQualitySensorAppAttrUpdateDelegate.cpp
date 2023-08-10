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

#include "AirQualitySensorAppAttrUpdateDelegate.h"
#include "AirQualitySensorAttrUpdateHandler.h"

#include <platform/PlatformManager.h>

using namespace chip;

void AirQualitySensorAppAttrUpdateDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = AirQualitySensorAttrUpdateHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "AllClusters App: Unable to instantiate a command handler");
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(AirQualitySensorAttrUpdateHandler::HandleCommand,
                                                  reinterpret_cast<intptr_t>(handler));
}
