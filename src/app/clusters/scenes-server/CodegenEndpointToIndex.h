/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/util/attribute-storage.h>

namespace chip::scenes {

/// Converts an endpoint id into a 0-based index using codegen functions.
///
/// @tparam ClusterId The cluster ID value used for endpoint indexing
/// @tparam FixedEndpointCount The number of fixed endpoints for this cluster in codegen/ember
/// @tparam MaxIndexCount The maximum number of indexes (return value on success will always be less than this value)
template <ClusterId ClusterId, size_t FixedEndpointCount, size_t MaxIndexCount>
struct CodegenEndpointToIndex
{
    static constexpr size_t kMaxEndpointCount = MaxIndexCount;

    static bool EndpointIdToIndex(EndpointId endpointId, uint16_t & index)
    {
        uint16_t idx = emberAfGetClusterServerEndpointIndex(endpointId, ClusterId, FixedEndpointCount);
        if (idx >= MaxIndexCount)
        {
            return false;
        }
        index = idx;
        return true;
    }
};

} // namespace chip::scenes
