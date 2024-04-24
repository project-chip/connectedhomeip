/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryDryerControls {

/**
 * @brief LaundryDryerControls Server Plugin class
 */
class LaundryDryerControlsServer : public AttributeAccessInterface
{
public:
    LaundryDryerControlsServer() : AttributeAccessInterface(Optional<EndpointId>::Missing(), LaundryDryerControls::Id) {}
    static LaundryDryerControlsServer & Instance();

    /**
     * Set the default delegate of laundry dryer server at endpoint x
     * @param endpoint ID of the endpoint
     * @param delegate The default delegate at the endpoint
     */
    static void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

    /**
     * API to set/get the SelectedDrynessLevel attribute
     */
    Protocols::InteractionModel::Status SetSelectedDrynessLevel(EndpointId endpointId, DrynessLevelEnum newSelectedDrynessLevel);
    Protocols::InteractionModel::Status GetSelectedDrynessLevel(EndpointId endpointId,
                                                                DataModel::Nullable<DrynessLevelEnum> & selectedDrynessLevel);

private:
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR ReadSupportedDrynessLevels(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);

    static LaundryDryerControlsServer sInstance;
};

} // namespace LaundryDryerControls
} // namespace Clusters
} // namespace app
} // namespace chip
