/*
 *
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

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {
namespace Clusters {

/** @brief
 *    Encapsulate a Network Commissioning Driver with its network commissioning cluster instance
 *
 *    The passed NetworkCommissioningDriver typename should be the Network Commissioning Driver implemented class to be used.
 *    For example your platform's selected implementation of one of the following drivers.
 *    - DeviceLayer::NetworkCommissioning::WiFiDriver;
 *    - DeviceLayer::NetworkCommissioning::ThreadDriver;
 *    - DeviceLayer::NetworkCommissioning::EthernetDriver;
 */

template <typename NetworkCommissioningDriver>
class NetworkDriverObj
{
public:
    NetworkDriverObj(chip::EndpointId endpoint) : instance(endpoint, &driver) {}
    virtual ~NetworkDriverObj() { instance.Shutdown(); }

    // instance.Init is left out of the contructor as the drivers call MemoryAlloc before MemoryInit is called.
    void Init() { instance.Init(); }
    NetworkCommissioningDriver * GetDriver() { return &driver; }

private:
    NetworkCommissioningDriver driver;
    NetworkCommissioning::Instance instance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
