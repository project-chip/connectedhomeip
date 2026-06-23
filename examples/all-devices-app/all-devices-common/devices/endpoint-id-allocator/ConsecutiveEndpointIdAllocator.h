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

#include <devices/endpoint-id-allocator/EndpointIdAllocator.h>

namespace chip::app {

/// Allocates consecutive endpoint IDs starting from a specified initial ID.
///
/// Intended for standalone or embedded examples that require simple incremental allocation.
class ConsecutiveEndpointIdAllocator : public EndpointIdAllocator
{
public:
    explicit ConsecutiveEndpointIdAllocator(EndpointId startEndpoint = 1) : mNext(startEndpoint) {}

    EndpointId Allocate() override { return mNext++; }

private:
    EndpointId mNext;
};

} // namespace chip::app
