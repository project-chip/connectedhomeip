/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/support/Variant.h>

namespace chip {
namespace Inet {

/// The host can't tell whether it has a route to the destination.
struct UnknownRoute
{
};

/// The host knows that it has no route to the destination.
struct NoRoute
{
};

/// The host has a route to the destination, and would send to it from `localAddress` on `interfaceId`.
struct RouteFromLocalAddress
{
    IPAddress localAddress;
    InterfaceId interfaceId;
};

using RouteLookupResult = Variant<UnknownRoute, NoRoute, RouteFromLocalAddress>;

/**
 * Looks up the host's route to `destination`. `interfaceId` is the interface that a destination needing one, such as a
 * link-local address, is on; it may be InterfaceId::Null().
 *
 * The answer reflects the host's routes at the time of the call, and can change later, for example once a prefix is
 * learned from a Router Advertisement.
 */
RouteLookupResult LookUpRouteTo(const IPAddress & destination, InterfaceId interfaceId);

} // namespace Inet
} // namespace chip
