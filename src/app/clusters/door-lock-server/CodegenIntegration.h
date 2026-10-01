/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/door-lock-server/DoorLockCluster.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

/// Application-defined adjustments to the ZAP-derived per-endpoint cluster
/// configuration. Values set here take priority over the static application
/// configuration defaults used by the generated `.matter` files.
///
/// The legacy ember implementation applied runtime overrides by writing ember
/// attributes at endpoint init (feature map, user capacities, auto relock
/// time...). Attributes are no longer ember-served, so applications provide
/// the same values through this struct instead.
struct ServerConfigOverrides
{
    std::optional<BitFlags<Feature>> features;

    // Capacities (only meaningful for the corresponding enabled features).
    std::optional<uint16_t> numberOfTotalUsersSupported;
    std::optional<uint16_t> numberOfPINUsersSupported;
    std::optional<uint16_t> numberOfRFIDUsersSupported;
    std::optional<uint8_t> numberOfCredentialsSupportedPerUser;

    std::optional<uint32_t> autoRelockTime;
    std::optional<DataModel::Nullable<DlLockState>> lockState;
};

/// Registers the delegate used for hardware actuation and Aliro support on the
/// given endpoint. Must be called before the endpoint (with this cluster) is
/// initialized. When no delegate is set, a stub answering with default values
/// is used.
void SetDelegate(EndpointId endpointId, Delegate * delegate);

/// Sets configuration overrides for the given endpoint. Must be called before
/// the cluster instance is created (before endpoint initialization).
void ApplyServerConfigOverrides(EndpointId endpointId, const ServerConfigOverrides & overrides);

/// Returns the DoorLockCluster serving `endpointId`, or nullptr.
DoorLockCluster * FindClusterOnEndpoint(EndpointId endpointId);

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip