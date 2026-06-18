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

#include <devices/endpoint-allocator/EndpointIdAllocator.h>
#include <set>

namespace chip::app {

/// Allocates non-colliding endpoint IDs while respecting a set of used or reserved IDs.
///
/// Maintains an invariant where `mNext` is always the next returned value. `ForceNext()`
/// can be used to set `mNext` directly for user-requested endpoint IDs.
class DynamicEndpointIdAllocator : public EndpointIdAllocator
{
public:
    explicit DynamicEndpointIdAllocator(std::set<EndpointId> reservedIds = {});

    /// Forces mNext to the given value, even if it was marked reserved or used.
    void ForceNext(EndpointId endpoint);

    EndpointId Allocate() override;

private:
    void Advance();

    std::set<EndpointId> mUsedIds;
    EndpointId mNext;
};

} // namespace chip::app
