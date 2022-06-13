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
constexpr size_t kArgumentMaxLen   = 128;
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
        AddSystemCommandArgument(builder, "--KVS", value.kvs.Value());
    }

    if (value.minCommissioningTimeout.HasValue())
    {
        builder.Add(" --min_commissioning_timeout ");
        builder.Add(value.minCommissioningTimeout.Value());
    }

    // OTA provider specific arguments
    if (value.filepath.HasValue())
    {
        AddSystemCommandArgument(builder, "--filepath", value.filepath.Value());
    }

    // OTA requstor specific arguments
    if (value.otaDownloadPath.HasValue())
    {
        AddSystemCommandArgument(builder, "--otaDownloadPath", value.otaDownloadPath.Value());
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

CHIP_ERROR SystemCommands::CreateOtaImage(const char * identity,
                                          const chip::app::Clusters::SystemCommands::Commands::CreateOtaImage::Type & value)
{
    VerifyOrReturnError(!value.rawImageContent.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!value.rawImageFilePath.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!value.otaImageFilePath.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "CreateOtaImage.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s %.*s %.*s %.*s", scriptDir, scriptName,
                                 static_cast<int>(value.otaImageFilePath.size()), value.otaImageFilePath.data(),
                                 static_cast<int>(value.rawImageFilePath.size()), value.rawImageFilePath.data(),
                                 static_cast<int>(value.rawImageContent.size()), value.rawImageContent.data()) >= 0,
                        CHIP_ERROR_INTERNAL);
    return RunInternal(command);
}

CHIP_ERROR SystemCommands::CompareFiles(const char * identity,
                                        const chip::app::Clusters::SystemCommands::Commands::CompareFiles::Type & value)
{
    VerifyOrReturnError(!value.file1.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!value.file2.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    const char * scriptDir            = getScriptsFolder();
    constexpr const char * scriptName = "CompareFiles.py";

    char command[128];
    VerifyOrReturnError(snprintf(command, sizeof(command), "%s%s %.*s %.*s", scriptDir, scriptName,
                                 static_cast<int>(value.file1.size()), value.file1.data(), static_cast<int>(value.file2.size()),
                                 value.file2.data()) >= 0,
                        CHIP_ERROR_INTERNAL);
    return RunInternal(command);
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

CHIP_ERROR SystemCommands::AddSystemCommandArgument(chip::StringBuilderBase & builder, const char * argName,
                                                    const chip::CharSpan & argValue)
{
    VerifyOrReturnError(argValue.size() < kArgumentMaxLen, CHIP_ERROR_INVALID_ARGUMENT);

    builder.Add(" ");
    builder.Add(argName);
    builder.Add(" ");

    char arg[kArgumentMaxLen];
    memset(arg, 0, sizeof(arg));
    strncpy(arg, argValue.data(), argValue.size());
    builder.Add(arg);

    return CHIP_NO_ERROR;
}
