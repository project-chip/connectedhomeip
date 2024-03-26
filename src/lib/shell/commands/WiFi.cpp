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
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/NetworkCommissioning.h>

using chip::DeviceLayer::ConnectivityManager;
using chip::DeviceLayer::ConnectivityMgr;
using namespace chip::DeviceLayer::NetworkCommissioning;

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

static CHIP_ERROR WifiAddNetwork(char * ssid, char * password)
{
    if (ConnectivityMgr().GetDriver() == nullptr)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    uint8_t networkIndex;
    chip::MutableCharSpan debugText;
    chip::ByteSpan ssidSpan;
    chip::ByteSpan passwordSpan;

    ssidSpan     = chip::ByteSpan(reinterpret_cast<const uint8_t *>(ssid), strlen(ssid));
    passwordSpan = chip::ByteSpan(reinterpret_cast<const uint8_t *>(password), strlen(password));

    if (IsSpanUsable(ssidSpan) && IsSpanUsable(passwordSpan))
    {
        ChipLogProgress(DeviceLayer, "[Shell] Adding/Updating network %s", ssidSpan.data());

        /* AddOrUpdateNetwork() checks ssid length and password length. The network info is not persistent. */
        ConnectivityMgr().GetDriver()->AddOrUpdateNetwork(ssidSpan, passwordSpan, debugText, networkIndex);

        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

static CHIP_ERROR WiFiConnectHandler(int argc, char ** argv)
{
    if (ConnectivityMgr().GetDriver() == nullptr)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /* Command accepts running either without arguments or either with SSID and password */
    /* Running without arguments implies that "wifi add ssid password" was executed before */
    if ((argc != 0) && (argc != 2))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    chip::ByteSpan ssidSpan;
    bool connectToNetwork = false;

    if (argc == 0)
    {
        /* Retrieve previously added SSID */
        char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength + 1] = { 0 };

        ConnectivityMgr().GetNetworkSSID(&ssid[0]);
        ssidSpan = chip::ByteSpan(reinterpret_cast<const uint8_t *>(ssid), strlen(ssid));

        if (IsSpanUsable(ssidSpan))
        {
            connectToNetwork = true;
        }
        else
        {
            ChipLogError(
                DeviceLayer,
                "[Shell] No network credentials found! Please add using <wifi add ssid password> or <wifi connect ssid password>");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else if (argc == 2)
    {
        ssidSpan = chip::ByteSpan(reinterpret_cast<const uint8_t *>(argv[0]), strlen(argv[0]));

        if ((IsSpanUsable(ssidSpan)) && (WifiAddNetwork(argv[0], argv[1]) == CHIP_NO_ERROR))
        {
            connectToNetwork = true;
        }
        else
        {
            ChipLogError(DeviceLayer, "[Shell] Failed to add network credentials!");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    if (connectToNetwork == true)
    {
        ChipLogProgress(DeviceLayer, "[Shell] Progress: Connecting to network");
        /* Connection event will be returned in OnWiFiConnectivityChange from DeviceCallbacks.cpp */
        ConnectivityMgr().GetDriver()->ConnectNetwork(ssidSpan, nullptr);
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR WiFiDisconnectHandler(int argc, char ** argv)
{
    if (argc != 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return ConnectivityMgr().DisconnectNetwork();
}

static CHIP_ERROR WiFiAddNwkHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return WifiAddNetwork(argv[0], argv[1]);
}

static CHIP_ERROR WiFiRemoveNwkHandler(int argc, char ** argv)
{
    if (ConnectivityMgr().GetDriver() == nullptr)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    chip::DeviceLayer::NetworkCommissioning::Status status;
    uint8_t networkIndex;
    chip::ByteSpan ssidSpan;
    char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength + 1] = { 0 };
    chip::MutableCharSpan debugText;

    ConnectivityMgr().GetNetworkSSID(&ssid[0]);
    ssidSpan = chip::ByteSpan(reinterpret_cast<const uint8_t *>(ssid), strlen(ssid));

    status = ConnectivityMgr().GetDriver()->RemoveNetwork(ssidSpan, debugText, networkIndex);

    if (status != chip::DeviceLayer::NetworkCommissioning::Status::kSuccess)
    {
        ChipLogError(DeviceLayer, "[Shell] Error: RemoveNetwork: %u", (uint8_t) status);
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR WiFiScanHandler(int argc, char ** argv)
{
    if (ConnectivityMgr().GetDriver() == nullptr)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /* Command accepts running either without arguments or either with ssid */
    /* Running with argument ssid shall restrict the scan to the interested ssid */
    if ((argc != 0) && (argc != 1))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    chip::ByteSpan ssidSpan;

    if (argc == 0)
    {
        ssidSpan = chip::ByteSpan();
    }
    else if (argc == 1)
    {
        ssidSpan = chip::ByteSpan(reinterpret_cast<const uint8_t *>(argv[0]), strlen(argv[0]));
    }

    ConnectivityMgr().GetDriver()->ScanNetworks(ssidSpan, nullptr);

    return CHIP_NO_ERROR;
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
        { &WiFiModeHandler, "mode", "Get/Set wifi mode. Usage: wifi mode [disable|ap|sta]" },
        { &WiFiConnectHandler, "connect", "Connect to AP. Usage: wifi connect <ssid> <psk>. ssid and psk are optional." },
        { &WiFiDisconnectHandler, "disconnect", "Disconnect device from AP. Usage: wifi disconnect" },
        { &WiFiAddNwkHandler, "add", "Add credentials for Wi-Fi network. Usage: wifi add <ssid> <psk>" },
        { &WiFiRemoveNwkHandler, "remove", "Remove credentials for Wi-Fi network. Usage: wifi remove <ssid>" },
        { &WiFiScanHandler, "scan", "Scan for Wi-Fi networks. Usage: wifi scan <ssid>. ssid is optional." },
    };
    static const shell_command_t sWiFiCommand = { &WiFiDispatch, "wifi", "Usage: wifi <subcommand>" };

    sShellWiFiSubCommands.RegisterCommands(sWiFiSubCommands, ArraySize(sWiFiSubCommands));
    Engine::Root().RegisterCommands(&sWiFiCommand, 1);
}

} // namespace Shell
} // namespace chip
