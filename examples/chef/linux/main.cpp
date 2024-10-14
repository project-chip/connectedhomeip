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
