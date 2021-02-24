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

#include <cstdint>

#include <core/CHIPError.h>
#include <inet/IPAddress.h>

namespace chip {
namespace Mdns {

struct ResolvedNodeData
{
    Inet::IPAddress mAddress;
    Inet::IPAddressType mAddressType;
    uint16_t mPort;
};

/// Groups callbacks for CHIP service resolution requests
class ResolverDelegate
{
public:
    virtual ~ResolverDelegate() {}

    /// Called when a requested CHIP node has been successfully resolved or the request has failed
    virtual void OnNodeIdResolved(CHIP_ERROR error, uint64_t nodeId, const ResolvedNodeData & nodeData) = 0;
};

/// Interface for resolving CHIP services
class Resolver
{
public:
    virtual ~Resolver() {}

    /// Registers a resolver delegate if none has been registered before
    virtual CHIP_ERROR SetResolverDelegate(ResolverDelegate * delegate);

    /// Requests resolution of a node ID to its address
    virtual CHIP_ERROR ResolveNodeId(uint64_t nodeId, uint64_t fabricId, Inet::IPAddressType type);

    /// Provides the system-wide implementation of the service resolver
    static Resolver & Instance();
};

} // namespace Mdns
} // namespace chip
