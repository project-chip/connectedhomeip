/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/dnssd/Resolver.h>

namespace chip {
namespace Dnssd {

/**
 * Convenience class for discovering Matter commissioners and commissionable nodes.
 *
 * The class simplifies the usage of the global DNS-SD resolver to discover Matter nodes by managing
 * the lifetime of the discovery context object.
 *
 * The proxy provides a method to register an external commissioning delegate whose
 * `OnNodeDiscovered` method is called whenever a new node is discovered. The delegate is
 * automatically unregistered when the proxy is shut down or destroyed.
 *
 * The proxy can be safely shut down or destroyed even if the last discovery operation has not
 * finished yet.
 */
class ResolverProxy
{
public:
    explicit ResolverProxy(Resolver * resolver = nullptr) : mResolver(resolver != nullptr ? *resolver : Resolver::Instance()) {}
    ~ResolverProxy() { Shutdown(); }

    CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPoint = nullptr);
    void Shutdown();

    void SetDiscoveryDelegate(DiscoverNodeDelegate * delegate)
    {
        if (mContext != nullptr)
        {
            mContext->SetDiscoveryDelegate(delegate);
        }
    }

    CHIP_ERROR DiscoverCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter());
    CHIP_ERROR DiscoverCommissioners(DiscoveryFilter filter = DiscoveryFilter());
    CHIP_ERROR DiscoverOperationalNodes(DiscoveryFilter filter = DiscoveryFilter());
    CHIP_ERROR StopDiscovery();

private:
    Resolver & mResolver;
    DiscoveryContext * mContext = nullptr;
};

} // namespace Dnssd
} // namespace chip
