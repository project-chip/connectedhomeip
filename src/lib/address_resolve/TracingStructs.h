/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
