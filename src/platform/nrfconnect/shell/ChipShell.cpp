/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>

#include "shell/shell.h"

#define CHIP_SHELL_MAX_BUFFER_SIZE 128

using namespace chip::DeviceLayer;

static int cmd_chip_qrcode(const struct shell * shell, size_t argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    std::string QRCode;

    error = GetQRCode(QRCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    if (error == CHIP_NO_ERROR)
    {
        shell_print(shell, "%s", QRCode.c_str());
    }
    else
    {
        shell_print(shell, "Getting QR code failed with error: %d", error);
    }

    return (error == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int cmd_chip_qrcodeurl(const struct shell * shell, size_t argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    std::string QRCode;

    error = GetQRCode(QRCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    VerifyOrExit(error == CHIP_NO_ERROR, shell_print(shell, "Getting QR code failed with error: %d", error));

    char qrCodeBuffer[CHIP_SHELL_MAX_BUFFER_SIZE];

    error = GetQRCodeUrl(qrCodeBuffer, sizeof(qrCodeBuffer), QRCode);
    VerifyOrExit(error == CHIP_NO_ERROR, shell_print(shell, "Getting QR code url failed with error: %d", error));

    shell_print(shell, "%s", qrCodeBuffer);

exit:
    return (error == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int cmd_chip_setuppincode(const struct shell * shell, size_t argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint32_t setupPinCode;

    error = ConfigurationMgr().GetSetupPinCode(setupPinCode);
    if (error == CHIP_NO_ERROR)
    {
        shell_print(shell, "%d", setupPinCode);
    }
    else
    {
        shell_print(shell, "Getting Setup Pin Code failed with error: %d", error);
    }

    return (error == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int cmd_chip_discriminator(const struct shell * shell, size_t argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint16_t discriminator;

    error = ConfigurationMgr().GetSetupDiscriminator(discriminator);
    if (error == CHIP_NO_ERROR)
    {
        shell_print(shell, "%d", discriminator);
    }
    else
    {
        shell_print(shell, "Getting Setup Discriminator failed with error: %d", error);
    }

    return (error == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int cmd_chip_vendorid(const struct shell * shell, size_t argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint16_t vendorId;

    error = ConfigurationMgr().GetVendorId(vendorId);
    if (error == CHIP_NO_ERROR)
    {
        shell_print(shell, "%d", vendorId);
    }
    else
    {
        shell_print(shell, "Getting Vendor ID failed with error: %d", error);
    }

    return (error == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int cmd_chip_productid(const struct shell * shell, size_t argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint16_t productId;

    error = ConfigurationMgr().GetProductId(productId);
    if (error == CHIP_NO_ERROR)
    {
        shell_print(shell, "%d", productId);
    }
    else
    {
        shell_print(shell, "Getting Product ID failed with error: %d", error);
    }

    return (error == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int cmd_chip_manualpairingcode(const struct shell * shell, size_t argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    std::string manualPairingCode;

    error = GetManualPairingCode(manualPairingCode, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    if (error == CHIP_NO_ERROR)
    {
        shell_print(shell, "%s", manualPairingCode.c_str());
    }
    else
    {
        shell_print(shell, "Getting Manual Pairing Code failed with error: %d", error);
    }

    return (error == CHIP_NO_ERROR) ? 0 : -ENOEXEC;
}

static int cmd_chip_bleadvertising(const struct shell * shell, size_t argc, char ** argv)
{
    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        shell_print(shell, "1");
    }
    else
    {
        shell_print(shell, "0");
    }

    return 0;
}

static int cmd_chip_nfcemulation(const struct shell * shell, size_t argc, char ** argv)
{
    if (NFCMgr().IsTagEmulationStarted())
    {
        shell_print(shell, "1");
    }
    else
    {
        shell_print(shell, "0");
    }

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(chipCommands, SHELL_CMD_ARG(qrcode, NULL, "qrcode", cmd_chip_qrcode, 0, 0),
                               SHELL_CMD_ARG(qrcodeurl, NULL, "qrcodeurl", cmd_chip_qrcodeurl, 0, 0),
                               SHELL_CMD_ARG(setuppincode, NULL, "setuppincode", cmd_chip_setuppincode, 0, 0),
                               SHELL_CMD_ARG(discriminator, NULL, "discriminator", cmd_chip_discriminator, 0, 0),
                               SHELL_CMD_ARG(vendorid, NULL, "vendorid", cmd_chip_vendorid, 0, 0),
                               SHELL_CMD_ARG(productid, NULL, "productid", cmd_chip_productid, 0, 0),
                               SHELL_CMD_ARG(manualpairingcode, NULL, "manualpairingcode", cmd_chip_manualpairingcode, 0, 0),
                               SHELL_CMD_ARG(bleadvertising, NULL, "bleadvertising", cmd_chip_bleadvertising, 0, 0),
                               SHELL_CMD_ARG(nfcemulation, NULL, "nfcemulation", cmd_chip_nfcemulation, 0, 0),
                               SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(chip, &chipCommands, "CHIP commands", NULL);
