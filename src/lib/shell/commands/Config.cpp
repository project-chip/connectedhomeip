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
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

using chip::DeviceLayer::ConfigurationMgr;

namespace chip {
namespace Shell {

static chip::Shell::Engine sShellConfigSubcommands;

CHIP_ERROR ConfigHelpHandler(int argc, char ** argv)
{
    sShellConfigSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetVendorId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    ReturnErrorOnFailure(DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(value16));
    if (printHeader)
    {
        streamer_printf(sout, "VendorId:        ");
    }
    streamer_printf(sout, "%u (0x%X)\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigVendorId(int argc, char ** argv)
{
    return ConfigGetVendorId(false);
}

static CHIP_ERROR ConfigGetProductId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    ReturnErrorOnFailure(DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(value16));
    if (printHeader)
    {
        streamer_printf(sout, "ProductId:       ");
    }
    streamer_printf(sout, "%u (0x%X)\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigProductId(int argc, char ** argv)
{
    return ConfigGetProductId(false);
}

static CHIP_ERROR ConfigGetHardwareVersion(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    ReturnErrorOnFailure(DeviceLayer::GetDeviceInstanceInfoProvider()->GetHardwareVersion(value16));
    if (printHeader)
    {
        streamer_printf(sout, "HardwareVersion: ");
    }
    streamer_printf(sout, "%u (0x%X)\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigHardwareVersion(int argc, char ** argv)
{
    return ConfigGetHardwareVersion(false);
}

static CHIP_ERROR ConfigGetSetupPinCode(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint32_t setupPinCode;

    ReturnErrorOnFailure(DeviceLayer::GetCommissionableDataProvider()->GetSetupPasscode(setupPinCode));
    if (printHeader)
    {
        streamer_printf(sout, "PinCode:         ");
    }
    streamer_printf(sout, "%08u\r\n", setupPinCode);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigPinCode(int argc, char ** argv)
{
    return ConfigGetSetupPinCode(false);
}

static CHIP_ERROR ConfigGetSetupDiscriminator(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t setupDiscriminator;

    ReturnErrorOnFailure(DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(setupDiscriminator));
    if (printHeader)
    {
        streamer_printf(sout, "Discriminator:   ");
    }
    streamer_printf(sout, "%03x\r\n", setupDiscriminator & chip::kMaxDiscriminatorValue);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigSetSetupDiscriminator(char * argv)
{
    CHIP_ERROR error;
    streamer_t * sout           = streamer_get();
    uint16_t setupDiscriminator = strtoull(argv, nullptr, 10);

    VerifyOrReturnError(setupDiscriminator != 0 && setupDiscriminator < chip::kMaxDiscriminatorValue, CHIP_ERROR_INVALID_ARGUMENT);

    error = DeviceLayer::GetCommissionableDataProvider()->SetSetupDiscriminator(setupDiscriminator);

    if (error == CHIP_NO_ERROR)
    {
        streamer_printf(sout, "Setup discriminator set to: %d\r\n", setupDiscriminator);
    }
    else
    {
        streamer_printf(sout, "Setup discriminator setting failed with code: %d\r\n", error);
    }

    return error;
}

static CHIP_ERROR ConfigDiscriminator(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ConfigGetSetupDiscriminator(false);
    }

    return ConfigSetSetupDiscriminator(argv[0]);
}

static CHIP_ERROR PrintAllConfigs()
{
    ReturnErrorOnFailure(ConfigGetVendorId(true));
    ReturnErrorOnFailure(ConfigGetProductId(true));
    ReturnErrorOnFailure(ConfigGetHardwareVersion(true));

    ReturnErrorOnFailure(ConfigGetSetupPinCode(true));
    ReturnErrorOnFailure(ConfigGetSetupDiscriminator(true));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigHandler(int argc, char ** argv)
{
    switch (argc)
    {
    case 0:
        return PrintAllConfigs();
    case 1:
        if ((strcmp(argv[0], "help") == 0) || (strcmp(argv[0], "-h") == 0))
        {
            return ConfigHelpHandler(argc, argv);
        }
    }
    return sShellConfigSubcommands.ExecCommand(argc, argv);
}

void RegisterConfigCommands()
{

    static const shell_command_t sConfigComand = { &ConfigHandler, "config",
                                                   "Manage device configuration. Usage to dump value: config [param_name] and "
                                                   "to set some values (discriminator): config [param_name] [param_value]." };

    static const shell_command_t sConfigSubCommands[] = {
        { &ConfigHelpHandler, "help", "Usage: config <subcommand>" },
        { &ConfigVendorId, "vendorid", "Get VendorId. Usage: config vendorid" },
        { &ConfigProductId, "productid", "Get ProductId. Usage: config productid" },
        { &ConfigHardwareVersion, "hardwarever", "Get HardwareVersion. Usage: config hardwarever" },
        { &ConfigPinCode, "pincode", "Get commissioning pincode. Usage: config pincode" },
        { &ConfigDiscriminator, "discriminator", "Get/Set commissioning discriminator. Usage: config discriminator [value]" },
    };

    // Register `config` subcommands with the local shell dispatcher.
    sShellConfigSubcommands.RegisterCommands(sConfigSubCommands, ArraySize(sConfigSubCommands));

    // Register the root `config` command with the top-level shell.
    Engine::Root().RegisterCommands(&sConfigComand, 1);
}

} // namespace Shell
} // namespace chip
