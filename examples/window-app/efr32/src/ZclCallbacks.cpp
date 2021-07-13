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
#include <app/CommandHandler.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/af-types.h>
#include <app/util/af.h>

using namespace ::chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
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
 * @brief Window Covering Cluster UpOrOpen Command callback
 */

bool emberAfWindowCoveringClusterUpOrOpenCallback(chip::app::CommandHandler *)
{
    EFR32_LOG("Window UpOrOpen command received");
    AppTask::Instance().Cover().Open();

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster DownOrClose Command callback
 */

bool emberAfWindowCoveringClusterDownOrCloseCallback(chip::app::CommandHandler *)
{
    EFR32_LOG("Window DownOrClose command received");
    AppTask::Instance().Cover().Close();

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster GoToLiftPercentage Command callback
 * @param liftPercentageValue
 */

bool emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::CommandHandler *, uint8_t liftPercentageValue)
{
    EFR32_LOG("Window GoToLiftPercentage command received");
    AppTask::Instance().Cover().LiftGotoPercent(liftPercentageValue);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster GoToLiftPercentage Command callback
 * @param liftPercentageValue
 * @param liftPercent100thsValue
 */

bool emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::CommandHandler *, uint8_t liftPercentageValue,
                                                            uint16_t liftPercent100thsValue)
{
    EFR32_LOG("Window GoToLiftPercentage w/ Percent100ths command received");
    AppTask::Instance().Cover().LiftGotoPercent(liftPercent100thsValue / 100);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster GoToLiftValue Command callback
 * @param liftValue
 */

bool emberAfWindowCoveringClusterGoToLiftValueCallback(chip::app::CommandHandler *, uint16_t liftValue)
{
    EFR32_LOG("Window GoToLiftValue command received");
    AppTask::Instance().Cover().LiftGotoValue(liftValue);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster GoToTiltPercentage Command callback
 * @param tiltPercentageValue
 */

bool emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::CommandHandler *, uint8_t tiltPercentageValue)
{
    EFR32_LOG("Window GoToTiltPercentage command received");
    AppTask::Instance().Cover().TiltGotoPercent(tiltPercentageValue);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster GoToTiltPercentage Command callback
 * @param tiltPercentageValue
 * @param tiltPercent100thsValue
 */

bool emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::CommandHandler *, uint8_t tiltPercentageValue,
                                                            uint16_t tiltPercent100thsValue)
{
    EFR32_LOG("Window GoToTiltPercentage w/ Percent100ths command received");
    AppTask::Instance().Cover().TiltGotoPercent(tiltPercent100thsValue / 100);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster GoToTiltValue Command callback
 * @param tiltValue
 */

bool emberAfWindowCoveringClusterGoToTiltValueCallback(chip::app::CommandHandler *, uint16_t tiltValue)
{
    EFR32_LOG("Window GoToTiltValue command received");
    AppTask::Instance().Cover().TiltGotoValue(tiltValue);

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief Window Covering Cluster StopMotion Command callback
 */

bool emberAfWindowCoveringClusterStopMotionCallback(chip::app::CommandHandler *)
{
    EFR32_LOG("Window StopMotion command received");
    AppTask::Instance().Cover().Stop();

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
