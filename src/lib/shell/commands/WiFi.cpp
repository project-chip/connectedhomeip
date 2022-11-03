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

#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/shell/streamer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

using chip::DeviceLayer::ConnectivityManager;
using chip::DeviceLayer::ConnectivityMgr;

namespace chip {
namespace Shell {

static chip::Shell::Engine sShellWiFiSubCommands;

static CHIP_ERROR WiFiHelpHandler(int argc, char ** argv)
{
    sShellWiFiSubCommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR PrintWiFiMode()
{
    streamer_t * sout                            = streamer_get();
    ConnectivityManager::WiFiAPMode apMode       = ConnectivityMgr().GetWiFiAPMode();
    ConnectivityManager::WiFiStationMode staMode = ConnectivityMgr().GetWiFiStationMode();
    bool apEnabled                               = (apMode == ConnectivityManager::kWiFiAPMode_Enabled);
    bool staEnabled                              = (staMode == ConnectivityManager::kWiFiStationMode_Enabled);

    if (apEnabled && !staEnabled)
    {
        streamer_printf(sout, "ap\r\n");
    }
    else if (!apEnabled && staEnabled)
    {
        streamer_printf(sout, "sta\r\n");
    }
    else if (!apEnabled && !staEnabled)
    {
        streamer_printf(sout, "disable\r\n");
    }
    else
    {
        streamer_printf(sout, "mode not supported\r\n");
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR SetWiFiMode(const char * mode)
{
    if (strcmp(mode, "disable") == 0)
    {
        ReturnErrorOnFailure(ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Disabled));
        ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    }
    else if (strcmp(mode, "ap") == 0)
    {
        ReturnErrorOnFailure(ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Enabled));
        ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    }
    else if (strcmp(mode, "sta") == 0)
    {
        ReturnErrorOnFailure(ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Disabled));
        ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR WiFiModeHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return PrintWiFiMode();
    }
    if (argc != 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return SetWiFiMode(argv[0]);
}

static CHIP_ERROR WiFiConnectHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // TODO:Provision WiFi using WirelessDriver
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

static CHIP_ERROR WiFiDispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        return WiFiHelpHandler(argc, argv);
    }
    return sShellWiFiSubCommands.ExecCommand(argc, argv);
}

void RegisterWiFiCommands()
{
    /// Subcommands for root command: `device <subcommand>`
    static const shell_command_t sWiFiSubCommands[] = {
        { &WiFiHelpHandler, "help", "" },
        { &WiFiModeHandler, "mode", "Get/Set wifi mode. Usage: wifi mode [disable|ap|sta]." },
        { &WiFiConnectHandler, "connect", "Connect to AP. Usage: wifi connect ssid psk." },
    };
    static const shell_command_t sWiFiCommand = { &WiFiDispatch, "wifi", "Usage: wifi <subcommand>" };

    sShellWiFiSubCommands.RegisterCommands(sWiFiSubCommands, ArraySize(sWiFiSubCommands));
    Engine::Root().RegisterCommands(&sWiFiCommand, 1);
}

} // namespace Shell
} // namespace chip
