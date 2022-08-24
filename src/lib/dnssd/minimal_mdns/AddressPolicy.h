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

#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/ServerIPAddresses.h>
#include <lib/support/CHIPMem.h>

namespace mdns {
namespace Minimal {
namespace Policies {

/// Get all endpoints on which minimal MDNS should listen on.
chip::Platform::UniquePtr<mdns::Minimal::ListenIterator> GetListenEndpoints();

/// Fetch all the IP addresses for the given interface which are valid
/// for DNSSD server advertisement.
///
/// Generally this should skip invalid addresses even if reported by the
/// underlying operating system (e.g. linux could skip deprecated/temporary/dad-failed ones).
chip::Platform::UniquePtr<mdns::Minimal::IpAddressIterator> GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId);

} // namespace Policies
} // namespace Minimal
} // namespace mdns
