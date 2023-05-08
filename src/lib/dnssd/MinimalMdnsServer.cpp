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
#include "MinimalMdnsServer.h"

#include <lib/dnssd/minimal_mdns/AddressPolicy.h>

#ifndef CHIP_MINMDNS_DEFAULT_POLICY
#define CHIP_MINMDNS_DEFAULT_POLICY 0
#endif

#ifndef CHIP_MINMDNS_LIBNL_POLICY
#define CHIP_MINMDNS_LIBNL_POLICY 0
#endif

#if CHIP_MINMDNS_DEFAULT_POLICY
#include <lib/dnssd/minimal_mdns/AddressPolicy_DefaultImpl.h> // nogncheck
#endif

#if CHIP_MINMDNS_LIBNL_POLICY
#include <lib/dnssd/minimal_mdns/AddressPolicy_LibNlImpl.h> // nogncheck
#endif

namespace chip {
namespace Dnssd {

using namespace mdns::Minimal;
using chip::Platform::UniquePtr;

GlobalMinimalMdnsServer::GlobalMinimalMdnsServer()
{
    mServer.SetDelegate(this);

#if CHIP_MINMDNS_DEFAULT_POLICY
    mdns::Minimal::SetDefaultAddressPolicy();
#endif

#if CHIP_MINMDNS_LIBNL_POLICY
    mdns::Minimal::LibNl::SetAddressPolicy();
#endif
}

GlobalMinimalMdnsServer & GlobalMinimalMdnsServer::Instance()
{
    static GlobalMinimalMdnsServer _instance;
    return _instance;
}

CHIP_ERROR GlobalMinimalMdnsServer::StartServer(chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager,
                                                uint16_t port)
{
    GlobalMinimalMdnsServer::Server().ShutdownEndpoints();

    UniquePtr<ListenIterator> endpoints = GetAddressPolicy()->GetListenEndpoints();

    return GlobalMinimalMdnsServer::Server().Listen(udpEndPointManager, endpoints.get(), port);
}

void GlobalMinimalMdnsServer::ShutdownServer()
{
    GlobalMinimalMdnsServer::Server().Shutdown();
}

} // namespace Dnssd
} // namespace chip
