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
#include <lib/dnssd/minimal_mdns/MinMdnsConfig.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

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
using namespace chip::Dnssd;
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

    mUdpEndPointManager = udpEndPointManager;
    mPort               = port;

#if CHIP_MINMDNS_INTERFACE_MONITOR
    if (!mInterfaceMonitor.IsRunning())
    {
        // Without it, interfaces that change after this are only picked up by the next start.
        LogErrorOnFailure(mInterfaceMonitor.Init(static_cast<chip::System::LayerSockets &>(udpEndPointManager->SystemLayer()),
                                                 HandleInterfacesChanged, this));
    }
#endif

    UniquePtr<ListenIterator> endpoints = GetAddressPolicy()->GetListenEndpoints();

    return GlobalMinimalMdnsServer::Server().Listen(udpEndPointManager, endpoints.get(), port);
}

void GlobalMinimalMdnsServer::ShutdownServer()
{
#if CHIP_MINMDNS_INTERFACE_MONITOR
    mInterfaceMonitor.Shutdown();
#endif
    mUdpEndPointManager = nullptr;

    GlobalMinimalMdnsServer::Server().Shutdown();
}

CHIP_ERROR GlobalMinimalMdnsServer::ListenAgain()
{
    VerifyOrReturnError(mUdpEndPointManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

    UniquePtr<ListenIterator> endpoints = GetAddressPolicy()->GetListenEndpoints();
    VerifyOrReturnError(endpoints != nullptr, CHIP_ERROR_NO_MEMORY);

    return GlobalMinimalMdnsServer::Server().Listen(mUdpEndPointManager, endpoints.get(), mPort, mListenDelegate);
}

#if CHIP_MINMDNS_INTERFACE_MONITOR
void GlobalMinimalMdnsServer::HandleInterfacesChanged(void * context)
{
    ChipLogDetail(Discovery, "Network interfaces changed, listening again");
    LogErrorOnFailure(static_cast<GlobalMinimalMdnsServer *>(context)->ListenAgain());
}
#endif

} // namespace Dnssd
} // namespace chip
