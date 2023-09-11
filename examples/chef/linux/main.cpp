/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <AppMain.h>

#include <ChipShellCollection.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteAttributePath.h>
#include <lib/shell/Engine.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::app;
using namespace chip::app::Clusters;

void ApplicationInit() {}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    const int rc = Engine::Root().Init();
    if (rc != 0)
    {
        ChipLogError(Shell, "Streamer initialization failed: %d", rc);
        return rc;
    }

    cmd_misc_init();
    cmd_otcli_init();
#if CHIP_SHELL_ENABLE_CMD_SERVER
    cmd_app_server_init();
#endif

    ChipLinuxAppMainLoop();

    return 0;
}
