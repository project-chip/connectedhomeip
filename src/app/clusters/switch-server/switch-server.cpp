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

#include "switch-server.h"
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

namespace chip {
namespace app {
namespace Clusters {

SwitchServer SwitchServer::instance;

/**********************************************************
 * SwitchServer Implementation
 *********************************************************/

SwitchServer & SwitchServer::Instance()
{
    return instance;
}

void SwitchServer::OnSwitchLatch(EndpointId endpoint, uint8_t newPosition)
{
    ChipLogProgress(Zcl, "SwitchServer: OnSwitchLatch");

    // Record SwitchLatched event
    EventNumber eventNumber;
    Events::SwitchLatched::Type event{ newPosition };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SwitchServer: Failed to record SwitchLatched event");
    }
}

void SwitchServer::OnInitialPress(EndpointId endpoint, uint8_t newPosition)
{
    ChipLogProgress(Zcl, "SwitchServer: OnInitialPress");

    // Record InitialPress event
    EventNumber eventNumber;
    Events::InitialPress::Type event{ newPosition };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SwitchServer: Failed to record InitialPress event");
    }
}

void SwitchServer::OnLongPress(EndpointId endpoint, uint8_t newPosition)
{
    ChipLogProgress(Zcl, "SwitchServer: OnLongPress");

    // Record LongPress event
    EventNumber eventNumber;
    Events::LongPress::Type event{ newPosition };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SwitchServer: Failed to record LongPress event");
    }
}

void SwitchServer::OnShortRelease(EndpointId endpoint, uint8_t previousPosition)
{
    ChipLogProgress(Zcl, "SwitchServer: OnShortRelease");

    // Record ShortRelease event
    EventNumber eventNumber;
    Events::ShortRelease::Type event{ previousPosition };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SwitchServer: Failed to record ShortRelease event");
    }
}

void SwitchServer::OnLongRelease(EndpointId endpoint, uint8_t previousPosition)
{
    ChipLogProgress(Zcl, "SwitchServer: OnLongRelease");

    // Record LongRelease event
    EventNumber eventNumber;
    Events::LongRelease::Type event{ previousPosition };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SwitchServer: Failed to record LongRelease event");
    }
}

void SwitchServer::OnMultiPressOngoing(EndpointId endpoint, uint8_t newPosition, uint8_t count)
{
    ChipLogProgress(Zcl, "SwitchServer: OnMultiPressOngoing");

    // Record MultiPressOngoing event
    EventNumber eventNumber;
    Events::MultiPressOngoing::Type event{ newPosition, count };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SwitchServer: Failed to record MultiPressOngoing event");
    }
}

void SwitchServer::OnMultiPressComplete(EndpointId endpoint, uint8_t previousPosition, uint8_t count)
{
    ChipLogProgress(Zcl, "SwitchServer: OnMultiPressComplete");

    // Record MultiPressComplete event
    EventNumber eventNumber;
    Events::MultiPressComplete::Type event{ previousPosition, count };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "SwitchServer: Failed to record MultiPressComplete event");
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterSwitchPluginServerInitCallback() {}
