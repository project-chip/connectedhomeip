/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app/clusters/switch-server/switch-server.h>

#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

namespace Switch {

class SwitchEventHandler
{
public:
    SwitchEventHandler(EndpointId endpoint):mEndpointId(endpoint){};

    /**
     * Should be called when the latching switch is moved to a new position.
     */
    void OnSwitchLatched(uint8_t newPosition);

    /**
     * Should be called when the momentary switch starts to be pressed.
     */
    void OnSwitchInitialPressed(uint8_t newPosition);

    /**
     * Should be called when the momentary switch has been pressed for a "long" time.
     */
    void OnSwitchLongPressed(uint8_t newPosition);

    /**
     * Should be called when the momentary switch has been released.
     */
    void OnSwitchShortReleased(uint8_t previousPosition);

    /**
     * Should be called when the momentary switch has been released after having been pressed for a long time.
     */
    void OnSwitchLongReleased(uint8_t previousPosition);

    /**
     * Should be called to indicate how many times the momentary switch has been pressed in a multi-press
     * sequence, during that sequence.
     */
    void OnSwitchMultiPressOngoing(uint8_t newPosition, uint8_t count);

    /**
     * Should be called to indicate how many times the momentary switch has been pressed in a multi-press
     * sequence, after it has been detected that the sequence has ended.
     */
    void OnSwitchMultiPressComplete(uint8_t previousPosition, uint8_t count);

private:
    EndpointId mEndpointId;
};


} // namespace Switch
} // namespace Clusters
} // namespace app
} // namespace chip



//class AllClustersCommandDelegate : public NamedPipeCommandDelegate
//{
//public:
//    void OnEventCommandReceived(const char * json) override;
//};
