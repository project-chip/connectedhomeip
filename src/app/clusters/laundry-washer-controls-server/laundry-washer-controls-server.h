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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>
#include <app/AttributeAccessInterface.h>
#include "LaundryWasherDataProvider.h"

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
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    static LaundryWasherControlsServer & Instance();

    using SpinSpeedList = DataModel::List<const chip::CharSpan>;
    using SupportedRinsesList = DataModel::List<const NumberOfRinsesEnum>;

    /**
     * Init the laundry washer server.
     * @param void
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR Init();

    /**
     * @brief Set the attribute newSpinSpeedCurrent
     *
     * @param endpointId ID of the endpoint
     * @param newSpinSpeedCurrent attribute SpinSpeedCurrent
     * @return true on success, false on failure
     */
    EmberAfStatus SetSpinSpeedCurrent(chip::EndpointId endpointId, DataModel::Nullable<uint8_t> newSpinSpeedCurrent);

    /**
     * @brief Get the attribute newSpinSpeedCurrent
     *
     * @param endpointId ID of the endpoint
     * @param SpinSpeedCurrent attribute SpinSpeedCurrent
     * @return true on success, false on failure
     */
    EmberAfStatus GetSpinSpeedCurrent(chip::EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent);

    /**
     * @brief Set the attribute NumberOfRinses
     *
     * @param endpointId ID of the endpoint
     * @param newNumberOfRinses attribute NumberOfRinses
     * @return true on success, false on failure
     */
    EmberAfStatus SetNumberOfRinses(chip::EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses);

    /**
     * @brief Get the attribute NumberOfRinses
     *
     * @param endpointId ID of the endpoint
     * @param NumberOfRinses attribute NumberOfRinses
     * @return true on success, false on failure
     */
    EmberAfStatus GetNumberOfRinses(chip::EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses);

    /**
     * Set spin speed list.
     * @param endpointId ID of the endpoint
     * @param spinSpeedList The spin speed list for which to save.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    EmberAfStatus SetSpinSpeedList(EndpointId endpointId, const SpinSpeedList & spinSpeedList);

    /**
     * Get spin speed list.
     * @param endpointId ID of the endpoint
     * @param spinSpeedList The pointer to load spin speed list.
     * @param size The number of phase list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    EmberAfStatus GetSpinSpeedList(EndpointId endpointId, SpinSpeedListCharSpan ** spinSpeedList, size_t & size);

    /**
     * Set supportd rinses list.
     * @param endpointId ID of the endpoint
     * @param SupportedRinsesList The supported rinses list for which to save.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    EmberAfStatus SetSupportedRinsesList(EndpointId endpointId, const SupportedRinsesList & supportedRinsesList);

    /**
     * Get supported rinses list.
     * @param endpointId ID of the endpoint
     * @param supportedRinsesList The pointer to load supported rinses list.
     * @param size The number of phase list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    EmberAfStatus GetSupportedRinsesList(EndpointId endpointId, SupportedRinsesListSpan ** supportedRinsesList, size_t & size);
private:
    /**
     * Rlease SpinSpeedListCharSpan
     * @param spinSpeedList The pointer for which to clear the SpinSpeedListCharSpan.
     * @return void
     */
    void ReleaseSpinSpeedList(SpinSpeedListCharSpan * spinSpeedList);

    /**
     * Clear spin speed list.
     * @param endpointId ID of the endpoint
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearSpinSpeedList(EndpointId endpointId);

    /**
     * Rlease SupportedRinsesListSpan
     * @param supportedRinsesList The pointer for which to clear the SupportedRinsesListSpan.
     * @return void
     */
    void ReleaseSupportedRinsesList(SupportedRinsesListSpan* supportedRinsesList);

    /**
     * Clear supported rinses list.
     * @param endpointId ID of the endpoint
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearSupportedRinsesList(EndpointId endpointId);

    CHIP_ERROR ReadSpinSpeeds(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedRinses(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);

    LaundryWasherDataProvider mLaundryWasherDataProvider;

    static LaundryWasherControlsServer sInstance;
};

} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
