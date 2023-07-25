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
#include "AppTv.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/util/af.h>

#if defined(ENABLE_CHIP_SHELL)
#include "AppTvShellCommands.h"
#endif

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;

void ApplicationInit()
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationInit()");

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    ChipLogDetail(DeviceLayer, "TV Linux App: Warning - Fixed Content App Endpoint Not Disabled");
    // Can't disable this without breaking CI unit tests that act upon account login cluster (only available on ep3)
    // emberAfEndpointEnableDisable(3, false);
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    AppTvInit();

#if defined(ENABLE_CHIP_SHELL)
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    Shell::RegisterAppTvCommands();
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#endif

    ChipLinuxAppMainLoop();

    return 0;
}
