/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief switch-server class
 */
class SwitchServer
{
public:
    static SwitchServer & Instance();

    /**
     * @brief
     *   Called when the latching switch is moved to a new position.
     */
    void OnSwitchLatch(EndpointId endpoint, uint8_t newPosition);

    /**
     * @brief
     *   Called when the momentary switch starts to be pressed.
     */
    void OnInitialPress(EndpointId endpoint, uint8_t newPosition);

    /**
     * @brief
     *   Called when the momentary switch has been pressed for a "long" time.
     */
    void OnLongPress(EndpointId endpoint, uint8_t newPosition);

    /**
     * @brief
     *   Called when the momentary switch has been released.
     */
    void OnShortRelease(EndpointId endpoint, uint8_t previousPosition);

    /**
     * @brief
     *   Called when the momentary switch has been released (after debouncing)
     *   after having been pressed for a long time.
     */
    void OnLongRelease(EndpointId endpoint, uint8_t previousPosition);

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, during that sequence.
     */
    void OnMultiPressOngoing(EndpointId endpoint, uint8_t newPosition, uint8_t count);

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, after it has been detected that the sequence has ended.
     */
    void OnMultiPressComplete(EndpointId endpoint, uint8_t newPosition, uint8_t count);

private:
    static SwitchServer instance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
