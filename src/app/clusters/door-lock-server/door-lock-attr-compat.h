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

#include <app/data-model/Nullable.h>
#include <clusters/DoorLock/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/Constants.h>

using chip::app::DataModel::Nullable;

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {
namespace Attributes {

// The generated ember `Attributes::X::Get/Set` accessors are not produced for
// code-driven clusters (CodeDrivenClusters in config-data.yaml). The functions
// below are a compatibility surface, living in the same namespaces so legacy
// application integrations keep compiling: they are implemented in
// CodegenIntegration.cpp and route state access into the code-driven cluster.
//
// Attributes owned by the fixed ZAP configuration (lock type, feature map,
// capacities, supported modes) accept the legacy writes as no-ops: their value
// comes from the endpoint configuration, so applications needing a value other
// than the ZAP default have to use DoorLock::ApplyServerConfigOverrides.

namespace LockState {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<DlLockState> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, DlLockState value);
} // namespace LockState

namespace RequirePINforRemoteOperation {
Protocols::InteractionModel::Status Get(EndpointId endpoint, bool * value);
} // namespace RequirePINforRemoteOperation

namespace LockType {
Protocols::InteractionModel::Status Set(EndpointId endpoint, DlLockType value);
} // namespace LockType

namespace FeatureMap {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value);
} // namespace FeatureMap

namespace NumberOfTotalUsersSupported {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value);
} // namespace NumberOfTotalUsersSupported

namespace NumberOfPINUsersSupported {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value);
} // namespace NumberOfPINUsersSupported

namespace NumberOfRFIDUsersSupported {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value);
} // namespace NumberOfRFIDUsersSupported

namespace NumberOfCredentialsSupportedPerUser {
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value);
} // namespace NumberOfCredentialsSupportedPerUser

namespace OperatingMode {
Protocols::InteractionModel::Status Set(EndpointId endpoint, OperatingModeEnum value);
} // namespace OperatingMode

namespace SupportedOperatingModes {
Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::BitMask<DlSupportedOperatingModes> value);
} // namespace SupportedOperatingModes

} // namespace Attributes
} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
