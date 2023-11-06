/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/dnssd/minimal_mdns/AddressPolicy.h>

namespace mdns {
namespace Minimal {
namespace LibNl {

class LibNl_AddressPolicy : public mdns::Minimal::AddressPolicy
{
public:
    chip::Platform::UniquePtr<ListenIterator> GetListenEndpoints() override;

    chip::Platform::UniquePtr<IpAddressIterator> GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId,
                                                                           chip::Inet::IPAddressType type) override;
};

void SetAddressPolicy();

} // namespace LibNl
} // namespace Minimal
} // namespace mdns
