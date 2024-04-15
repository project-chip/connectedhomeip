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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/att-storage.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::DeviceLayer;

class SwitchEventHandler
{
public:
    SwitchEventHandler(EndpointId endpoint):mEndpointId(endpoint){};

    /**
     * Should be called when the latching switch is moved to a new position.
     */
    void OnSwitchLatchedHandler(uint8_t newPosition);

    /**
     * Should be called when the momentary switch starts to be pressed.
     */
    void OnSwitchInitialPressedHandler(uint8_t newPosition);

    /**
     * Should be called when the momentary switch has been pressed for a "long" time.
     */
    void OnSwitchLongPressedHandler(uint8_t newPosition);

    /**
     * Should be called when the momentary switch has been released.
     */
    void OnSwitchShortReleasedHandler(uint8_t previousPosition);

    /**
     * Should be called when the momentary switch has been released after having been pressed for a long time.
     */
    void OnSwitchLongReleasedHandler(uint8_t previousPosition);

    /**
     * Should be called to indicate how many times the momentary switch has been pressed in a multi-press
     * sequence, during that sequence.
     */
    void OnSwitchMultiPressOngoingHandler(uint8_t newPosition, uint8_t count);

    /**
     * Should be called to indicate how many times the momentary switch has been pressed in a multi-press
     * sequence, after it has been detected that the sequence has ended.
     */
    void OnSwitchMultiPressCompleteHandler(uint8_t previousPosition, uint8_t count);

private:
    EndpointId mEndpointId;
};


void SwitchEventHandler::OnSwitchLatchedHandler(uint8_t newPosition)
{
    ChipLogDetail(NotSpecified, "The latching switch is moved to a new position:%d", newPosition);

    Clusters::SwitchServer::Instance().OnSwitchLatch(mEndpointId, newPosition);
}

void SwitchEventHandler::OnSwitchInitialPressedHandler(uint8_t newPosition)
{
    ChipLogDetail(NotSpecified, "The new position when the momentary switch starts to be pressed:%d", newPosition);

    Clusters::SwitchServer::Instance().OnInitialPress(mEndpointId, newPosition);
}

void SwitchEventHandler::OnSwitchLongPressedHandler(uint8_t newPosition)
{
    ChipLogDetail(NotSpecified, "The new position when the momentary switch has been pressed for a long time:%d", newPosition);

    Clusters::SwitchServer::Instance().OnLongPress(mEndpointId, newPosition);
}

void SwitchEventHandler::OnSwitchShortReleasedHandler(uint8_t previousPosition)
{
    ChipLogDetail(NotSpecified, "The the previous value of the CurrentPosition when the momentary switch has been released:%d",
                  previousPosition);

    Clusters::SwitchServer::Instance().OnShortRelease(mEndpointId, previousPosition);
}

void SwitchEventHandler::OnSwitchLongReleasedHandler(uint8_t previousPosition)
{
    ChipLogDetail(NotSpecified,
                  "The the previous value of the CurrentPosition when the momentary switch has been released after having been "
                  "pressed for a long time:%d",
                  previousPosition);

    Clusters::SwitchServer::Instance().OnLongRelease(mEndpointId, previousPosition);
}

void SwitchEventHandler::OnSwitchMultiPressOngoingHandler(uint8_t newPosition, uint8_t count)
{
    ChipLogDetail(NotSpecified, "The new position when the momentary switch has been pressed in a multi-press sequence:%d",
                  newPosition);
    ChipLogDetail(NotSpecified, "%d times the momentary switch has been pressed", count);

    Clusters::SwitchServer::Instance().OnMultiPressOngoing(mEndpointId, newPosition, count);
}

void SwitchEventHandler::OnSwitchMultiPressCompleteHandler(uint8_t previousPosition, uint8_t count)
{
    ChipLogDetail(NotSpecified, "The previous position when the momentary switch has been pressed in a multi-press sequence:%d",
                  previousPosition);
    ChipLogDetail(NotSpecified, "%d times the momentary switch has been pressed", count);

    Clusters::SwitchServer::Instance().OnMultiPressComplete(mEndpointId, previousPosition, count);
}

static std::map<int, SwitchEventHandler *> gSwitchEventHandlers{};

SwitchEventHandler * GetSwitchEventHandler(EndpointId endpointId)
{
    if (gSwitchEventHandlers.find(endpointId) == gSwitchEventHandlers.end()) {
        return nullptr;
    }

    return gSwitchEventHandlers[endpointId];
}

void emberAfSwitchClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Chef: emberAfSwitchClusterInitCallback");
    gSwitchEventHandlers[endpoint] = new SwitchEventHandler(endpoint);
printf("\033[44m %s, %d, Switch::ID=%u \033[0m \n", __func__, __LINE__, Switch::Id);
}


