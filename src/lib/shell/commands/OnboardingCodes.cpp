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

#include <app/server/OnboardingCodesUtil.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#define CHIP_SHELL_MAX_BUFFER_SIZE 128

using chip::DeviceLayer::ConfigurationMgr;

namespace chip {
namespace Shell {

static CHIP_ERROR GetOnboardingQRCode(bool printHeader)
{
    streamer_t * sout = streamer_get();
    std::string QRCode;

    if (printHeader)
    {
        streamer_printf(sout, "QRCode:            ");
    }
    ReturnErrorOnFailure(GetQRCode(QRCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)));
    streamer_printf(sout, "%s\r\n", QRCode.c_str());
    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetOnboardingQRCodeUrl(bool printHeader)
{
    streamer_t * sout = streamer_get();
    std::string QRCode;

    if (printHeader)
    {
        streamer_printf(sout, "QRCodeUrl:          ");
    }
    ReturnErrorOnFailure(GetQRCode(QRCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)));

    char qrCodeBuffer[CHIP_SHELL_MAX_BUFFER_SIZE];

    ReturnErrorOnFailure(GetQRCodeUrl(qrCodeBuffer, sizeof(qrCodeBuffer), QRCode));
    streamer_printf(sout, "%s\r\n", qrCodeBuffer);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetOnboardingManualPairingCode(bool printHeader)
{
    streamer_t * sout = streamer_get();
    std::string manualPairingCode;

    if (printHeader)
    {
        streamer_printf(sout, "ManualPairingCode:  ");
    }
    ReturnErrorOnFailure(
        GetManualPairingCode(manualPairingCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE)));
    streamer_printf(sout, "%s\r\n", manualPairingCode.c_str());
    return CHIP_NO_ERROR;
}

static int PrintAllOnboardingCodes()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    error |= GetOnboardingQRCode(true);
    error |= GetOnboardingQRCodeUrl(true);
    error |= GetOnboardingManualPairingCode(true);

    return error;
}

static CHIP_ERROR OnboardingHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return PrintAllOnboardingCodes();
    }

    if (strcmp(argv[0], "qrcode") == 0)
    {
        return GetOnboardingQRCode(false);
    }
    else if (strcmp(argv[0], "qrcodeurl") == 0)
    {
        return GetOnboardingQRCodeUrl(false);
    }
    else if (strcmp(argv[0], "manualpairingcode") == 0)
    {
        return GetOnboardingManualPairingCode(false);
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void RegisterOnboardingCodesCommands()
{

    static const shell_command_t sDeviceComand = { &OnboardingHandler, "onboardingcodes",
                                                   "Dump device onboarding codes. Usage: onboardingcodes [param_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
