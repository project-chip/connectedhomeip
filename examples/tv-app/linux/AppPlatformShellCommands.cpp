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

/**
 * @file Contains shell commands for a ContentApp relating to Content App platform of the Video Player.
 */

#include "AppPlatformShellCommands.h"
#include "ControllerShellCommands.h"
#include <AppMain.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace ::chip::Controller;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::app::Clusters;

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

namespace chip {
namespace Shell {

static CHIP_ERROR pairApp(bool printHeader, size_t index)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "udc-commission %ld\r\n", index);
    }

    DeviceCommissioner * commissioner = GetDeviceCommissioner();
    UDCClientState * state = commissioner->GetUserDirectedCommissioningServer()->GetUDCClients().GetUDCClientState(index);
    if (state == nullptr)
    {
        streamer_printf(sout, "udc client[%d] null \r\n", index);
    }
    else
    {
        ContentApp * app = ContentAppPlatform::GetInstance().LoadContentAppByClient(state->GetVendorId(), state->GetProductId());
        if (app == nullptr)
        {
            streamer_printf(sout, "no app found for vendor id=%d \r\n", state->GetVendorId());
            return CHIP_ERROR_BAD_REQUEST;
        }

        if (app->GetAccountLoginDelegate() == nullptr)
        {
            streamer_printf(sout, "no AccountLogin cluster for app with vendor id=%d \r\n", state->GetVendorId());
            return CHIP_ERROR_BAD_REQUEST;
        }

        char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(state->GetRotatingId(), state->GetRotatingIdLength(), rotatingIdString,
                                            sizeof(rotatingIdString));

        CharSpan rotatingIdSpan = CharSpan(rotatingIdString, strlen(rotatingIdString));

        static const size_t kSetupPinSize = 12;
        char setupPin[kSetupPinSize];

        app->GetAccountLoginDelegate()->GetSetupPin(setupPin, kSetupPinSize, rotatingIdSpan);
        std::string pinString(setupPin);

        char * eptr;
        uint32_t pincode = (uint32_t) strtol(pinString.c_str(), &eptr, 10);
        if (pincode == 0)
        {
            streamer_printf(sout, "udc no pin returned for vendor id=%d rotating ID=%s \r\n", state->GetVendorId(),
                            rotatingIdString);
            return CHIP_ERROR_BAD_REQUEST;
        }

        return CommissionerPairUDC(pincode, index);
    }
    return CHIP_NO_ERROR;
}

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                 Usage: app <subcommand>\r\n");
    streamer_printf(sout, "  add <vid> [<pid>]    Add app with given vendor ID [1, 2, 9050]. Usage: app add 9050\r\n");
    streamer_printf(sout, "  remove <endpoint>    Remove app at given endpoint [6, 7, etc]. Usage: app remove 6\r\n");
    streamer_printf(sout,
                    "  setpin <endpoint> <pincode>  Set pincode for app with given endpoint ID. Usage: app setpin 6 34567890\r\n");
    streamer_printf(sout,
                    "  commission <udc-entry>     Commission given udc-entry using given pincode from corresponding app. Usage: "
                    "app commission 0\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR AppPlatformHandler(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
    else if (strcmp(argv[0], "add") == 0)
    {
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;

        uint16_t vid = (uint16_t) strtol(argv[1], &eptr, 10);
        uint16_t pid = 0;
        if (argc >= 3)
        {
            pid = (uint16_t) strtol(argv[2], &eptr, 10);
        }
        ContentAppPlatform::GetInstance().LoadContentAppByClient(vid, pid);

        ChipLogProgress(DeviceLayer, "added app");

        return CHIP_NO_ERROR;
    }
    else if (strcmp(argv[0], "remove") == 0)
    {
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;

        uint16_t endpoint = (uint16_t) strtol(argv[1], &eptr, 10);
        ContentApp * app  = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
        if (app == nullptr)
        {
            ChipLogProgress(DeviceLayer, "app not found");
            return CHIP_ERROR_BAD_REQUEST;
        }
        ContentAppPlatform::GetInstance().RemoveContentApp(app);

        ChipLogProgress(DeviceLayer, "removed app");

        return CHIP_NO_ERROR;
    }
    else if (strcmp(argv[0], "setpin") == 0)
    {
        if (argc < 3)
        {
            return PrintAllCommands();
        }
        char * eptr;

        uint16_t endpoint = (uint16_t) strtol(argv[1], &eptr, 10);
        char * pincode    = argv[2];
        ContentApp * app  = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
        if (app == nullptr)
        {
            ChipLogProgress(DeviceLayer, "app not found");
            return CHIP_ERROR_BAD_REQUEST;
        }
        if (app->GetAccountLoginDelegate() == nullptr)
        {
            ChipLogProgress(DeviceLayer, "no AccountLogin cluster for app with endpoint id=%d ", endpoint);
            return CHIP_ERROR_BAD_REQUEST;
        }
        app->GetAccountLoginDelegate()->SetSetupPin(pincode);

        ChipLogProgress(DeviceLayer, "set pin success");

        return CHIP_NO_ERROR;
    }
    else if (strcmp(argv[0], "commission") == 0)
    {
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        size_t index = (size_t) strtol(argv[1], &eptr, 10);
        return error = pairApp(true, index);
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

void RegisterAppPlatformCommands()
{

    static const shell_command_t sDeviceComand = { &AppPlatformHandler, "app", "App commands. Usage: app [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
