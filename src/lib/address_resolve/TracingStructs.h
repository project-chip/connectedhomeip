/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/address_resolve/AddressResolve.h>
#include <tracing/log_declares.h>

namespace chip {
namespace Tracing {

/// Concrete definitions of the DNSSD tracing info that the address resolver will
/// report.

/// A node lookup has been requested
struct NodeLookupInfo
{
    const chip::AddressResolve::NodeLookupRequest * request; // request for node lookup
};

enum class DiscoveryInfoType
{
    kIntermediateResult = 0, // Received intermediate address data
    kResolutionDone     = 1, // resolution completed
    kRetryDifferent     = 2, // Try a different/new IP address
};

/// A node was discovered and we have information about it.
struct NodeDiscoveredInfo
{
    DiscoveryInfoType type;                             // separate callbacks depending on state
    const PeerId * peerId;                              // what peer was discovered
    const chip::AddressResolve::ResolveResult * result; // a SINGLE result representing the resolution
};

/// A node lookup failed / give up on this discovery
struct NodeDiscoveryFailedInfo
{
    const PeerId * peerId; // what peer discovery failed
    CHIP_ERROR error;      // error that caused a failure
};

} // namespace Tracing
} // namespace chip
