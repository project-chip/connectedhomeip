/**
 *
 *    Copyright (c) 2023 - 2026 Project CHIP Authors
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

#include "laundry-dryer-controls-delegate.h"

#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/LaundryDryerControls/Attributes.h>

namespace chip {
namespace app {
namespace Clusters {

class LaundryDryerControlsCluster : public DefaultServerCluster
{
public:
    /**
     * Creates a LaundryDryerControls Cluster instance.
     * @param aEndpointId The endpoint on which this cluster exists.
     * @param aDelegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    LaundryDryerControlsCluster(EndpointId endpointId, LaundryDryerControls::Delegate & delegate);
    ~LaundryDryerControlsCluster();

    // Attribute Setters and Getters
    // SelectedDrynessLevel
    Protocols::InteractionModel::Status
    SetSelectedDrynessLevel(DataModel::Nullable<LaundryDryerControls::DrynessLevelEnum> drynessLevel);

    // Attribute Getters
    // SelectedDrynessLevel
    DataModel::Nullable<LaundryDryerControls::DrynessLevelEnum> GetSelectedDrynessLevel() const;

    /**
     * @brief ServerClusterInterface methods.
     */
    CHIP_ERROR Startup(ServerClusterContext & context) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

private:
    LaundryDryerControls::Delegate & mDelegate;

    // Attribute local storage
    DataModel::Nullable<LaundryDryerControls::DrynessLevelEnum> mSelectedDrynessLevel{};

    // Helpers
    // Loads all the persistent attributes from the KVS.
    void LoadPersistentAttributes();

    // Encodes the SupportedDrynessLevels list from the delegate.
    CHIP_ERROR EncodeSupportedDrynessLevels(const AttributeValueEncoder::ListEncodeHelper & encoder);

    // Checks whether the given dryness level is advertised by the delegate's SupportedDrynessLevels list.
    bool IsSupportedDrynessLevel(LaundryDryerControls::DrynessLevelEnum drynessLevel);
};

} // namespace Clusters
} // namespace app
} // namespace chip
