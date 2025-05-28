/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#pragma once

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

// NetworkDriver for the devices that don't have / don't need a real network driver.
class NullNetworkDriver : public DeviceLayer::NetworkCommissioning::EthernetDriver
{
public:
    ~NullNetworkDriver() override = default;

    uint8_t GetMaxNetworks() override { return 1; }
    DeviceLayer::NetworkCommissioning::NetworkIterator * GetNetworks() override { return nullptr; }
    bool GetEnabled() override { return false; }
};

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
