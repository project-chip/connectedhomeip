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

#include "DelayCommands.h"

namespace {
const char basePath[] = "./src/app/tests/suites/commands/delay/scripts/";
const char * getScriptsFolder()
{
    return basePath;
}
} // namespace

CHIP_ERROR DelayCommands::WaitForMs(uint16_t ms)
{
    const auto duration = chip::System::Clock::Milliseconds32(ms);
    return chip::DeviceLayer::SystemLayer().StartTimer(duration, OnWaitForMsFn, this);
}

void DelayCommands::OnWaitForMsFn(chip::System::Layer * systemLayer, void * context)
{
    auto * command = static_cast<DelayCommands *>(context);
    command->OnWaitForMs();
}

CHIP_ERROR DelayCommands::WaitForCommissionableAdvertisement()
{
    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "WaitForCommissionableAdvertisement.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s", scriptDir, scriptName) >= 0, CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR DelayCommands::WaitForOperationalAdvertisement()
{
    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "WaitForOperationalAdvertisement.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s", scriptDir, scriptName) >= 0, CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR DelayCommands::RunInternal(const char * command)
{
    VerifyOrReturnError(system(command) == 0, CHIP_ERROR_INTERNAL);
    return ContinueOnChipMainThread(CHIP_NO_ERROR);
}
