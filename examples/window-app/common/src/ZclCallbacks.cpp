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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/CommandHandler.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/util/af-types.h>
#include <app/util/af.h>

using namespace chip::app::Clusters::WindowCovering;

void emberAfPostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                        uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    if (ZCL_WINDOW_COVERING_CLUSTER_ID == clusterId)
    {
        WindowApp & app = WindowApp::Instance();
        uint16_t current;
        uint16_t target;

        switch (attributeId)
        {
        case ZCL_WC_TYPE_ATTRIBUTE_ID:
            app.PostEvent(WindowApp::Event(WindowApp::EventId::CoverTypeChange, endpoint));
            break;

        case ZCL_WC_CURRENT_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID:
            app.PostEvent(WindowApp::Event(WindowApp::EventId::LiftChanged, endpoint));
            break;

        case ZCL_WC_CURRENT_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID:
            app.PostEvent(WindowApp::Event(WindowApp::EventId::TiltChanged, endpoint));
            break;

        case ZCL_WC_TARGET_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID:
            Attributes::GetTargetPositionLiftPercent100ths(endpoint, &target);
            Attributes::GetCurrentPositionLiftPercent100ths(endpoint, &current);
            if (current > target)
            {
                app.PostEvent(WindowApp::Event(WindowApp::EventId::LiftDown, endpoint));
            }
            else if (current < target)
            {
                app.PostEvent(WindowApp::Event(WindowApp::EventId::LiftUp, endpoint));
            }
            break;

        case ZCL_WC_TARGET_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID:
            Attributes::GetTargetPositionTiltPercent100ths(endpoint, &target);
            Attributes::GetCurrentPositionTiltPercent100ths(endpoint, &current);
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
    else
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: %ld", clusterId);
    }
}

/**
 * @brief  Cluster StopMotion Command callback (from client)
 */
bool emberAfWindowCoveringClusterStopMotionCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj)
{
    ChipLogProgress(Zcl, "StopMotion command received");
    WindowApp::Instance().PostEvent(WindowApp::Event(WindowApp::EventId::StopMotion, endpoint));
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
