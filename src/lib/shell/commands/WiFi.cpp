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
#include <lib/support/AutoRelease.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/NetworkCommissioning.h>

using chip::DeviceLayer::ConnectivityManager;
using chip::DeviceLayer::ConnectivityMgr;
using namespace chip::DeviceLayer::NetworkCommissioning;

/// Convenience macro to auto-create a variable for you to release the given name at
/// the exit of the current scope.
#define DEFER_AUTO_RELEASE(name) AutoRelease autoRelease##__COUNTER__(name)

namespace chip {
namespace Shell {

class ShellScanCallback : public WiFiDriver::ScanCallback
{
public:
    void OnFinished(Status status, CharSpan debugText, WiFiScanResponseIterator * networks) override
    {
        DEFER_AUTO_RELEASE(networks);
        VerifyOrReturn(status == Status::kSuccess,
                       ChipLogError(Shell, "WiFi scan failed with status: %d", static_cast<int>(status)));

        ChipLogProgress(Shell, "WiFi scan completed");

        if (networks != nullptr)
        {
            WiFiScanResponse scanResponse;
            while (networks->Next(scanResponse))
            {
                ChipLogProgress(Shell, "SSID: %.*s", static_cast<int>(scanResponse.ssidLen), scanResponse.ssid);
            }
        }
    }
};

static ShellScanCallback sScanCallback;

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

    /* Command accepts running with SSID and password (optional) as parameters */
    VerifyOrReturnError((argc == 1 || argc == 2), CHIP_ERROR_INVALID_ARGUMENT);

    ByteSpan ssidSpan = ByteSpan::fromCharString(argv[0]);
    VerifyOrReturnError(!ssidSpan.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    ByteSpan passwordSpan;
    if (argc == 2)
    {
        passwordSpan = ByteSpan::fromCharString(argv[1]);
        VerifyOrReturnError(!passwordSpan.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        // If no password is provided, use an empty password
        passwordSpan = ByteSpan();
    }

    ChipLogProgress(Shell, "Adding/Updating network %s", argv[0]);

    /* AddOrUpdateNetwork() checks ssid length and password length. The network info is not persistent. */
    GetWiFiDriver()->AddOrUpdateNetwork(ssidSpan, passwordSpan, debugText, networkIndex);

    ChipLogProgress(Shell, "Connecting to network");
    /* Connection event will be returned in OnWiFiConnectivityChange from DeviceCallbacks.cpp */
    GetWiFiDriver()->ConnectNetwork(ssidSpan, nullptr);

    return error;
}
static CHIP_ERROR WiFiScanHandler(int argc, char ** argv)
{
    VerifyOrReturnError((argc == 0), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(GetWiFiDriver() != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    ByteSpan ssidSpan;
    GetWiFiDriver()->ScanNetworks(ssidSpan, &sScanCallback);

    return CHIP_NO_ERROR;
}
static CHIP_ERROR WiFiDisconnectHandler(int argc, char ** argv)
{
    VerifyOrReturnError((argc == 0), CHIP_ERROR_INVALID_ARGUMENT);

    return ConnectivityMgr().DisconnectNetwork();
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
static bool IsValidPAFFrequency(uint16_t freq)
{
    // 2.4 GHz: ch1-13 (2412-2472 MHz, 5 MHz spacing) and ch14 (2484 MHz)
    if (freq == 2484 || (freq >= 2412 && freq <= 2472 && (freq - 2412) % 5 == 0))
        return true;
    // 5 GHz: 20 MHz channels only (per Matter spec Section 5.4.2.6.2)
    // UNII-1:  5180-5320 MHz (ch36-64)
    // UNII-2e: 5500-5720 MHz (ch100-144)
    // UNII-3:  5745-5885 MHz (ch149-177)
    if ((freq >= 5180 && freq <= 5320 && (freq - 5000) % 20 == 0) || (freq >= 5500 && freq <= 5720 && (freq - 5000) % 20 == 0) ||
        (freq >= 5745 && freq <= 5885 && (freq - 5745) % 20 == 0))
        return true;
    return false;
}

static CHIP_ERROR WiFiPAFHandler(int argc, char ** argv)
{
    streamer_t * sout = streamer_get();

    if (argc != 2 || strcmp(argv[0], "freq_list") != 0)
    {
        streamer_printf(sout, "Usage: wifi paf freq_list <freq1[,freq2,...]>\r\n");
        streamer_printf(sout, "  e.g. wifi paf freq_list 5220        (5G ch44)\r\n");
        streamer_printf(sout, "  e.g. wifi paf freq_list 5745        (5G ch149)\r\n");
        streamer_printf(sout, "  e.g. wifi paf freq_list 5220,5745\r\n");
        streamer_printf(sout, "  The default publish channel is always used in addition to this list\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    constexpr size_t kMaxFreqEntries = 10;
    uint16_t freqs[kMaxFreqEntries];
    uint16_t count = 0;

    char buf[128];
    if (strlen(argv[1]) >= sizeof(buf))
    {
        streamer_printf(sout, "WiFi-PAF: frequency list is too long\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    strcpy(buf, argv[1]);

    char * token = strtok(buf, ",");
    while (token != nullptr)
    {
        if (count >= kMaxFreqEntries)
        {
            streamer_printf(sout, "WiFi-PAF: too many frequencies (max %u)\r\n", static_cast<unsigned>(kMaxFreqEntries));
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        char * end           = nullptr;
        unsigned long parsed = strtoul(token, &end, 10);
        if (end == token || *end != '\0')
        {
            streamer_printf(sout, "WiFi-PAF: invalid freq value: %s\r\n", token);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (parsed == 0 || parsed > UINT16_MAX)
        {
            streamer_printf(sout, "WiFi-PAF: freq value out of range (1-%u): %s\r\n", UINT16_MAX, token);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (!IsValidPAFFrequency(static_cast<uint16_t>(parsed)))
        {
            streamer_printf(sout, "WiFi-PAF: invalid Wi-Fi frequency: %s\r\n", token);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        freqs[count++] = static_cast<uint16_t>(parsed);
        token          = strtok(nullptr, ",");
    }

    if (count == 0)
    {
        streamer_printf(sout, "WiFi-PAF: no valid frequencies parsed\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Save the frequency list; it will take effect on the next opencommissioningwindow.
    DeviceLayer::ConnectivityManager::WiFiPAFAdvertiseParam params;
    params.freq_list_len = count;
    params.freq_list     = std::make_unique<uint16_t[]>(count);
    for (uint16_t i = 0; i < count; i++)
    {
        params.freq_list[i] = freqs[i];
    }
    ConnectivityMgr().WiFiPAFSetParam(params);

    streamer_printf(sout, "WiFi-PAF: freq_list saved: ");
    for (uint16_t i = 0; i < count; i++)
    {
        streamer_printf(sout, "%u%s", freqs[i], (i < count - 1) ? "," : "");
    }
    streamer_printf(sout, "\r\n");
    streamer_printf(sout, "WiFi-PAF: Close and reopen commissioning window to apply\r\n");

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

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
        { &WiFiConnectHandler, "connect", "Connect to AP. Usage: wifi connect <ssid> [<psk>]" },
        { &WiFiDisconnectHandler, "disconnect", "Disconnect device from AP. Usage: wifi disconnect" },
        { &WiFiScanHandler, "scan", "Scan networks (concurrent scans are not suported). Usage: wifi scan" },
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
        { &WiFiPAFHandler, "paf", "Add Wi-Fi PAF publish channels. Usage: wifi paf freq_list <freq1[,freq2,...]>" },
#endif
    };

    static constexpr Command wifiCommand = { &SubShellCommand<MATTER_ARRAY_SIZE(subCommands), subCommands>, "wifi",
                                             "Wi-Fi commands" };

    Engine::Root().RegisterCommands(&wifiCommand, 1);
}

} // namespace Shell
} // namespace chip
