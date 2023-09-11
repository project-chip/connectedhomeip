/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

constexpr const char * kDefaultKey = "default";

CHIP_ERROR DelayCommands::WaitForMs(const char * identity,
                                    const chip::app::Clusters::DelayCommands::Commands::WaitForMs::Type & value)
{
    const auto duration = chip::System::Clock::Milliseconds32(value.ms);
    return chip::DeviceLayer::SystemLayer().StartTimer(duration, OnWaitForMsFn, this);
}

void DelayCommands::OnWaitForMsFn(chip::System::Layer * systemLayer, void * context)
{
    auto * command = static_cast<DelayCommands *>(context);
    command->OnWaitForMs();
}

CHIP_ERROR DelayCommands::WaitForMessage(const char * identity,
                                         const chip::app::Clusters::DelayCommands::Commands::WaitForMessage::Type & value)
{
    VerifyOrReturnError(!value.message.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "WaitForMessage.py";
    const char * registerKeyValue     = value.registerKey.HasValue() ? value.registerKey.Value().data() : kDefaultKey;
    const size_t registerKeyLen       = value.registerKey.HasValue() ? value.registerKey.Value().size() : strlen(kDefaultKey);

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s %.*s %.*s", scriptDir, scriptName,
                                 static_cast<int>(registerKeyLen), registerKeyValue, static_cast<int>(value.message.size()),
                                 value.message.data()) >= 0,
                        CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR DelayCommands::RunInternal(const char * command)
{
    VerifyOrReturnError(system(command) == 0, CHIP_ERROR_INTERNAL);
    return ContinueOnChipMainThread(CHIP_NO_ERROR);
}

CHIP_ERROR DelayCommands::BusyWaitFor(chip::System::Clock::Milliseconds32 durationInMs)
{
    auto & clock = chip::System::SystemClock();
    auto start   = clock.GetMonotonicTimestamp();
    while (clock.GetMonotonicTimestamp() - start < durationInMs)
    {
        // nothing to do.
    };
    return CHIP_NO_ERROR;
}
