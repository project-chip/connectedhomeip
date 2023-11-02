/*
 *  Copyright 2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/support/Span.h>
#include <lwip/api.h>
#include <platform/ConnectivityManager.h>
#include <platform/nxp/common/NetworkCommissioningDriver.h>

extern "C" {
#include "wifi.h"
#include "wlan.h"
#include "wm_net.h"
#include <wm_os.h>
}

using namespace ::chip::DeviceLayer;

/* Wifi Connect task configuration */
#define WIFI_CONNECT_TASK_NAME "wifi_connect"
#define WIFI_CONNECT_TASK_SIZE 1024
#define WIFI_CONNECT_TASK_PRIO (configMAX_PRIORITIES - 10)

/* Set to 1 in order to connect to a dedicated Wi-Fi network. */
#ifndef WIFI_CONNECT
#define WIFI_CONNECT 0
#endif

bool networkAdded = false;

void _wifiConnect(void * pvParameters)
{
#if WIFI_CONNECT
    CHIP_ERROR chip_err;
    NetworkCommissioning::Status status;
    chip::MutableCharSpan debugText;
    uint8_t networkIndex;

    int len_ssid, len_pass;
    char ssidBuf[Internal::kMaxWiFiSSIDLength] = { 0 };
    char passBuf[Internal::kMaxWiFiKeyLength]  = { 0 };

    chip::ByteSpan ssid;
    chip::ByteSpan password;

    /* The Init() method also checks previously saved configurations, but the saving is currently done in RAM. */
    chip_err = NetworkCommissioning::NXPWiFiDriver::GetInstance().Init(nullptr);
    if (chip_err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "[WIFI_CONNECT_TASK] Error: Init: SSID and/or PASSWORD not found in persistent storage");

#ifdef WIFI_SSID
        /* If SSID and PASSWORD are entered at build time, they can be used */
        ssid = chip::ByteSpan(reinterpret_cast<const uint8_t *>(WIFI_SSID), strlen(WIFI_SSID));
        ChipLogProgress(DeviceLayer, "[WIFI_CONNECT_TASK] Info: Using the default SSID %s", ssid.data());
#endif

#ifdef WIFI_PASSWORD
        /* If SSID and PASSWORD are entered at build time, they can be used */
        password = chip::ByteSpan(reinterpret_cast<const uint8_t *>(WIFI_PASSWORD), strlen(WIFI_PASSWORD));
        ChipLogProgress(DeviceLayer, "[WIFI_CONNECT_TASK] Info: Using the default password %s", password.data());
#endif

        /* If SSID and Password are filled, WiFi can be automatically started */
        if (IsSpanUsable(ssid) && IsSpanUsable(password))
        {
            ChipLogProgress(DeviceLayer, "[WIFI_CONNECT_TASK] Connecting to Wi-Fi network: SSID = %s and PASSWORD = %s",
                            ssid.data(), password.data());

            status = NetworkCommissioning::NXPWiFiDriver::GetInstance().AddOrUpdateNetwork(ssid, password, debugText, networkIndex);
            networkAdded = true;

            if (status != NetworkCommissioning::Status::kSuccess)
            {
                ChipLogError(DeviceLayer, "[WIFI_CONNECT_TASK] Error: AddOrUpdateNetwork: %u", (uint8_t) status);
            }

            /* Connection event will be returned in OnWiFiConnectivityChange from DeviceCallbacks.cpp */
            NetworkCommissioning::NXPWiFiDriver::GetInstance().ConnectNetwork(ssid, nullptr);
        }
        else
        {
            ChipLogError(DeviceLayer, "[WIFI_CONNECT_TASK] No valid SSID and password found!");
        }
    }
    else if (chip_err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "[WIFI_CONNECT_TASK] Error: Init: %" CHIP_ERROR_FORMAT, chip_err.Format());
    }

    /* Delete task after connection establishment */
    vTaskDelete(NULL);

#endif /* WIFI_CONNECT */
}

/* Creates a dedicated Task responsible for connecting to a WiFi network */
void WifiConnectTaskCreate(void)
{
    TaskHandle_t taskHandle;

    if (xTaskCreate(&_wifiConnect, WIFI_CONNECT_TASK_NAME, WIFI_CONNECT_TASK_SIZE, NULL, WIFI_CONNECT_TASK_PRIO, &taskHandle) !=
        pdPASS)
    {
        ChipLogError(DeviceLayer, "Failed to start wifi_connect task");
        assert(false);
    }
}
