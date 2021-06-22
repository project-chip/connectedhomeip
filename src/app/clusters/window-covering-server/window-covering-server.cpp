/**
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

/****************************************************************************
 * @file
 * @brief Routines for the Window Covering Server cluster
 *******************************************************************************
 ******************************************************************************/

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/cluster-id.h>

#include <app/Command.h>
#include <app/reporting/reporting.h>

#include <app/util/af.h>
#include <app/util/af-event.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

#include "window-covering-server.h"

using namespace chip;

typedef struct wcFeature
{
    /* data */
    bool LF;
    bool TL;
    bool PA;
} wcFeature_t;

/* Simulate a default featureMAP TBD later */
static wcFeature_t m_wcFeature = { true, true, true };


//_______________________________________________________________________________________________________________________
// HELPERs
//_______________________________________________________________________________________________________________________

uint16_t wcRelPercent100thsToAbsPosition(uint16_t openLimit, uint16_t closedLimit, posPercent100ths_t percent100ths)
{
    uint16_t minimum = 0, maximum = UINT16_MAX, range = UINT16_MAX;

    if (openLimit > closedLimit) {
        minimum = closedLimit;
        maximum = openLimit;
    } else {
        minimum = openLimit;
        maximum = closedLimit;
    }

    range = maximum - minimum;

    if (percent100ths > WC_PERCENT100THS_MAX) return maximum;

    return minimum + ((range * percent100ths) / WC_PERCENT100THS_MAX);
}

posPercent100ths_t wcAbsPositionToRelPercent100ths(uint16_t openLimit, uint16_t closedLimit, uint16_t position)
{
    posPercent100ths_t minimum = 0, range = UINT16_MAX;

    if (openLimit > closedLimit) {
        minimum = closedLimit;
        range = openLimit - minimum;
    } else {
        minimum = openLimit;
        range = closedLimit - minimum;
    }

    if (position < minimum)
        return 0;

    if (range > 0) {
        return (posPercent100ths_t) (WC_PERCENT100THS_MAX * (position - minimum) / range);
    }

    return WC_PERCENT100THS_MAX;
}

EmberAfStatus wcWriteAttribute(chip::EndpointId ep, chip::AttributeId attributeID, uint8_t * dataPtr, EmberAfAttributeType dataType)
{
    if (!dataPtr) return EMBER_ZCL_STATUS_INVALID_FIELD;

    EmberAfStatus status = emberAfWriteAttribute(ep, ZCL_WINDOW_COVERING_CLUSTER_ID, attributeID, CLUSTER_MASK_SERVER, dataPtr, dataType);

    if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfWindowCoveringClusterPrint("Err: WC Writing Attribute failed: %x", status);
    }

    return status;
}

EmberAfStatus wcReadAttribute(chip::EndpointId ep, chip::AttributeId attributeID, uint8_t * dataPtr, uint16_t readLength)
{
    if (!dataPtr) return EMBER_ZCL_STATUS_INVALID_FIELD;

    EmberAfStatus status = emberAfReadAttribute(ep, ZCL_WINDOW_COVERING_CLUSTER_ID, attributeID, CLUSTER_MASK_SERVER, dataPtr, readLength, NULL);

    if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfWindowCoveringClusterPrint("Err: WC Reading Attribute failed: %x", status);
    }

    return status;
}

EmberAfStatus wcSetTargetPositionLift(EndpointId ep, posPercent100ths_t liftPercent100ths)
{
    if (CHECK_BOUNDS_INVALID(0, liftPercent100ths, WC_PERCENT100THS_MAX)) return EMBER_ZCL_STATUS_INVALID_VALUE;

    return wcWriteAttribute(ep, ZCL_WC_TARGET_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &liftPercent100ths, ZCL_INT16U_ATTRIBUTE_TYPE);
}

EmberAfStatus wcSetTargetPositionTilt(EndpointId ep, posPercent100ths_t tiltPercent100ths)
{
    if (CHECK_BOUNDS_INVALID(0, tiltPercent100ths, WC_PERCENT100THS_MAX)) return EMBER_ZCL_STATUS_INVALID_VALUE;

    return wcWriteAttribute(ep, ZCL_WC_TARGET_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &tiltPercent100ths, ZCL_INT16U_ATTRIBUTE_TYPE);
}

EmberAfStatus wcGetCurrentPositionLift(EndpointId ep, posPercent100ths_t * p_liftPercent100ths)
{
    return wcReadAttribute(ep, ZCL_WC_CURRENT_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) p_liftPercent100ths, sizeof(posPercent100ths_t));
}

EmberAfStatus wcGetCurrentPositionTilt(EndpointId ep, posPercent100ths_t * p_tiltPercent100ths)
{
    return wcReadAttribute(ep, ZCL_WC_CURRENT_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) p_tiltPercent100ths, sizeof(posPercent100ths_t));
}

EmberAfStatus wcSetCurrentPositionLift(EndpointId ep, posPercent100ths_t liftPercent100ths, uint16_t liftValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_ACTION_DENIED;

    if (CHECK_BOUNDS_INVALID(0, liftPercent100ths, WC_PERCENT100THS_MAX)) return EMBER_ZCL_STATUS_INVALID_VALUE;

    uint8_t liftPercentage = liftPercent100ths / WC_PERCENTAGE_COEF;

    /* Since we have multiple attribute for positionning lets use always this helper function to update the current position */
    if (m_wcFeature.PA && m_wcFeature.LF) {
        if (EMBER_ZCL_STATUS_SUCCESS != (status = wcWriteAttribute(ep, ZCL_WC_CURRENT_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &liftPercent100ths, ZCL_INT16U_ATTRIBUTE_TYPE))) return status;
        if (EMBER_ZCL_STATUS_SUCCESS != (status = wcWriteAttribute(ep, ZCL_WC_CURRENT_POSITION_LIFT_PERCENTAGE_ATTRIBUTE_ID    , (uint8_t *) &liftPercentage   , ZCL_INT8U_ATTRIBUTE_TYPE ))) return status;
        if (EMBER_ZCL_STATUS_SUCCESS != (status = wcWriteAttribute(ep, ZCL_WC_CURRENT_POSITION_LIFT_ATTRIBUTE_ID               , (uint8_t *) &liftValue        , ZCL_INT16U_ATTRIBUTE_TYPE))) return status;
    } else {
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or LF=%u", m_wcFeature.PA, m_wcFeature.LF);
    }

    return status;
}

EmberAfStatus wcSetCurrentPositionTilt(EndpointId ep, posPercent100ths_t tiltPercent100ths, uint16_t tiltValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_ACTION_DENIED;

    if (CHECK_BOUNDS_INVALID(0, tiltPercent100ths, WC_PERCENT100THS_MAX)) return EMBER_ZCL_STATUS_INVALID_VALUE;

    uint8_t tiltPercentage = tiltPercent100ths / WC_PERCENTAGE_COEF;

    /* Since we have multiple attribute for positionning lets use always this helper function to update the current position */
    if (m_wcFeature.PA && m_wcFeature.TL) {
        if (EMBER_ZCL_STATUS_SUCCESS != (status = wcWriteAttribute(ep, ZCL_WC_CURRENT_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &tiltPercent100ths, ZCL_INT16U_ATTRIBUTE_TYPE))) return status;
        if (EMBER_ZCL_STATUS_SUCCESS != (status = wcWriteAttribute(ep, ZCL_WC_CURRENT_POSITION_TILT_PERCENTAGE_ATTRIBUTE_ID    , (uint8_t *) &tiltPercentage   , ZCL_INT8U_ATTRIBUTE_TYPE ))) return status;
        if (EMBER_ZCL_STATUS_SUCCESS != (status = wcWriteAttribute(ep, ZCL_WC_CURRENT_POSITION_TILT_ATTRIBUTE_ID               , (uint8_t *) &tiltValue        , ZCL_INT16U_ATTRIBUTE_TYPE))) return status;
    } else {
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", m_wcFeature.PA, m_wcFeature.TL);
    }

    return status;
}



//_______________________________________________________________________________________________________________________
// CALLBACKs
//_______________________________________________________________________________________________________________________


/**
 * @brief Window Covering Cluster UpOrOpen Command callback
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterUpOrOpenCallback(chip::app::Command * commandObj)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    EndpointId ep = emberAfCurrentEndpoint();

    emberAfWindowCoveringClusterPrint("UpOrOpen command received");

    wcSetTargetPositionLift(ep, WC_PERCENT100THS_MAX);
    wcSetTargetPositionTilt(ep, WC_PERCENT100THS_MAX);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster DownOrClose Command callback
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterDownOrCloseCallback(chip::app::Command *commandObj)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    EndpointId ep = emberAfCurrentEndpoint();

    emberAfWindowCoveringClusterPrint("DownOrClose command received");

    wcSetTargetPositionLift(ep, 0);
    wcSetTargetPositionTilt(ep, 0);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster StopMotion Command callback
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterStopMotionCallback(chip::app::Command *)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    posPercent100ths_t liftPercent100ths, tiltPercent100ths;
    EndpointId ep = emberAfCurrentEndpoint();

    emberAfWindowCoveringClusterPrint("StopMotion command received");

    wcGetCurrentPositionLift(ep, &liftPercent100ths);
    wcGetCurrentPositionTilt(ep, &tiltPercent100ths);

    wcSetTargetPositionLift(ep, liftPercent100ths);
    wcSetTargetPositionTilt(ep, tiltPercent100ths);

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster GoToLiftPercentage Command callback
 * @param liftPercentageValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::Command *, uint8_t liftPercentageValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster GoToLiftPercentage Command callback
 * @param liftPercentageValue
 * @param liftPercent100thsValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::Command *, uint8_t liftPercentageValue, uint16_t liftPercent100thsValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfWindowCoveringClusterPrint("GoToLiftPercentage w/ Percent100ths command received");

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster GoToLiftValue Command callback
 * @param liftValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToLiftValueCallback(chip::app::Command *, uint16_t liftValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfWindowCoveringClusterPrint("GoToLiftValue Value command received");

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster GoToTiltPercentage Command callback
 * @param tiltPercentageValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::Command *, uint8_t tiltPercentageValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster GoToTiltPercentage Command callback
 * @param tiltPercentageValue
 * @param tiltPercent100thsValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::Command *, uint8_t tiltPercentageValue, uint16_t tiltPercent100thsValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfWindowCoveringClusterPrint("GoToTiltPercentage w/ Percent100ths command received");

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster GoToTiltValue Command callback
 * @param tiltValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToTiltValueCallback(chip::app::Command *, uint16_t tiltValue)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfWindowCoveringClusterPrint("GoToTiltValue command received");

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/** @brief Window Covering Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint Endpoint that is being initialized
 */

void __attribute__((weak)) emberAfWindowCoveringClusterInitCallback(chip::EndpointId endpoint)
{
    emberAfWindowCoveringClusterPrint("Window Covering Cluster init");
}
