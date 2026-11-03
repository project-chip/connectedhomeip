/*
 *
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

#include <array>
#include <cstddef>

#include <app/clusters/general-commissioning-server/GeneralCommissioningCluster.h>
#include <clusters/Descriptor/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/root-node/RootNode.h>
#include <devices/Types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

/// Feature policy contract used by `RootNodeWith<Features...>`.
///
/// Each feature policy is a small value type that owns whatever additional
/// clusters live on the root endpoint for that feature. A feature MUST expose:
///
///   struct Context { ... };                     // per-feature construction inputs
///   explicit Feature(const Context &);          // constructor
///
///   static constexpr std::array<DataModel::DeviceTypeEntry, N> kExtraDeviceTypes;
///   static constexpr std::array<ClusterId, M>                  kExtraClientClusters;
///
///   CHIP_ERROR RegisterFeatureClusters(EndpointId endpointId,
///                                      CodeDrivenDataModelProvider & provider,
///                                      RootNode::Context & rootContext,
///                                      Clusters::GeneralCommissioningCluster & generalCommissioning);
///   void UnregisterFeatureClusters(CodeDrivenDataModelProvider & provider);
///
/// Both `kExtraDeviceTypes` and `kExtraClientClusters` may be zero-sized. When
/// a feature has no clusters of its own (only extra device types / client
/// clusters, like OTA Requestor), its Register/Unregister methods are no-ops.
///
/// The composite device-type array and client-cluster array are assembled by
/// `RootNodeWith` at compile time via `aggregate::ConcatArrays`.

namespace aggregate {

/// Concatenate two `std::array` instances of the same element type.
template <typename T, std::size_t N1, std::size_t N2>
constexpr std::array<T, N1 + N2> ConcatArrays(const std::array<T, N1> & a, const std::array<T, N2> & b)
{
    std::array<T, N1 + N2> out{};
    for (std::size_t i = 0; i < N1; ++i)
    {
        out[i] = a[i];
    }
    for (std::size_t i = 0; i < N2; ++i)
    {
        out[N1 + i] = b[i];
    }
    return out;
}

/// Fold N `std::array`s into a single concatenated array (compile time).
template <typename T, std::size_t N>
constexpr auto ConcatArrays(const std::array<T, N> & a)
{
    return a;
}

template <typename T, std::size_t N, typename... Rest>
constexpr auto ConcatArrays(const std::array<T, N> & a, const Rest &... rest)
{
    return ConcatArrays(a, ConcatArrays(rest...));
}

} // namespace aggregate
} // namespace app
} // namespace chip
