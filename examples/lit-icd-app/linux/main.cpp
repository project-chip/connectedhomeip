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

#include "AppMain.h"
#include <app-common/zap-generated/ids/Clusters.h>

#include "system/SystemClock.h"

using namespace chip;
using namespace chip::app;
using namespace chip::System::Clock::Literals;

void ApplicationInit() {}

void ApplicationShutdown() {}

void notifyIcdActive(System::Layer * layer, void *)
{
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
    DeviceLayer::SystemLayer().StartTimer(10000_ms32, notifyIcdActive, nullptr);
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);
    DeviceLayer::SystemLayer().StartTimer(10000_ms32, notifyIcdActive, nullptr);
    ChipLinuxAppMainLoop();
    return 0;
}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    // TODO: Watch ICDM Cluster changes
}
