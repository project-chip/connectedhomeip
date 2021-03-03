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

#include "Resolver.h"

#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Mdns {
namespace {

class NoneResolver : public Resolver
{
public:
    CHIP_ERROR SetResolverDelegate(ResolverDelegate *) override
    {
        ChipLogError(Discovery, "Failed to register mDNS resolver delegate: mDNS resolving not available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR ResolveNodeId(uint64_t nodeId, uint64_t fabricId, Inet::IPAddressType type) override
    {
        ChipLogError(Discovery, "Failed to resolve node ID: mDNS resolving not available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

NoneResolver gResolver;

} // namespace

Resolver & chip::Mdns::Resolver::Instance()
{
    return gResolver;
}
} // namespace Mdns
} // namespace chip
