/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
