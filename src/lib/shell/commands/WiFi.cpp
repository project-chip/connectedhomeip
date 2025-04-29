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
#include <lib/shell/SubShellCommand.h>
#include <lib/shell/commands/WiFi.h>
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

static DeviceLayer::NetworkCommissioning::WiFiDriver * sDriver;

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
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint8_t networkIndex;
    char debugBuffer[CHIP_CONFIG_NETWORK_COMMISSIONING_DEBUG_TEXT_BUFFER_SIZE];
    MutableCharSpan debugText(debugBuffer);

    VerifyOrReturnError(GetWiFiDriver() != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    /* Command accepts running with SSID and password as parameters */
    VerifyOrReturnError((argc == 2), CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan ssidSpan     = ByteSpan(Uint8::from_const_char(argv[0]), strlen(argv[0]));
    ByteSpan passwordSpan = ByteSpan(Uint8::from_const_char(argv[1]), strlen(argv[1]));

    VerifyOrReturnError(IsSpanUsable(ssidSpan) && IsSpanUsable(passwordSpan), CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(Shell, "Adding/Updating network %s", argv[0]);

    /* AddOrUpdateNetwork() checks ssid length and password length. The network info is not persistent. */
    GetWiFiDriver()->AddOrUpdateNetwork(ssidSpan, passwordSpan, debugText, networkIndex);

    ChipLogProgress(Shell, "Connecting to network");
    /* Connection event will be returned in OnWiFiConnectivityChange from DeviceCallbacks.cpp */
    GetWiFiDriver()->ConnectNetwork(ssidSpan, nullptr);

    return error;
}

static CHIP_ERROR WiFiDisconnectHandler(int argc, char ** argv)
{
    VerifyOrReturnError((argc == 0), CHIP_ERROR_INVALID_ARGUMENT);

    return ConnectivityMgr().DisconnectNetwork();
}

void SetWiFiDriver(WiFiDriver * driver)
{
    sDriver = driver;
}

WiFiDriver * GetWiFiDriver()
{
    return sDriver;
}

void RegisterWiFiCommands()
{
    static constexpr Command subCommands[] = {
        { &WiFiModeHandler, "mode", "Get/Set wifi mode. Usage: wifi mode [disable|ap|sta]" },
        { &WiFiConnectHandler, "connect", "Connect to AP. Usage: wifi connect <ssid> <psk>" },
        { &WiFiDisconnectHandler, "disconnect", "Disconnect device from AP. Usage: wifi disconnect" },
    };

    static constexpr Command wifiCommand = { &SubShellCommand<MATTER_ARRAY_SIZE(subCommands), subCommands>, "wifi",
                                             "Wi-Fi commands" };

    Engine::Root().RegisterCommands(&wifiCommand, 1);
}

} // namespace Shell
} // namespace chip
