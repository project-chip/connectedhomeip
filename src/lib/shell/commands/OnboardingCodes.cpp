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

#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#define CHIP_SHELL_MAX_BUFFER_SIZE 128

using chip::DeviceLayer::ConfigurationMgr;

namespace chip {
namespace Shell {

static CHIP_ERROR GetOnboardingQRCode(bool printHeader, chip::RendezvousInformationFlags aRendezvousFlags)
{
    streamer_t * sout = streamer_get();

    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan QRCode(qrCodeBuffer);

    if (printHeader)
    {
        streamer_printf(sout, "QRCode:            ");
    }
    ReturnErrorOnFailure(GetQRCode(QRCode, aRendezvousFlags));
    streamer_printf(sout, "%s\r\n", QRCode.data());
    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetOnboardingQRCodeUrl(bool printHeader, chip::RendezvousInformationFlags aRendezvousFlags)
{
    streamer_t * sout = streamer_get();

    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan QRCode(qrCodeBuffer);

    if (printHeader)
    {
        streamer_printf(sout, "QRCodeUrl:         ");
    }
    ReturnErrorOnFailure(GetQRCode(QRCode, aRendezvousFlags));

    char qrCodeUrlBuffer[CHIP_SHELL_MAX_BUFFER_SIZE];

    ReturnErrorOnFailure(GetQRCodeUrl(qrCodeUrlBuffer, sizeof(qrCodeUrlBuffer), QRCode));
    streamer_printf(sout, "%s\r\n", qrCodeUrlBuffer);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetOnboardingManualPairingCode(bool printHeader, chip::RendezvousInformationFlags aRendezvousFlags)
{
    streamer_t * sout = streamer_get();

    // Create buffer for manual pariting code that can fit max size + check digit + null terminator.
    char manualPairingCodeBuffer[chip::kManualSetupLongCodeCharLength + 1];
    chip::MutableCharSpan manualPairingCode(manualPairingCodeBuffer);

    if (printHeader)
    {
        streamer_printf(sout, "ManualPairingCode: ");
    }
    ReturnErrorOnFailure(GetManualPairingCode(manualPairingCode, aRendezvousFlags));
    streamer_printf(sout, "%s\r\n", manualPairingCode.data());
    return CHIP_NO_ERROR;
}

static CHIP_ERROR PrintAllOnboardingCodes(chip::RendezvousInformationFlags aRendezvousFlags)
{
    ReturnErrorOnFailure(GetOnboardingQRCode(true, aRendezvousFlags));
    ReturnErrorOnFailure(GetOnboardingQRCodeUrl(true, aRendezvousFlags));
    ReturnErrorOnFailure(GetOnboardingManualPairingCode(true, aRendezvousFlags));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR RendezvousStringToFlag(char * str, chip::RendezvousInformationFlags * aRendezvousFlags)
{
    if (str == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (aRendezvousFlags == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    if (strcmp(str, "none") == 0)
    {
        *aRendezvousFlags = chip::RendezvousInformationFlag::kNone;
        return CHIP_NO_ERROR;
    }
    if (strcmp(str, "softap") == 0)
    {
        *aRendezvousFlags = chip::RendezvousInformationFlag::kSoftAP;
        return CHIP_NO_ERROR;
    }
    if (strcmp(str, "ble") == 0)
    {
        *aRendezvousFlags = chip::RendezvousInformationFlag::kBLE;
        return CHIP_NO_ERROR;
    }
    if (strcmp(str, "onnetwork") == 0)
    {
        *aRendezvousFlags = chip::RendezvousInformationFlag::kOnNetwork;
        return CHIP_NO_ERROR;
    }
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    if (strcmp(str, "wifipaf") == 0)
    {
        *aRendezvousFlags = chip::RendezvousInformationFlag::kWiFiPAF;
        return CHIP_NO_ERROR;
    }
#endif
    return CHIP_ERROR_INVALID_ARGUMENT;
}

static CHIP_ERROR OnboardingHandler(int argc, char ** argv)
{
    chip::RendezvousInformationFlags aRendezvousFlags;

    if (argc == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = RendezvousStringToFlag(argv[0], &aRendezvousFlags);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    if (argc == 1)
    {
        return PrintAllOnboardingCodes(aRendezvousFlags);
    }

    if (strcmp(argv[1], "qrcode") == 0)
    {
        return GetOnboardingQRCode(false, aRendezvousFlags);
    }
    if (strcmp(argv[1], "qrcodeurl") == 0)
    {
        return GetOnboardingQRCodeUrl(false, aRendezvousFlags);
    }
    if (strcmp(argv[1], "manualpairingcode") == 0)
    {
        return GetOnboardingManualPairingCode(false, aRendezvousFlags);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

void RegisterOnboardingCodesCommands()
{
    static constexpr Command deviceComand = {
        &OnboardingHandler, "onboardingcodes",
        "Dump device onboarding codes. Usage: onboardingcodes none|softap|ble|onnetwork [qrcode|qrcodeurl|manualpairingcode]"
    };

    Engine::Root().RegisterCommands(&deviceComand, 1);
}

} // namespace Shell
} // namespace chip
