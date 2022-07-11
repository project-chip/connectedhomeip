/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>

#include <LightingManager.h>
#include <TizenServiceAppMain.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if (attributePath.mClusterId == OnOff::Id && attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
}

void ApplicationInit() {}

int main(int argc, char * argv[])
{
    TizenServiceAppMain app;
    VerifyOrDie(app.Init(argc, argv) == 0);

    VerifyOrDie(LightingMgr().Init() == CHIP_NO_ERROR);

    return app.RunMainLoop();
}
