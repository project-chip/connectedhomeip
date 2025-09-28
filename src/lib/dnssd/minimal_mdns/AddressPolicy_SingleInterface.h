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

#include <inet/InetInterface.h>
#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/support/CHIPMem.h>

namespace mdns {
namespace Minimal {

class SingleInterfaceAddressPolicy : public AddressPolicy
{
public:
    SingleInterfaceAddressPolicy() = default;

    SingleInterfaceAddressPolicy(AddressPolicy * parentPolicy, chip::Inet::InterfaceId & selectedIface);

    chip::Platform::UniquePtr<ListenIterator> GetListenEndpoints() override;
    chip::Platform::UniquePtr<IpAddressIterator> GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId,
                                                                           chip::Inet::IPAddressType type) override;

    ~SingleInterfaceAddressPolicy() override = default;

private:
    mdns::Minimal::AddressPolicy * mParentPolicy;
    chip::Inet::InterfaceId * mSelectedIface;
};

} // namespace Minimal
} // namespace mdns
