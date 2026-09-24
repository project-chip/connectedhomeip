/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ESP32/ESP32Utils.h>

using namespace ::chip::DeviceLayer::Internal;

struct netif * ESP32Utils::GetStationNetif(void)
{
    return nullptr;
}

struct netif * ESP32Utils::GetNetif(const char *)
{
    return nullptr;
}

bool ESP32Utils::IsInterfaceUp(const char *)
{
    return false;
}

bool ESP32Utils::HasIPv6LinkLocalAddress(const char *)
{
    return false;
}
