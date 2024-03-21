/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/core/DataModelTypes.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::DeviceLayer;

namespace chip {
namespace Shell {
namespace {

Shell::Engine sSubShell;

CHIP_ERROR QueryImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);

    return GetRequestorInstance()->TriggerImmediateQuery();
}

const char * UpdateStateToString(app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum state)
{
    switch (state)
    {
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kIdle:
        return "idle";
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kQuerying:
        return "querying";
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnQuery:
        return "delayed on query";
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDownloading:
        return "downloading";
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kApplying:
        return "applying";
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnApply:
        return "delayed on apply";
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kRollingBack:
        return "rolling back";
    case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnUserConsent:
        return "delayed on user consent";
    default:
        return "unknown";
    }
}

CHIP_ERROR StateHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);

    app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum state;
    ReturnErrorOnFailure(GetRequestorInstance()->GetUpdateStateAttribute(kRootEndpointId, state));

    streamer_printf(streamer_get(), "Update state: %s\r\n", UpdateStateToString(state));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ProgressHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);

    app::DataModel::Nullable<uint8_t> progress;
    ReturnErrorOnFailure(GetRequestorInstance()->GetUpdateStateProgressAttribute(kRootEndpointId, progress));

    if (progress.IsNull())
    {
        streamer_printf(streamer_get(), "Update progress: unknown\r\n");
    }
    else
    {
        streamer_printf(streamer_get(), "Update progress: %d %%\r\n", progress.Value());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    return sSubShell.ExecCommand(argc, argv);
}
} // namespace

void RegisterOtaCommands()
{
    // Register subcommands of the `ota` commands.
    static const shell_command_t subCommands[] = {
        { &QueryImageHandler, "query", "Query for a new image. Usage: ota query" },
        { &StateHandler, "state", "Gets state of a current image update process. Usage: ota state" },
        { &ProgressHandler, "progress", "Gets progress of a current image update process. Usage: ota progress" }
    };

    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    // Register the root `ota` command in the top-level shell.
    static const shell_command_t otaCommand = { &OtaHandler, "ota", "OTA commands" };

    Engine::Root().RegisterCommands(&otaCommand, 1);
}

} // namespace Shell
} // namespace chip
