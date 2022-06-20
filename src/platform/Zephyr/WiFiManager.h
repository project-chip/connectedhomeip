/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Provides the wrapper for Zephyr WiFi API
 */

#pragma once

#include "lib/core/CHIPError.h"

#include <net/wifi_mgmt.h>

#include <zephyr.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

class WiFiManager
{

public:
    static WiFiManager & Instance()
    {
        static WiFiManager sInstance;
        return sInstance;
    }

    CHIP_ERROR Init()
    {
        // for now just attempt to call anything from Zephyr net_mgmt API (for instance scanning)
        // struct net_if * iface = net_if_get_default();
        // auto ret              = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);
        // if (ret)
        // {
        //     ChipLogDetail(DeviceLayer, "Scan cannot be started. Error: %d", ret);
        //     return CHIP_ERROR_INCORRECT_STATE;
        // }
        // else
        // {
        //     ChipLogDetail(DeviceLayer, "Scan started");
        // }
        return CHIP_NO_ERROR;
    }
};

} // namespace DeviceLayer
} // namespace chip
