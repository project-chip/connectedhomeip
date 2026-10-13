/**
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

#include <app/clusters/door-lock-server/DoorLockCluster.h>
#include <app/data-model/Nullable.h>
#include <lib/support/BitFlags.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

/// Application-provided runtime overrides of the code-driven DoorLockCluster
/// configuration (all optional: unset fields keep the values derived from the
/// ZAP configuration).
///
/// Must be applied during application init, before the Door Lock cluster init
/// callback (which runs during endpoint initialization) consumes them.
struct ServerConfigOverrides
{
    std::optional<BitFlags<Feature>> features;
    std::optional<uint16_t> numberOfTotalUsersSupported;
    std::optional<uint16_t> numberOfPINUsersSupported;
    std::optional<uint16_t> numberOfRFIDUsersSupported;
    std::optional<uint8_t> numberOfCredentialsSupportedPerUser;
    std::optional<uint32_t> autoRelockTime;
    std::optional<DataModel::Nullable<DlLockState>> lockState;
};

/// Installs the cluster delegate (hardware actuation, Aliro provisioning) for
/// the given endpoint. The delegate must outlive the cluster on that endpoint.
/// Must be called before the Door Lock cluster is created for the endpoint,
/// i.e. during application init, before endpoint initialization runs.
///
/// Endpoints without a delegate use a neutral default implementation
/// (actuation fails, Aliro attributes report null/not-configured).
void SetDelegate(EndpointId endpointId, Delegate * delegate);

/// Applies runtime configuration overrides for the given endpoint.
void ApplyServerConfigOverrides(EndpointId endpointId, const ServerConfigOverrides & overrides);

DoorLockCluster * FindClusterOnEndpoint(EndpointId endpointId);

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip