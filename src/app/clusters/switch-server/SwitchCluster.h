/*
 *    Copyright (c) 2021-2026 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/Switch/Attributes.h>

namespace chip::app::Clusters {

class SwitchCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<Switch::Attributes::MultiPressMax::Id>;

    struct StartupConfiguration
    {
        uint8_t multiPressMax{};
    };

    SwitchCluster(EndpointId endpointId, const BitFlags<Switch::Feature> features,
                  const OptionalAttributeSet & optionalAttributeSet, const StartupConfiguration & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetNumberOfPositions(uint8_t numberOfPositions);
    uint8_t GetNumberOfPositions() const { return mNumberOfPositions; }

    CHIP_ERROR SetCurrentPosition(uint8_t currentPosition);
    uint8_t GetCurrentPosition() const { return mCurrentPosition; }

    /**
     * @brief
     *   Called when the latching switch is moved to a new position.
     */
    void OnSwitchLatch(uint8_t newPosition);

    /**
     * @brief
     *   Called when the momentary switch starts to be pressed.
     */
    void OnInitialPress(uint8_t newPosition);

    /**
     * @brief
     *   Called when the momentary switch has been pressed for a "long" time.
     */
    void OnLongPress(uint8_t newPosition);

    /**
     * @brief
     *   Called when the momentary switch has been released.
     */
    void OnShortRelease(uint8_t previousPosition);

    /**
     * @brief
     *   Called when the momentary switch has been released (after debouncing)
     *   after having been pressed for a long time.
     */
    void OnLongRelease(uint8_t previousPosition);

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, during that sequence.
     */
    void OnMultiPressOngoing(uint8_t newPosition, uint8_t count);

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, after it has been detected that the sequence has ended.
     */
    void OnMultiPressComplete(uint8_t newPosition, uint8_t count);

protected:
    const BitFlags<Switch::Feature> mFeatures;
    const OptionalAttributeSet mOptionalAttributeSet;
    const StartupConfiguration mConfig;
    uint8_t mNumberOfPositions{};
    uint8_t mCurrentPosition{};
};

} // namespace chip::app::Clusters
