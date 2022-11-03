/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppMain.h"
#include "AppOptions.h"
#include "binding-handler.h"
#include "main-common.h"

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, AppOptions::GetOptions()) == 0);
    VerifyOrDie(InitBindingHandlers() == CHIP_NO_ERROR);

    LinuxDeviceOptions::GetInstance().dacProvider = AppOptions::GetDACProvider();

    ChipLinuxAppMainLoop();
    ApplicationExit();

    return 0;
}
