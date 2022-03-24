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
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

using namespace chip::DeviceLayer;

namespace chip {
namespace Shell {
namespace {

Shell::Engine sSubShell;

CHIP_ERROR QueryImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->TriggerImmediateQuery(); });
    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplyImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->ApplyUpdate(); });
    return CHIP_NO_ERROR;
}

CHIP_ERROR NotifyImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);

    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->NotifyUpdateApplied(); });
    return CHIP_NO_ERROR;
}

static void HandleState(intptr_t context)
{
    app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum state;
    CHIP_ERROR err = GetRequestorInstance()->GetUpdateStateAttribute(0, state);

    if (err == CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Update state: ");
        switch (state)
        {
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kUnknown:
            streamer_printf(streamer_get(), "unknown");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kIdle:
            streamer_printf(streamer_get(), "idle");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kQuerying:
            streamer_printf(streamer_get(), "querying");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnQuery:
            streamer_printf(streamer_get(), "delayed on query");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDownloading:
            streamer_printf(streamer_get(), "downloading");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kApplying:
            streamer_printf(streamer_get(), "applying");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnApply:
            streamer_printf(streamer_get(), "delayed on apply");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kRollingBack:
            streamer_printf(streamer_get(), "rolling back");
            break;
        case app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kDelayedOnUserConsent:
            streamer_printf(streamer_get(), "delayed on user consent");
            break;
        default:
            streamer_printf(streamer_get(), "invalid");
            break;
        }
        streamer_printf(streamer_get(), "\r\n");
    }
    else
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", err.Format());
    }
}

static void HandleProgress(intptr_t context)
{
    chip::app::DataModel::Nullable<uint8_t> progress;
    CHIP_ERROR err = GetRequestorInstance()->GetUpdateStateProgressAttribute(0, progress);

    if (err == CHIP_NO_ERROR)
    {
        if (progress.IsNull())
        {
            streamer_printf(streamer_get(), "Update progress: NULL\r\n");
        }
        else
        {
            streamer_printf(streamer_get(), "Update progress: %d %%\r\n", progress.Value());
        }
    }
    else
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", err.Format());
    }
}

CHIP_ERROR StateHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);

    PlatformMgr().ScheduleWork(HandleState, reinterpret_cast<intptr_t>(nullptr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ProgressHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);

    PlatformMgr().ScheduleWork(HandleProgress, reinterpret_cast<intptr_t>(nullptr));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR error = sSubShell.ExecCommand(argc, argv);

    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
    }

    return error;
}
} // namespace

void RegisterOtaCommands()
{
    // Register subcommands of the `ota` commands.
    static const shell_command_t subCommands[] = {
        { &QueryImageHandler, "query", "Query for a new image. Usage: ota query" },
        { &ApplyImageHandler, "apply", "Apply the current update. Usage: ota apply" },
        { &NotifyImageHandler, "notify", "Notify the new image has been applied. Usage: ota notify <version>" },
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
