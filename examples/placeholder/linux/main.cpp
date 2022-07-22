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

#include "AppMain.h"
#include "AppOptions.h"
#include "MatterCallbacks.h"

void ApplicationInit() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, AppOptions::GetOptions()) == 0);

    auto test = GetTargetTest();
    if (test != nullptr)
    {
        test->NextTest();
    }

    LinuxDeviceOptions::GetInstance().dacProvider = AppOptions::GetDACProvider();
    ChipLinuxAppMainLoop();
    return 0;
}
