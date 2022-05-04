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

constexpr size_t kCommandMaxLen    = 256;
constexpr const char * kDefaultKey = "default";

CHIP_ERROR SystemCommands::Start(const char * identity, const chip::app::Clusters::SystemCommands::Commands::Start::Type & value)
{
    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "Start.py";

    char command[kCommandMaxLen];
    chip::StringBuilderBase builder(command, sizeof(command));
    builder.Add(scriptDir);
    builder.Add(scriptName);
    builder.Add(" ");

    if (value.registerKey.HasValue())
    {
        VerifyOrReturnError(value.registerKey.Value().size() < 128, CHIP_ERROR_INVALID_ARGUMENT);
        char registerKey[128];
        memset(registerKey, '\0', sizeof(registerKey));
        strncpy(registerKey, value.registerKey.Value().data(), value.registerKey.Value().size());
        builder.Add(registerKey);
    }
    else
    {
        builder.Add(kDefaultKey);
    }

    // Add any applicable optional command line options
    if (value.discriminator.HasValue())
    {
        builder.Add(" --discriminator ");
        builder.Add(value.discriminator.Value());
    }

    if (value.port.HasValue())
    {
        builder.Add(" --secured-device-port ");
        builder.Add(value.port.Value());
    }

    if (value.kvs.HasValue())
    {
        VerifyOrReturnError(value.kvs.Value().size() < 128, CHIP_ERROR_INVALID_ARGUMENT);
        builder.Add(" --KVS ");
        char kvs[128];
        memset(kvs, '\0', sizeof(kvs));
        strncpy(kvs, value.kvs.Value().data(), value.kvs.Value().size());
        builder.Add(kvs);
    }

    if (value.minCommissioningTimeout.HasValue())
    {
        builder.Add(" --min_commissioning_timeout ");
        builder.Add(value.minCommissioningTimeout.Value());
    }

    VerifyOrReturnError(builder.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);

    return RunInternal(command);
}

CHIP_ERROR SystemCommands::Stop(const char * identity, const chip::app::Clusters::SystemCommands::Commands::Stop::Type & value)
{
    constexpr const char * scriptName = "Stop.py";
    return RunInternal(scriptName, value.registerKey);
}

CHIP_ERROR SystemCommands::Reboot(const char * identity, const chip::app::Clusters::SystemCommands::Commands::Reboot::Type & value)
{
    constexpr const char * scriptName = "Reboot.py";
    return RunInternal(scriptName, value.registerKey);
}

CHIP_ERROR SystemCommands::FactoryReset(const char * identity,
                                        const chip::app::Clusters::SystemCommands::Commands::FactoryReset::Type & value)
{
    constexpr const char * scriptName = "FactoryReset.py";
    return RunInternal(scriptName, value.registerKey);
}

CHIP_ERROR SystemCommands::RunInternal(const char * scriptName, const chip::Optional<chip::CharSpan> registerKey)
{
    const char * scriptDir        = getScriptsFolder();
    const char * registerKeyValue = registerKey.HasValue() ? registerKey.Value().data() : kDefaultKey;
    const size_t registerKeyLen   = registerKey.HasValue() ? registerKey.Value().size() : strlen(kDefaultKey);

    char command[kCommandMaxLen];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s %.*s", scriptDir, scriptName, static_cast<int>(registerKeyLen),
                                 registerKeyValue) >= 0,
                        CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR SystemCommands::RunInternal(const char * command)
{
    VerifyOrReturnError(system(command) == 0, CHIP_ERROR_INTERNAL);
    return ContinueOnChipMainThread(CHIP_NO_ERROR);
}
