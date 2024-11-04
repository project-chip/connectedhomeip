/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <admin/PairingManager.h>

#if defined(ENABLE_CHIP_SHELL)
#include "ShellCommands.h"
#endif

using namespace chip;

void ApplicationInit()
{
    ChipLogProgress(NotSpecified, "Fabric-Sync: ApplicationInit()");
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "Fabric-Sync: ApplicationShutdown()");
}

int main(int argc, char * argv[])
{

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

#if defined(ENABLE_CHIP_SHELL)
    Shell::RegisterCommands();
#endif

    CHIP_ERROR err = PairingManager::Instance().Init(GetDeviceCommissioner());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Failed to init PairingManager: %s ", ErrorStr(err));

        // End the program with non zero error code to indicate a error.
        return 1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
