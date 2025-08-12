/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <data-model-providers/codegen/ClusterIntegration.h>

namespace chip::app {

// FIXME: implement the following:
//
//   - use emberAfGetClusterServerEndpointIndex for multi-cluster support (including logging)
//   - use emberAfContainsAttribute to determine what attributes are enabled
//   - use Registry Register/Unregister logic (including logging)
//   - determine if we can get the raw feature map
//       - if FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
//         requires the FeatureMap to NOT be static. How to do this?
//
//
//         using Traits = NumericAttributeTraits<uint32_t>;
//         Traits::StorageType temp;
//         uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
//         Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::Identify::Id, Id, readable,
//         sizeof(temp)); VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status); if
//         (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
//         {
//             return Protocols::InteractionModel::Status::ConstraintError;
//         }
//         *value = Traits::StorageToWorking(temp);
//         return status;
//

void CodegenClusterIntegration::RegisterServer(const RegisterServerOptions &options, Delegate & delegate) {
    // FIXME: implement
}

void CodegenClusterIntegration::UnregisterServer(const UnregisterServerOptions &options, Delegate &delegate) {
    // FIXME: implement
}

} // namespace chip::app
