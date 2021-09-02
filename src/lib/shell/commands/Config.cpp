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

using chip::DeviceLayer::ConfigurationMgr;

namespace chip {
namespace Shell {

static CHIP_ERROR ConfigGetVendorId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    ReturnErrorOnFailure(ConfigurationMgr().GetVendorId(value16));
    if (printHeader)
    {
        streamer_printf(sout, "VendorId:        ");
    }
    streamer_printf(sout, "%" PRIu16 " (0x%" PRIX16 ")\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetProductId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    ReturnErrorOnFailure(ConfigurationMgr().GetProductId(value16));
    if (printHeader)
    {
        streamer_printf(sout, "ProductId:       ");
    }
    streamer_printf(sout, "%" PRIu16 " (0x%" PRIX16 ")\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetProductRevision(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    ReturnErrorOnFailure(ConfigurationMgr().GetProductRevision(value16));
    if (printHeader)
    {
        streamer_printf(sout, "ProductRevision: ");
    }
    streamer_printf(sout, "%" PRIu16 " (0x%" PRIX16 ")\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetDeviceId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint64_t value64;

    ReturnErrorOnFailure(ConfigurationMgr().GetDeviceId(value64));
    if (printHeader)
    {
        streamer_printf(sout, "DeviceId:        ");
    }
    streamer_printf(sout, "%" PRIu64 " (0x" ChipLogFormatX64 ")\r\n", value64, ChipLogValueX64(value64));
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetSetupPinCode(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint32_t setupPinCode;

    ReturnErrorOnFailure(ConfigurationMgr().GetSetupPinCode(setupPinCode));
    if (printHeader)
    {
        streamer_printf(sout, "PinCode:         ");
    }
    streamer_printf(sout, "%08u\r\n", setupPinCode);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetSetupDiscriminator(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t setupDiscriminator;

    ReturnErrorOnFailure(ConfigurationMgr().GetSetupDiscriminator(setupDiscriminator));
    if (printHeader)
    {
        streamer_printf(sout, "Discriminator:   ");
    }
    streamer_printf(sout, "%03x\r\n", setupDiscriminator & 0xFFF);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigSetSetupDiscriminator(char * argv)
{
    CHIP_ERROR error;
    streamer_t * sout           = streamer_get();
    uint16_t setupDiscriminator = strtoull(argv, NULL, 10);

    VerifyOrReturnError(setupDiscriminator != 0 && setupDiscriminator < 0xFFF, CHIP_ERROR_INVALID_ARGUMENT);

    error = ConfigurationMgr().StoreSetupDiscriminator(setupDiscriminator);

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

static CHIP_ERROR ConfigGetFabricId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint64_t value64;

    ReturnErrorOnFailure(ConfigurationMgr().GetFabricId(value64));
    if (printHeader)
    {
        streamer_printf(sout, "FabricId:        ");
    }
    streamer_printf(sout, "%" PRIu64 " (0x" ChipLogFormatX64 ")\r\n", value64, ChipLogValueX64(value64));
    return CHIP_NO_ERROR;
}

static CHIP_ERROR PrintAllConfigs()
{
    ReturnErrorOnFailure(ConfigGetVendorId(true));
    ReturnErrorOnFailure(ConfigGetProductId(true));
    ReturnErrorOnFailure(ConfigGetProductRevision(true));

    ReturnErrorOnFailure(ConfigGetFabricId(true));
    ReturnErrorOnFailure(ConfigGetSetupPinCode(true));
    ReturnErrorOnFailure(ConfigGetSetupDiscriminator(true));

    ReturnErrorOnFailure(ConfigGetDeviceId(true));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigHandler(int argc, char ** argv)
{
    switch (argc)
    {
    case 0:
        return PrintAllConfigs();
    case 1:
        if (strcmp(argv[0], "vendorid") == 0)
        {
            return ConfigGetVendorId(false);
        }
        else if (strcmp(argv[0], "productid") == 0)
        {
            return ConfigGetProductId(false);
        }
        else if (strcmp(argv[0], "productrev") == 0)
        {
            return ConfigGetProductRevision(false);
        }
        else if (strcmp(argv[0], "deviceid") == 0)
        {
            return ConfigGetDeviceId(false);
        }
        else if (strcmp(argv[0], "pincode") == 0)
        {
            return ConfigGetSetupPinCode(false);
        }
        else if (strcmp(argv[0], "discriminator") == 0)
        {
            return ConfigGetSetupDiscriminator(false);
        }
        else if (strcmp(argv[0], "fabricid") == 0)
        {
            return ConfigGetFabricId(false);
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    case 2:
        if (strcmp(argv[0], "discriminator") == 0)
        {
            return ConfigSetSetupDiscriminator(argv[1]);
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void RegisterConfigCommands()
{

    static const shell_command_t sDeviceComand = { &ConfigHandler, "config",
                                                   "Manage device configuration. Usage to dump value: config [param_name] and "
                                                   "to set some values (discriminator): config [param_name] [param_value]." };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
