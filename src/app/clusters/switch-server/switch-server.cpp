/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <platform/DeviceControlServer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::app::Clusters::Switch::Attributes;
using chip::DeviceLayer::DeviceControlServer;

namespace {

class SwitchDelegate : public DeviceLayer::SwitchDeviceControlDelegate
{
    /**
     * @brief
     *   Called when the latching switch is moved to a new position.
     */
    void OnSwitchLatched(uint8_t newPosition) override
    {
        ChipLogProgress(Zcl, "SwitchDelegate: OnSwitchLatched");

        for (auto endpointId : EnabledEndpointsWithServerCluster(Switch::Id))
        {
            // Record SwitchLatched event
            EventNumber eventNumber;
            Events::SwitchLatched::Type event{ newPosition };

            if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
            {
                ChipLogError(Zcl, "SwitchDelegate: Failed to record SwitchLatched event");
            }
        }
    }

    /**
     * @brief
     *   Called when the momentary switch starts to be pressed.
     */
    void OnInitialPressed(uint8_t newPosition) override
    {
        ChipLogProgress(Zcl, "SwitchDelegate: OnInitialPressed");

        for (auto endpointId : EnabledEndpointsWithServerCluster(Switch::Id))
        {
            // Record InitialPress event
            EventNumber eventNumber;
            Events::InitialPress::Type event{ newPosition };

            if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
            {
                ChipLogError(Zcl, "SwitchDelegate: Failed to record InitialPress event");
            }
        }
    }

    /**
     * @brief
     *   Called when the momentary switch has been pressed for a "long" time.
     */
    void OnLongPressed(uint8_t newPosition) override
    {
        ChipLogProgress(Zcl, "SwitchDelegate: OnLongPressed");

        for (auto endpointId : EnabledEndpointsWithServerCluster(Switch::Id))
        {
            // Record LongPress event
            EventNumber eventNumber;
            Events::LongPress::Type event{ newPosition };

            if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
            {
                ChipLogError(Zcl, "SwitchDelegate: Failed to record LongPress event");
            }
        }
    }

    /**
     * @brief
     *   Called when the momentary switch has been released.
     */
    void OnShortReleased(uint8_t previousPosition) override
    {
        ChipLogProgress(Zcl, "SwitchDelegate: OnShortReleased");

        for (auto endpointId : EnabledEndpointsWithServerCluster(Switch::Id))
        {
            // Record ShortRelease event
            EventNumber eventNumber;
            Events::ShortRelease::Type event{ previousPosition };

            if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
            {
                ChipLogError(Zcl, "SwitchDelegate: Failed to record ShortRelease event");
            }
        }
    }

    /**
     * @brief
     *   Called when the momentary switch has been released (after debouncing)
     *   and after having been pressed for a long time.
     */
    void OnLongReleased(uint8_t previousPosition) override
    {
        ChipLogProgress(Zcl, "SwitchDelegate: OnLongReleased");

        for (auto endpointId : EnabledEndpointsWithServerCluster(Switch::Id))
        {
            // Record LongRelease event
            EventNumber eventNumber;
            Events::LongRelease::Type event{ previousPosition };

            if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
            {
                ChipLogError(Zcl, "SwitchDelegate: Failed to record LongRelease event");
            }
        }
    }

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, during that sequence.
     */
    void OnMultiPressOngoing(uint8_t newPosition, uint8_t count) override
    {
        ChipLogProgress(Zcl, "SwitchDelegate: OnMultiPressOngoing");

        for (auto endpointId : EnabledEndpointsWithServerCluster(Switch::Id))
        {
            // Record MultiPressOngoing event
            EventNumber eventNumber;
            Events::MultiPressOngoing::Type event{ newPosition, count };

            if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
            {
                ChipLogError(Zcl, "SwitchDelegate: Failed to record MultiPressOngoing event");
            }
        }
    }

    /**
     * @brief
     *   Called to indicate how many times the momentary switch has been pressed
     *   in a multi-press sequence, after it has been detected that the sequence has ended.
     */
    void OnMultiPressComplete(uint8_t newPosition, uint8_t count) override
    {
        ChipLogProgress(Zcl, "SwitchDelegate: OnMultiPressComplete");

        for (auto endpointId : EnabledEndpointsWithServerCluster(Switch::Id))
        {
            // Record MultiPressComplete event
            EventNumber eventNumber;
            Events::MultiPressComplete::Type event{ newPosition, count };

            if (CHIP_NO_ERROR != LogEvent(event, endpointId, eventNumber))
            {
                ChipLogError(Zcl, "SwitchDelegate: Failed to record MultiPressComplete event");
            }
        }
    }
};

SwitchDelegate gSwitchDelegate;

} // anonymous namespace

void MatterSwitchPluginServerInitCallback()
{
    DeviceControlServer::DeviceControlSvr().SetSwitchDelegate(&gSwitchDelegate);
}
