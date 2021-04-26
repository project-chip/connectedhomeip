/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <AppTask.h>
#include <app/Command.h>
#include <app/server/DataModelHandler.h>
#include <app/util/af-types.h>
#include <gen/attribute-id.h>
#include <gen/cluster-id.h>

using namespace ::chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    if (clusterId != ZCL_WINDOW_COVERING_CLUSTER_ID)
    {
        EFR32_LOG("Unknown cluster ID: %d", clusterId);
    }
}

/** @brief Window Covering Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfWindowCoveringClusterInitCallback(chip::EndpointId endpoint)
{
    EFR32_LOG("Window Covering Cluster init");
}

/**
 * @brief Window Covering Cluster WindowCoveringUpOpen Command callback
 */

bool emberAfWindowCoveringClusterWindowCoveringUpOpenCallback(chip::app::Command *)
{
    EFR32_LOG("Window Up Open command received");
    AppTask::Instance().Cover().Open();
    return true;
}

/**
 * @brief Window Covering Cluster WindowCoveringDownClose Command callback
 */

bool emberAfWindowCoveringClusterWindowCoveringDownCloseCallback(chip::app::Command *)
{
    EFR32_LOG("Window Down Close command received");
    AppTask::Instance().Cover().Close();
    return true;
}

/**
 * @brief Window Covering Cluster WindowCoveringGoToLiftPercentage Command callback
 * @param percentageLiftValue
 */

bool emberAfWindowCoveringClusterWindowCoveringGoToLiftPercentageCallback(chip::app::Command *, uint8_t percentageLiftValue)
{
    EFR32_LOG("Window Go To Lift Percentage command received");
    AppTask::Instance().Cover().LiftGotoPercent(percentageLiftValue);
    return true;
}

/**
 * @brief Window Covering Cluster WindowCoveringGoToLiftValue Command callback
 * @param liftValue
 */

bool emberAfWindowCoveringClusterWindowCoveringGoToLiftValueCallback(chip::app::Command *, uint16_t liftValue)
{
    EFR32_LOG("Window Go To Lift Value command received");
    AppTask::Instance().Cover().LiftGotoValue(liftValue);
    return true;
}

/**
 * @brief Window Covering Cluster WindowCoveringGoToTiltPercentage Command callback
 * @param percentageTiltValue
 */

bool emberAfWindowCoveringClusterWindowCoveringGoToTiltPercentageCallback(chip::app::Command *, uint8_t percentageTiltValue)
{
    EFR32_LOG("Window Go To Tilt Percentage command received");
    AppTask::Instance().Cover().TiltGotoPercent(percentageTiltValue);
    return true;
}

/**
 * @brief Window Covering Cluster WindowCoveringGoToTiltValue Command callback
 * @param tiltValue
 */

bool emberAfWindowCoveringClusterWindowCoveringGoToTiltValueCallback(chip::app::Command *, uint16_t tiltValue)
{
    EFR32_LOG("Window Go To Tilt Value command received");
    AppTask::Instance().Cover().TiltGotoValue(tiltValue);
    return true;
}

/**
 * @brief Window Covering Cluster WindowCoveringStop Command callback
 */

bool emberAfWindowCoveringClusterWindowCoveringStopCallback(chip::app::Command *)
{
    EFR32_LOG("Window Stop command received");
    AppTask::Instance().Cover().Stop();
    return true;
}
