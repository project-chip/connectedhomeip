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

    if (printHeader)
    {
        streamer_printf(sout, "VendorId:        ");
    }
    ReturnErrorOnFailure(ConfigurationMgr().GetVendorId(value16));
    streamer_printf(sout, "%" PRIu16 " (0x%" PRIX16 ")\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetProductId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    if (printHeader)
    {
        streamer_printf(sout, "ProductId:       ");
    }
    ReturnErrorOnFailure(ConfigurationMgr().GetProductId(value16));
    streamer_printf(sout, "%" PRIu16 " (0x%" PRIX16 ")\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetProductRevision(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t value16;

    if (printHeader)
    {
        streamer_printf(sout, "ProductRevision: ");
    }
    ReturnErrorOnFailure(ConfigurationMgr().GetProductRevision(value16));
    streamer_printf(sout, "%" PRIu16 " (0x%" PRIX16 ")\r\n", value16, value16);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetDeviceId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint64_t value64;

    if (printHeader)
    {
        streamer_printf(sout, "DeviceId:        ");
    }
    ReturnErrorOnFailure(ConfigurationMgr().GetDeviceId(value64));
    streamer_printf(sout, "%" PRIu64 " (0x" ChipLogFormatX64 ")\r\n", value64, ChipLogValueX64(value64));
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetSetupPinCode(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint32_t setupPinCode;

    if (printHeader)
    {
        streamer_printf(sout, "PinCode:         ");
    }
    ReturnErrorOnFailure(ConfigurationMgr().GetSetupPinCode(setupPinCode));
    streamer_printf(sout, "%09u\r\n", setupPinCode);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetSetupDiscriminator(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint16_t setupDiscriminator;

    if (printHeader)
    {
        streamer_printf(sout, "Discriminator:   ");
    }
    ReturnErrorOnFailure(ConfigurationMgr().GetSetupDiscriminator(setupDiscriminator));
    streamer_printf(sout, "%03x\r\n", setupDiscriminator & 0xFFF);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ConfigGetFabricId(bool printHeader)
{
    streamer_t * sout = streamer_get();
    uint64_t value64;

    if (printHeader)
    {
        streamer_printf(sout, "FabricId:        ");
    }
    ReturnErrorOnFailure(ConfigurationMgr().GetFabricId(value64));
    streamer_printf(sout, "%" PRIu64 " (0x" ChipLogFormatX64 ")\r\n", value64, ChipLogValueX64(value64));
    return CHIP_NO_ERROR;
}

static int PrintAllConfigs()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    error |= ConfigGetVendorId(true);
    error |= ConfigGetProductId(true);
    error |= ConfigGetProductRevision(true);

    error |= ConfigGetFabricId(true);
    error |= ConfigGetSetupPinCode(true);
    error |= ConfigGetSetupDiscriminator(true);

    error |= ConfigGetDeviceId(true);

    return (error) ? CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND : CHIP_NO_ERROR;
}

static int ConfigHandler(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc == 0)
    {
        return PrintAllConfigs();
    }

    if (strcmp(argv[0], "vendorid") == 0)
    {
        return ConfigGetVendorId(false);
    }
    else if (strcmp(argv[0], "productid") == 0)
    {
        return error = ConfigGetProductId(false);
    }
    else if (strcmp(argv[0], "productrev") == 0)
    {
        return error = ConfigGetProductRevision(false);
    }
    else if (strcmp(argv[0], "deviceid") == 0)
    {
        return error = ConfigGetDeviceId(false);
    }
    else if (strcmp(argv[0], "pincode") == 0)
    {
        return error = ConfigGetSetupPinCode(false);
    }
    else if (strcmp(argv[0], "discriminator") == 0)
    {
        return error = ConfigGetSetupDiscriminator(false);
    }
    else if (strcmp(argv[0], "fabricid") == 0)
    {
        return error = ConfigGetFabricId(false);
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void RegisterConfigCommands()
{

    static const shell_command_t sDeviceComand = { &ConfigHandler, "config",
                                                   "Dump device configuration. Usage: config [param_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
