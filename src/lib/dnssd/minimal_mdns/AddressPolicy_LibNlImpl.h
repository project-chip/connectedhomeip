/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
