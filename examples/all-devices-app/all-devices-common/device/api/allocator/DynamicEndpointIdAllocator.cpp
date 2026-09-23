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

#include "DynamicEndpointIdAllocator.h"

#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <utility>

namespace chip::app {

DynamicEndpointIdAllocator::DynamicEndpointIdAllocator(std::set<EndpointId> reservedIds) :
    mUsedIds(std::move(reservedIds)), mNext(1)
{
    mUsedIds.insert(kRootEndpointId);
    Advance();
}

void DynamicEndpointIdAllocator::Advance()
{
    while (mNext < kInvalidEndpointId && mUsedIds.count(mNext))
    {
        mNext++;
    }
}

void DynamicEndpointIdAllocator::ForceNext(EndpointId endpoint)
{
    if (endpoint != kInvalidEndpointId)
    {
        mNext = endpoint;
    }
}

EndpointId DynamicEndpointIdAllocator::Allocate()
{
    // Exhaustion of the 16-bit endpoint ID space is treated as a fatal configuration error.
    VerifyOrDie(mNext != kInvalidEndpointId);
    EndpointId allocated = mNext++;
    mUsedIds.insert(allocated);
    Advance();
    return allocated;
}

} // namespace chip::app
