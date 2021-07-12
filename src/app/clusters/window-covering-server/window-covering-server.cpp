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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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

#include "window-covering-server.h"

#include <app/util/af.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>

#include <app/Command.h>
#include <app/reporting/reporting.h>
#include <app/util/af-event.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

using namespace chip;

#define CHECK_BOUNDS_INVALID(MIN, VAL, MAX) ((VAL < MIN) || (VAL > MAX))
#define CHECK_BOUNDS_VALID(MIN, VAL, MAX) (!CHECK_BOUNDS_INVALID(MIN, VAL, MAX))

#define WC_PERCENTAGE_COEF 100
#define WC_PERCENT100THS_MAX 10000

static EmberAfStatus writeAttribute(chip::EndpointId ep, chip::AttributeId attributeID, uint8_t * dataPtr,
                                    EmberAfAttributeType dataType)
{
    if (!dataPtr)
        return EMBER_ZCL_STATUS_INVALID_FIELD;

    EmberAfStatus status =
        emberAfWriteAttribute(ep, ZCL_WINDOW_COVERING_CLUSTER_ID, attributeID, CLUSTER_MASK_SERVER, dataPtr, dataType);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfWindowCoveringClusterPrint("Err: WC Writing Attribute failed, ep:%u, attr:%04x, err:%04x", ep, attributeID, status);
    }
    return status;
}

static EmberAfStatus readAttribute(chip::EndpointId ep, chip::AttributeId attributeID, uint8_t * dataPtr, uint16_t readLength)
{
    if (!dataPtr)
        return EMBER_ZCL_STATUS_INVALID_FIELD;

    EmberAfStatus status =
        emberAfReadAttribute(ep, ZCL_WINDOW_COVERING_CLUSTER_ID, attributeID, CLUSTER_MASK_SERVER, dataPtr, readLength, NULL);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfWindowCoveringClusterPrint("Err: WC Reading Attribute failed, ep:%u, attr:%04x, err:%04x", ep, attributeID, status);
    }

    return status;
}

//------------------------------------------------------------------------------
// WindowCover
//------------------------------------------------------------------------------

WindowCover WindowCover::sInstance;

WindowCover & WindowCover::Instance()
{
    return sInstance;
}

WindowCover::WindowCover() {}

void WindowCover::TypeSet(EmberAfWcType type)
{
    writeAttribute(mEndPoint, ZCL_WC_TYPE_ATTRIBUTE_ID, (uint8_t *) &type, ZCL_ENUM8_ATTRIBUTE_TYPE);
}

EmberAfWcType WindowCover::TypeGet(void)
{
    EmberAfWcType type;
    readAttribute(mEndPoint, ZCL_WC_TYPE_ATTRIBUTE_ID, (uint8_t *) &type, sizeof(uint8_t));
    return type;
}

void WindowCover::ConfigStatusSet(WindowCover::ConfigStatus status)
{
    writeAttribute(mEndPoint, ZCL_WC_CONFIG_STATUS_ATTRIBUTE_ID, (uint8_t *) &status, ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

WindowCover::ConfigStatus WindowCover::ConfigStatusGet(void)
{
    ConfigStatus status;
    readAttribute(mEndPoint, ZCL_WC_CONFIG_STATUS_ATTRIBUTE_ID, (uint8_t *) &status, sizeof(uint8_t));
    return status;
}

void WindowCover::OperationalStatusSet(WindowCover::OperationalStatus status)
{
    writeAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, (uint8_t *) &status, ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

WindowCover::OperationalStatus WindowCover::OperationalStatusGet(void)
{
    OperationalStatus status;
    readAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, (uint8_t *) &status, sizeof(uint8_t));
    return status;
}

void WindowCover::EndProductTypeSet(EmberAfWcEndProductType type)
{
    writeAttribute(mEndPoint, ZCL_WC_END_PRODUCT_TYPE_ATTRIBUTE_ID, (uint8_t *) &type, ZCL_ENUM8_ATTRIBUTE_TYPE);
}

EmberAfWcEndProductType WindowCover::EndProductTypeGet(void)
{
    EmberAfWcEndProductType type;
    readAttribute(mEndPoint, ZCL_WC_END_PRODUCT_TYPE_ATTRIBUTE_ID, (uint8_t *) &type, sizeof(uint8_t));
    return type;
}

void WindowCover::ModeSet(WindowCover::Mode mode)
{
    writeAttribute(mEndPoint, ZCL_WC_MODE_ATTRIBUTE_ID, (uint8_t *) &mode, ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

WindowCover::Mode WindowCover::ModeGet(void)
{
    Mode mode;
    writeAttribute(mEndPoint, ZCL_WC_MODE_ATTRIBUTE_ID, (uint8_t *) &mode, sizeof(uint8_t));
    return mode;
}

void WindowCover::SafetyStatusSet(WindowCover::SafetyStatus status)
{
    writeAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, (uint8_t *) &status, ZCL_BITMAP16_ATTRIBUTE_TYPE);
}

WindowCover::SafetyStatus WindowCover::SafetyStatusGet(void)
{
    WindowCover::SafetyStatus status;
    readAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, (uint8_t *) &status, sizeof(uint16_t));
    return status;
}

bool WindowCover::IsOpen()
{
    uint16_t liftPosition = mLift.PositionGet(PositionUnits::Percentage100ths);
    uint16_t tiltPosition = mTilt.PositionGet(PositionUnits::Percentage100ths);
    return liftPosition == mLift.OpenLimitGet() && tiltPosition == mTilt.OpenLimitGet();
}

bool WindowCover::IsClosed()
{
    uint16_t liftPosition = mLift.PositionGet(PositionUnits::Percentage100ths);
    uint16_t tiltPosition = mTilt.PositionGet(PositionUnits::Percentage100ths);
    return liftPosition == mLift.ClosedLimitGet() && tiltPosition == mTilt.ClosedLimitGet();
}

//------------------------------------------------------------------------------
// Actuator
//------------------------------------------------------------------------------

void WindowCover::Actuator::OpenLimitSet(uint16_t limit)
{
    writeAttribute(mEndPoint, mAttributeIds.openLimit, (uint8_t *) &limit, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::Actuator::OpenLimitGet(void)
{
    uint16_t limit = 0;
    readAttribute(mEndPoint, mAttributeIds.openLimit, (uint8_t *) &limit, sizeof(uint16_t));
    return limit;
}

void WindowCover::Actuator::ClosedLimitSet(uint16_t limit)
{
    writeAttribute(mEndPoint, mAttributeIds.closedLimit, (uint8_t *) &limit, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::Actuator::ClosedLimitGet(void)
{
    uint16_t limit = 0;
    readAttribute(mEndPoint, mAttributeIds.closedLimit, (uint8_t *) &limit, sizeof(uint16_t));
    return limit;
}

void WindowCover::Actuator::PositionSet(uint16_t position, PositionUnits units)
{
    EmberAfStatus status   = EMBER_ZCL_STATUS_ACTION_DENIED;
    uint8_t percent        = 0;
    uint16_t percent100ths = 0;
    uint16_t value         = 0;

    switch (units)
    {
    case PositionUnits::Value:
        value         = position;
        percent100ths = ValueToPercent100ths(position);
        percent       = static_cast<uint8_t>(percent100ths / 100);
        break;
    case PositionUnits::Percentage:
        percent       = static_cast<uint8_t>(position);
        percent100ths = position * 100;
        value         = Percent100thsToValue(percent100ths);
        break;
    case PositionUnits::Percentage100ths:
    default:
        percent100ths = position;
        percent       = static_cast<uint8_t>(position / 100);
        value         = Percent100thsToValue(percent100ths);
        break;
    }
    emberAfWindowCoveringClusterPrint("Actuator::PositionSet, unit:%u, pos:%04x, val:%04x, %%:%u, 0.1%%:%u", units, position, value,
                                      percent, percent100ths);

    status = writeAttribute(mEndPoint, mAttributeIds.positionValue, (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        return;
    }

    status = writeAttribute(mEndPoint, mAttributeIds.positionPercent, (uint8_t *) &percent, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        return;
    }

    status = writeAttribute(mEndPoint, mAttributeIds.positionPercent100ths, (uint8_t *) &percent100ths, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        return;
    }
}

void WindowCover::Actuator::PositionSet(uint16_t value)
{
    PositionSet(value, PositionUnits::Percentage100ths);
}

uint16_t WindowCover::Actuator::PositionGet(PositionUnits units)
{
    uint16_t percent100ths = 0;

    readAttribute(mEndPoint, mAttributeIds.positionPercent100ths, (uint8_t *) &percent100ths, sizeof(uint16_t));

    emberAfWindowCoveringClusterPrint("Actuator::PositionGet, attr:%04x, value:%04x", mAttributeIds.positionPercent100ths,
                                      percent100ths);

    switch (units)
    {
    case PositionUnits::Value:
        return Percent100thsToValue(percent100ths);
    case PositionUnits::Percentage:
        return (uint16_t)(percent100ths / 100);
    case PositionUnits::Percentage100ths:
    default:
        return percent100ths;
    }
}

uint16_t WindowCover::Actuator::PositionGet()
{
    return PositionGet(PositionUnits::Percentage100ths);
}

void WindowCover::Actuator::NumberOfActuationsIncrement()
{
    uint16_t count = NumberOfActuationsGet() + 1;
    writeAttribute(mEndPoint, mAttributeIds.numberOfActuations, (uint8_t *) &count, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::Actuator::NumberOfActuationsGet(void)
{
    uint16_t count = 0;
    readAttribute(mEndPoint, mAttributeIds.numberOfActuations, (uint8_t *) &count, sizeof(uint16_t));
    return count;
}

void WindowCover::Actuator::TargetSet(uint16_t value, PositionUnits units)
{
    uint16_t target;
    switch (units)
    {
    case PositionUnits::Value:
        target = ValueToPercent100ths(value);
        break;
    case PositionUnits::Percentage:
        target = value * 100;
        break;
    case PositionUnits::Percentage100ths:
    default:
        target = value;
        break;
    }

    emberAfWindowCoveringClusterPrint("Actuator::TargetSet, attr:%04x, val:%04x, unit:%u, target:%04x (%u)",
                                      mAttributeIds.targetPercent100ths, value, units, target, target);
    writeAttribute(mEndPoint, mAttributeIds.targetPercent100ths, (uint8_t *) &target, ZCL_INT16U_ATTRIBUTE_TYPE);
}

void WindowCover::Actuator::TargetSet(uint16_t value)
{
    TargetSet(value, PositionUnits::Percentage100ths);
}

uint16_t WindowCover::Actuator::TargetGet(PositionUnits units)
{
    uint16_t target;

    readAttribute(mEndPoint, mAttributeIds.targetPercent100ths, (uint8_t *) &target, sizeof(uint16_t));

    emberAfWindowCoveringClusterPrint("Actuator::TargetGet, attr:%04x, units:%u, value:%04x", mAttributeIds.targetPercent100ths,
                                      units, target);

    switch (units)
    {
    case PositionUnits::Value:
        return Percent100thsToValue(target);
    case PositionUnits::Percentage:
        return (uint16_t)(target / 100);
    case PositionUnits::Percentage100ths:
    default:
        return target;
    }
}

uint16_t WindowCover::Actuator::TargetGet()
{
    return TargetGet(PositionUnits::Percentage100ths);
}

uint16_t WindowCover::Actuator::ValueToPercent100ths(uint16_t value)
{
    uint16_t openLimit   = OpenLimitGet();
    uint16_t closedLimit = ClosedLimitGet();
    uint16_t minimum = 0, range = UINT16_MAX;

    if (openLimit > closedLimit)
    {
        minimum = closedLimit;
        range   = openLimit - minimum;
    }
    else
    {
        minimum = openLimit;
        range   = closedLimit - minimum;
    }

    if (value < minimum)
    {
        return 0;
    }

    if (range > 0)
    {
        return (uint16_t)(WC_PERCENT100THS_MAX * (value - minimum) / range);
    }

    return WC_PERCENT100THS_MAX;
}

uint16_t WindowCover::Actuator::Percent100thsToValue(uint16_t percent100ths)
{
    uint16_t openLimit   = OpenLimitGet();
    uint16_t closedLimit = ClosedLimitGet();
    uint16_t minimum = 0, maximum = UINT16_MAX, range = UINT16_MAX;

    if (openLimit > closedLimit)
    {
        minimum = closedLimit;
        maximum = openLimit;
    }
    else
    {
        minimum = openLimit;
        maximum = closedLimit;
    }

    range = maximum - minimum;

    if (percent100ths > WC_PERCENT100THS_MAX)
        return maximum;

    return static_cast<uint16_t>(minimum + ((range * percent100ths) / WC_PERCENT100THS_MAX));
}

//------------------------------------------------------------------------------
// Callbacks
//------------------------------------------------------------------------------

/** @brief Window Covering Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfWindowCoveringClusterInitCallback(chip::EndpointId endpoint)
{
    emberAfWindowCoveringClusterPrint("Window Covering Cluster init");
}

/**
 * @brief Window Covering Cluster UpOrOpen Command callback
 */
bool __attribute__((weak)) emberAfWindowCoveringClusterUpOrOpenCallback(chip::app::Command * cmmd)
{
    WindowCover & cover  = WindowCover::Instance();
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("UpOrOpen command received");
    if (cover.hasFeature(WindowCover::Feature::Lift))
    {
        cover.Lift().TargetSet(0);
    }
    if (cover.hasFeature(WindowCover::Feature::Tilt))
    {
        cover.Tilt().TargetSet(0);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster DownOrClose Command callback
 */
bool __attribute__((weak)) emberAfWindowCoveringClusterDownOrCloseCallback(chip::app::Command * cmmd)
{
    WindowCover & cover  = WindowCover::Instance();
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("DownOrClose command received");
    if (cover.hasFeature(WindowCover::Feature::Lift))
    {
        cover.Lift().TargetSet(WC_PERCENT100THS_MAX);
    }
    if (cover.hasFeature(WindowCover::Feature::Tilt))
    {
        cover.Tilt().TargetSet(WC_PERCENT100THS_MAX);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster Stop Motion Command callback
 */
bool __attribute__((weak)) emberAfWindowCoveringClusterStopMotionCallback(chip::app::Command *)
{
    WindowCover & cover  = WindowCover::Instance();
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("StopMotion command received");
    cover.Lift().Stop();
    cover.Tilt().Stop();
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster Go To Lift Value Command callback
 * @param liftValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToLiftValueCallback(chip::app::Command * cmmd, uint16_t value)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftValue Value command received");
    if (hasLift && isPositionAware)
    {
        cover.Lift().TargetSet(value, WindowCover::PositionUnits::Value);
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or LF=%u", isPositionAware, hasLift);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster Go To Lift Percentage Command callback
 * @param percentLiftValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::Command * cmmd, uint8_t percent)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");
    if (hasLift && isPositionAware)
    {
        cover.Lift().TargetSet(percent, WindowCover::PositionUnits::Percentage);
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or LF=%u", isPositionAware, hasLift);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool __attribute__((weak))
emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::Command * cmmd, uint8_t percent, uint16_t percent100ths)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");
    if (hasLift && isPositionAware)
    {
        if (percent > 100)
        {
            cover.Lift().TargetSet(percent100ths, WindowCover::PositionUnits::Percentage100ths);
        }
        else
        {
            cover.Lift().TargetSet(percent, WindowCover::PositionUnits::Percentage);
        }
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or LF=%u", isPositionAware, hasLift);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster Go To Tilt Value Command callback
 * @param tiltValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToTiltValueCallback(chip::app::Command * cmmd, uint16_t value)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltValue command received");
    if (hasTilt && isPositionAware)
    {
        cover.Tilt().TargetSet(value, WindowCover::PositionUnits::Value);
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

/**
 * @brief Window Covering Cluster Go To Tilt Percentage Command callback
 * @param percentTiltValue
 */

bool __attribute__((weak)) emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::Command * cmmd, uint8_t percent)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");
    if (hasTilt && isPositionAware)
    {
        cover.Tilt().TargetSet(percent, WindowCover::PositionUnits::Percentage);
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool __attribute__((weak))
emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::Command * cmmd, uint8_t percent, uint16_t percent100ths)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");
    if (hasTilt && isPositionAware)
    {
        if (percent > 100)
        {
            cover.Tilt().TargetSet(percent100ths, WindowCover::PositionUnits::Percentage100ths);
        }
        else
        {
            cover.Tilt().TargetSet(percent, WindowCover::PositionUnits::Percentage);
        }
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
