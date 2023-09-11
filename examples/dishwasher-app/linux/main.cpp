/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "operational-state-delegate-impl.h"
#include <AppMain.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void ApplicationInit() {}

void ApplicationShutdown()
{
    OperationalState::Shutdown();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
