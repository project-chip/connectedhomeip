/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 * @file WifiConnect.h
 *
 * Wi-Fi Connect Module allowing to join a pre-defined Wi-Fi network
 *
 **/

#pragma once

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace NXP {
namespace App {

/*
 * Function allowing to join a Wi-Fi network based on Wi-Fi build credentials
 * Must be called after completing Wi-Fi driver initialization
 */
CHIP_ERROR WifiConnectAtboot(chip::DeviceLayer::NetworkCommissioning::WiFiDriver * wifiDriver);

} // namespace App
} // namespace NXP
} // namespace chip
