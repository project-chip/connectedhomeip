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

#include "laundry-washer-controls-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {

/**
 * @brief LaundryWasherControls Server Plugin class
 */
class LaundryWasherControlsServer : public AttributeAccessInterface
{
public:
    LaundryWasherControlsServer() : AttributeAccessInterface(Optional<EndpointId>::Missing(), LaundryWasherControls::Id) {}
    static LaundryWasherControlsServer & Instance();

    static constexpr uint8_t kMaxSpinSpeedLength = 64;

    /**
     * Set the default delegate of laundry washer server at endpoint x
     * @param endpoint ID of the endpoint
     * @param delegate The default delegate at the endpoint
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    static void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

    /**
     * Init the laundry washer server.
     * @param void
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    //    CHIP_ERROR Init();

    /**
     * @brief Set the attribute newSpinSpeedCurrent
     *
     * @param endpointId ID of the endpoint
     * @param newSpinSpeedCurrent attribute SpinSpeedCurrent
     * @return true on success, false on failure
     */
    EmberAfStatus SetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> newSpinSpeedCurrent);

    /**
     * @brief Get the attribute newSpinSpeedCurrent
     *
     * @param endpointId ID of the endpoint
     * @param SpinSpeedCurrent attribute SpinSpeedCurrent
     * @return true on success, false on failure
     */
    EmberAfStatus GetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent);

    /**
     * @brief Set the attribute NumberOfRinses
     *
     * @param endpointId ID of the endpoint
     * @param newNumberOfRinses attribute NumberOfRinses
     * @return true on success, false on failure
     */
    EmberAfStatus SetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses);

    /**
     * @brief Get the attribute NumberOfRinses
     *
     * @param endpointId ID of the endpoint
     * @param NumberOfRinses attribute NumberOfRinses
     * @return true on success, false on failure
     */
    EmberAfStatus GetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses);

private:
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR ReadSpinSpeeds(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedRinses(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);

    static LaundryWasherControlsServer sInstance;
};

} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
