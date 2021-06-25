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

#include <core/CHIPCore.h>
#include <lib/shell/Commands.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif
#include <app/server/OnboardingCodesUtil.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

using chip::DeviceLayer::ConnectivityMgr;

namespace chip {
namespace Shell {

static int NFCHandler(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    bool nfcEnabled;

    VerifyOrReturnError(argc == 1, error = CHIP_ERROR_INVALID_ARGUMENT);

    nfcEnabled = chip::DeviceLayer::NFCMgr().IsTagEmulationStarted();

    if (strcmp(argv[0], "start") == 0)
    {
        if (nfcEnabled)
        {
            streamer_printf(sout, "NFC tag emulation is already enabled\r\n");
        }
        else
        {
            ShareQRCodeOverNFC(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
            streamer_printf(sout, "NFC tag emulation started\r\n");
        }
    }
    else if (strcmp(argv[0], "stop") == 0)
    {
        if (nfcEnabled)
        {
            chip::DeviceLayer::NFCMgr().StopTagEmulation();
            streamer_printf(sout, "NFC tag emulation stopped\r\n");
        }
        else
        {
            streamer_printf(sout, "NFC tag emulation is already disabled\r\n");
        }
    }
    else if (strcmp(argv[0], "state") == 0)
    {
        if (nfcEnabled)
        {
            streamer_printf(sout, "NFC tag emulation is enabled\r\n");
        }
        else
        {
            streamer_printf(sout, "NFC tag emulation is disabled\r\n");
        }
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

void RegisterNFCCommands()
{
    static const shell_command_t sDeviceComand = { &NFCHandler, "nfc",
                                                   "Start, stop or get nfc emulation state. Usage: nfc <start|stop|state>" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

} // namespace Shell
} // namespace chip
