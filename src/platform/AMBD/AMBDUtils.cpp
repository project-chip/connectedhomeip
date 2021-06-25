/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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
 *          General utility methods for the AmebaD platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/AMBD/AMBDUtils.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

CHIP_ERROR AMBDUtils::IsAPEnabled(bool & apEnabled)
{
    // TODO
    return CHIP_NO_ERROR;
}

bool AMBDUtils::IsStationProvisioned(void)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR AMBDUtils::IsStationConnected(bool & connected)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR AMBDUtils::StartWiFiLayer(void)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR AMBDUtils::EnableStationMode(void)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR AMBDUtils::SetAPMode(bool enabled)
{
    // TODO
    return CHIP_NO_ERROR;
}

int AMBDUtils::OrderScanResultsByRSSI(const void * _res1, const void * _res2)
{
    // TODO
    return CHIP_NO_ERROR;
}


const char * AMBDUtils::WiFiModeToStr(int wifiMode)
{
    // TODO
    return "(unknown)";
}


struct netif * AMBDUtils::GetStationNetif(void)
{
    // TODO
    return CHIP_NO_ERROR;
}

struct netif * AMBDUtils::GetNetif(const char * ifKey)
{
    // TODO
    return CHIP_NO_ERROR;
}

bool AMBDUtils::IsInterfaceUp(const char * ifKey)
{
    // TODO
    return CHIP_NO_ERROR;
}

bool AMBDUtils::HasIPv6LinkLocalAddress(const char * ifKey)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR AMBDUtils::GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR AMBDUtils::SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR AMBDUtils::ClearWiFiStationProvision(void)
{
    // TODO
    return CHIP_NO_ERROR;
}
