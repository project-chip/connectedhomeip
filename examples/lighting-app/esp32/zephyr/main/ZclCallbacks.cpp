/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AppConfig.h"
#include "AppTask.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

// Invoked by the data model whenever an attribute is written. This runs on the
// CHIP (Matter) thread; SetOnOffLED is a direct GPIO write, safe from here.
void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if (attributePath.mEndpointId == LIGHT_ENDPOINT_ID && attributePath.mClusterId == OnOff::Id &&
        attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        AppTask::Instance().SetOnOffLED(*value != 0);
    }
}
