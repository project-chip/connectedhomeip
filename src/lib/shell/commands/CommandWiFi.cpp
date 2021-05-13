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
#include <lib/shell/commands/CommandsHelp.h>
#include <lib/shell/shell_core.h>
#include <lib/shell/streamer.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

// Include DeviceNetworkProvisioningDelegateImpl for WiFi provisioning.
// TODO: Enable wifi network should be done by ConnectivityManager. (Or other platform neutral interfaces)
#if defined(CHIP_DEVICE_LAYER_TARGET)
#define DEVICENETWORKPROVISIONING_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/DeviceNetworkProvisioningDelegateImpl.h>
#include DEVICENETWORKPROVISIONING_HEADER
#endif

using chip::DeviceLayer::ConnectivityManager;
using chip::DeviceLayer::ConnectivityMgr;

namespace chip {
namespace Shell {

static chip::Shell::Shell sShellWifiSubCommands;

static int CommandWifiHelp(int argc, char ** argv)
{
    sShellWifiSubCommands.ForEachCommand(PrintCommandHelp, nullptr);
    return 0;
}

static int PrintWifiMode()
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

    return 0;
}

static int SetWifiMode(const char * mode)
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

    return 0;
}

static int CommandWifiMode(int argc, char ** argv)
{
    if (argc == 0)
    {
        return PrintWifiMode();
    }
    else if (argc != 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return SetWifiMode(argv[0]);
}

static int CommandWifiConnect(int argc, char ** argv)
{
    if (argc != 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
    return deviceDelegate.ProvisionWiFi(argv[0], argv[1]);
}

static int CommandWifiDispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        return CommandWifiHelp(argc, argv);
    }
    return sShellWifiSubCommands.ExecCommand(argc, argv);
}

void CommandWifiInit()
{
    /// Subcommands for root command: `device <subcommand>`
    static const shell_command_t sWifiSubCommands[] = {
        { &CommandWifiHelp, "help", "" },
        { &CommandWifiMode, "mode", "Get/Set wifi mode. Usage: wifi mode [disable|ap|sta]." },
        { &CommandWifiConnect, "connect", "Connect to AP. Usage: wifi connect ssid psk." },
    };
    static const shell_command_t sWifiCommand = { &CommandWifiDispatch, "wifi", "Usage: wifi <subcommand>" };

    sShellWifiSubCommands.RegisterCommands(sWifiSubCommands, ArraySize(sWifiSubCommands));
    shell_register(&sWifiCommand, 1);
}

} // namespace Shell
} // namespace chip
