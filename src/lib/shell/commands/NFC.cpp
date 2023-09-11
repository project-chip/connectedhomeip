/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif
#include <app/server/OnboardingCodesUtil.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

using chip::DeviceLayer::ConnectivityMgr;

namespace chip {
namespace Shell {

static CHIP_ERROR NFCHandler(int argc, char ** argv)
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
