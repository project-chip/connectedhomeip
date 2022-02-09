/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "SystemCommands.h"

namespace {
const char basePath[] = "./src/app/tests/suites/commands/system/scripts/";
const char * getScriptsFolder()
{
    return basePath;
}
} // namespace

CHIP_ERROR SystemCommands::Start(uint16_t discriminator)
{
    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "Start.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s %u", scriptDir, scriptName, discriminator) >= 0,
                        CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR SystemCommands::Stop()
{
    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "Stop.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s", scriptDir, scriptName) >= 0, CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR SystemCommands::Reboot(uint16_t discriminator)
{
    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "Reboot.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s %u", scriptDir, scriptName, discriminator) >= 0,
                        CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR SystemCommands::FactoryReset()
{
    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "FactoryReset.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s", scriptDir, scriptName) >= 0, CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR SystemCommands::RunInternal(const char * command)
{
    VerifyOrReturnError(system(command) == 0, CHIP_ERROR_INTERNAL);
    return ContinueOnChipMainThread();
}
