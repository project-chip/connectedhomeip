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

#pragma once

#include <mdns/Resolver.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {

namespace Controller {

constexpr uint16_t kMdnsPort = 5353;

/**
 * @brief
 *   Convenient superclass for controller implementations that need to discover
 *   Commissioners or CommissionableNodes using mDNS. This Abstract class
 *   provides base implementations for logic to setup mDNS discovery requests,
 *   handling of received DiscoveredNodeData, etc. while expecting child classes
 *   to maintain a list of DiscoveredNodes and providing the implementation
 *   of the template GetDiscoveredNodes() function.
 */
class DLL_EXPORT AbstractMdnsDiscoveryController : public Mdns::ResolverDelegate
{
public:
    AbstractMdnsDiscoveryController(){};
    virtual ~AbstractMdnsDiscoveryController() {}

    void OnNodeDiscoveryComplete(const chip::Mdns::DiscoveredNodeData & nodeData) override;

protected:
    CHIP_ERROR SetUpNodeDiscovery();
    CHIP_ERROR SetUpNodeDiscoveryLongDiscriminator(uint16_t long_discriminator);
    const Mdns::DiscoveredNodeData * GetDiscoveredNode(int idx);
    virtual Mdns::DiscoveredNodeData * GetDiscoveredNodes() = 0;

    Mdns::DiscoveryFilter filter;
};

} // namespace Controller
} // namespace chip
