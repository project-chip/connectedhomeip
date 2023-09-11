/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppMain.h"
#include "AppOptions.h"
#include "MatterCallbacks.h"

void ApplicationInit() {}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, AppOptions::GetOptions()) == 0);

    auto test = GetTargetTest();
    if (test != nullptr)
    {
        test->NextTest();
    }

    LinuxDeviceOptions::GetInstance().dacProvider = AppOptions::GetDACProvider();

    auto & server = InteractiveServer::GetInstance();
    if (AppOptions::GetInteractiveMode())
    {
        server.Run(AppOptions::GetInteractiveModePort());
    }

    ChipLinuxAppMainLoop();

    if (test != nullptr)
    {
        return test->GetCommandExitCode();
    }

    return 0;
}
