/*
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include <AppConfig.h>
#include <WindowApp.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/util/af.h>

using namespace ::chip;
using namespace ::chip::app::Clusters::WindowCovering;

void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if (attributePath.mClusterId == app::Clusters::Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify cluster ID: " ChipLogFormatMEI " Type: %" PRIu8 " Value: %" PRIu16 ", length %" PRIu16,
                        ChipLogValueMEI(attributePath.mAttributeId), type, *value, size);
    }
    else if (attributePath.mClusterId == Id)
    {
        ChipLogProgress(Zcl, "Window  cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(attributePath.mClusterId));
    }
    else
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(attributePath.mClusterId));
    }

    WindowApp & app     = WindowApp::Instance();
    EndpointId endpoint = attributePath.mEndpointId;
    uint16_t current;
    uint16_t target;

    switch (attributePath.mAttributeId)
    {
    case Attributes::Type::Id:
        app.PostEvent(WindowApp::Event(WindowApp::EventId::CoverTypeChange, endpoint));
        break;

    case Attributes::CurrentPositionLiftPercent100ths::Id:
        app.PostEvent(WindowApp::Event(WindowApp::EventId::LiftChanged, endpoint));
        break;

    case Attributes::CurrentPositionTiltPercent100ths::Id:
        app.PostEvent(WindowApp::Event(WindowApp::EventId::TiltChanged, endpoint));
        break;

    case Attributes::TargetPositionLiftPercent100ths::Id:
        Attributes::TargetPositionLiftPercent100ths::Get(endpoint, &target);
        Attributes::CurrentPositionLiftPercent100ths::Get(endpoint, &current);
        if (current > target)
        {
            app.PostEvent(WindowApp::Event(WindowApp::EventId::LiftDown, endpoint));
        }
        else if (current < target)
        {
            app.PostEvent(WindowApp::Event(WindowApp::EventId::LiftUp, endpoint));
        }
        break;

    case Attributes::TargetPositionTiltPercent100ths::Id:
        Attributes::TargetPositionTiltPercent100ths::Get(endpoint, &target);
        Attributes::CurrentPositionTiltPercent100ths::Get(endpoint, &current);
        if (current > target)
        {
            app.PostEvent(WindowApp::Event(WindowApp::EventId::TiltDown, endpoint));
        }
        else if (current < target)
        {
            app.PostEvent(WindowApp::Event(WindowApp::EventId::TiltUp, endpoint));
        }
        break;

    default:
        break;
    }
}

/**
 * @brief  Cluster StopMotion Command callback (from client)
 */
bool emberAfWindowCoveringClusterStopMotionCallback(chip::app::CommandHandler * commandObj,
                                                    const chip::app::ConcreteCommandPath & commandPath,
                                                    const Commands::StopMotion::DecodableType & commandData)
{
    ChipLogProgress(Zcl, "StopMotion command received");
    WindowApp::Instance().PostEvent(WindowApp::Event(WindowApp::EventId::StopMotion, commandPath.mEndpointId));
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
