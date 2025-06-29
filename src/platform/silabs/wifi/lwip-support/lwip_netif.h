/*
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <platform/silabs/wifi/WifiInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Lwip {

/**
 * @brief Set station link status to up
 */
void SetLwipStationLinkUp(void);

/**
 * @brief Set station link status to down
 *
 */
void SetLwipStationLinkDown(void);

/**
 * @brief Initialize the LwIP stack
 *
 */
void InitializeLwip(void);

/**
 * @brief Returns the requested network interface
 *
 * @param interface requested network interface
 *
 * @return struct netif* if the requested interface is not supported, returns nullptr
 */
struct netif * GetNetworkInterface(sl_wfx_interface_t interface);

} // namespace Lwip
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
