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

static chip::Shell::Engine sShellWifiSubCommands;

static CHIP_ERROR WiFiHelpHandler(int argc, char ** argv)
{
    sShellWifiSubCommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
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

static CHIP_ERROR WiFiModeHandler(int argc, char ** argv)
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

static CHIP_ERROR WiFiConnectHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
    return deviceDelegate.ProvisionWiFi(argv[0], argv[1]);
}

static CHIP_ERROR WiFiDispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        return WiFiHelpHandler(argc, argv);
    }
    return sShellWifiSubCommands.ExecCommand(argc, argv);
}

void RegisterWiFiCommands()
{
    /// Subcommands for root command: `device <subcommand>`
    static const shell_command_t sWifiSubCommands[] = {
        { &WiFiHelpHandler, "help", "" },
        { &WiFiModeHandler, "mode", "Get/Set wifi mode. Usage: wifi mode [disable|ap|sta]." },
        { &WiFiConnectHandler, "connect", "Connect to AP. Usage: wifi connect ssid psk." },
    };
    static const shell_command_t sWifiCommand = { &WiFiDispatch, "wifi", "Usage: wifi <subcommand>" };

    sShellWifiSubCommands.RegisterCommands(sWifiSubCommands, ArraySize(sWifiSubCommands));
    Engine::Root().RegisterCommands(&sWifiCommand, 1);
}

} // namespace Shell
} // namespace chip
