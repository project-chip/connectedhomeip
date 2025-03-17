/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once
#include <lib/core/NodeId.h>

namespace chip {
namespace WiFiPAF {

/// Role of end points' associated WiFiPAF connections. Determines means used by end points to send and receive data.
typedef enum
{
    kWiFiPafRole_Publisher  = 0,
    kWiFiPafRole_Subscriber = 1
} WiFiPafRole;

inline constexpr uint32_t kUndefinedWiFiPafSessionId = UINT32_MAX;
struct WiFiPAFSession
{
    WiFiPafRole role;
    uint32_t id;
    uint32_t peer_id;
    uint8_t peer_addr[6];
    NodeId nodeId;
    uint16_t discriminator;
};

} // namespace WiFiPAF
} // namespace chip
