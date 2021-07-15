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
 *    you may not use this file except in compliance  the License.
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

#include "window-covering-server.h"

#include <app/util/af.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>

#include <app/CommandHandler.h>
#include <app/common/gen/attributes/Accessors.h>
#include <app/reporting/reporting.h>
#include <app/util/af-event.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <string.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

using namespace chip::app::Clusters;

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
    uint8_t value = (status.operational ? 0x01 : 0) | (status.online ? 0x02 : 0) | (status.liftIsReversed ? 0x04 : 0) |
        (status.liftIsPA ? 0x08 : 0) | (status.tiltIsPA ? 0x10 : 0) | (status.liftIsEncoderControlled ? 0x20 : 0) |
        (status.tiltIsEncoderControlled ? 0x40 : 0);
    writeAttribute(mEndPoint, ZCL_WC_CONFIG_STATUS_ATTRIBUTE_ID, &value, ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

const WindowCover::ConfigStatus WindowCover::ConfigStatusGet(void)
{
    uint8_t value = 0;
    ConfigStatus status;

    readAttribute(mEndPoint, ZCL_WC_CONFIG_STATUS_ATTRIBUTE_ID, &value, sizeof(uint8_t));
    status.operational             = (value & 0x01) ? 1 : 0;
    status.online                  = (value & 0x02) ? 1 : 0;
    status.liftIsReversed          = (value & 0x04) ? 1 : 0;
    status.liftIsPA                = (value & 0x08) ? 1 : 0;
    status.tiltIsPA                = (value & 0x10) ? 1 : 0;
    status.liftIsEncoderControlled = (value & 0x20) ? 1 : 0;
    status.tiltIsEncoderControlled = (value & 0x40) ? 1 : 0;
    return status;
}

void WindowCover::OperationalStatusSet(WindowCover::OperationalStatus status)
{
    uint8_t global = OperationalStateToValue(status.global);
    uint8_t lift   = OperationalStateToValue(status.lift);
    uint8_t tilt   = OperationalStateToValue(status.tilt);
    uint8_t value  = (global && 0x03) | ((lift && 0x03) >> 2) | ((tilt && 0x03) >> 4);
    writeAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, &value, ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

const WindowCover::OperationalStatus WindowCover::OperationalStatusGet(void)
{
    uint8_t value = 0;
    OperationalStatus status;

    readAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, &value, sizeof(uint8_t));
    status.global = ValueToOperationalState(value & 0x03);
    status.lift   = ValueToOperationalState((value << 2) & 0x03);
    status.tilt   = ValueToOperationalState((value << 4) & 0x03);
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
    uint8_t value = (mode.motorDirReversed ? 0x01 : 0) | (mode.calibrationMode ? 0x02 : 0) | (mode.maintenanceMode ? 0x04 : 0) |
        (mode.ledDisplay ? 0x08 : 0);
    writeAttribute(mEndPoint, ZCL_WC_MODE_ATTRIBUTE_ID, &value, ZCL_BITMAP8_ATTRIBUTE_TYPE);
}

const WindowCover::Mode WindowCover::ModeGet(void)
{
    uint8_t value = 0;
    Mode mode;
    writeAttribute(mEndPoint, ZCL_WC_MODE_ATTRIBUTE_ID, &value, sizeof(uint8_t));
    mode.motorDirReversed = (value & 0x01) ? 1 : 0;
    mode.calibrationMode  = (value & 0x02) ? 1 : 0;
    mode.maintenanceMode  = (value & 0x04) ? 1 : 0;
    mode.ledDisplay       = (value & 0x08) ? 1 : 0;
    return mode;
}

void WindowCover::SafetyStatusSet(WindowCover::SafetyStatus status)
{
    uint16_t value = (status.remoteLockout ? 0x0001 : 0) | (status.tamperDetection ? 0x0002 : 0) |
        (status.failedCommunication ? 0x0004 : 0) | (status.positionFailure ? 0x0008 : 0) |
        (status.thermalProtection ? 0x0010 : 0) | (status.obstacleDetected ? 0x0020 : 0) | (status.powerIssue ? 0x0040 : 0) |
        (status.stopInput ? 0x0080 : 0);
    value |= (uint16_t)(status.motorJammed ? 0x0100 : 0) | (uint16_t)(status.hardwareFailure ? 0x0200 : 0) |
        (uint16_t)(status.manualOperation ? 0x0400 : 0);
    writeAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, (uint8_t *) &value, ZCL_BITMAP16_ATTRIBUTE_TYPE);
}

const WindowCover::SafetyStatus WindowCover::SafetyStatusGet(void)
{
    uint16_t value = 0;
    WindowCover::SafetyStatus status;

    readAttribute(mEndPoint, ZCL_WC_OPERATIONAL_STATUS_ATTRIBUTE_ID, (uint8_t *) &value, sizeof(uint16_t));
    status.remoteLockout       = (value & 0x0001) ? 1 : 0;
    status.tamperDetection     = (value & 0x0002) ? 1 : 0;
    status.failedCommunication = (value & 0x0004) ? 1 : 0;
    status.positionFailure     = (value & 0x0008) ? 1 : 0;
    status.thermalProtection   = (value & 0x0010) ? 1 : 0;
    status.obstacleDetected    = (value & 0x0020) ? 1 : 0;
    status.powerIssue          = (value & 0x0040) ? 1 : 0;
    status.stopInput           = (value & 0x0080) ? 1 : 0;
    status.motorJammed         = (value & 0x0100) ? 1 : 0;
    status.hardwareFailure     = (value & 0x0200) ? 1 : 0;
    status.manualOperation     = (value & 0x0400) ? 1 : 0;
    return status;
}

bool WindowCover::IsOpen()
{
    uint16_t liftPosition = mLift.PositionValueGet();
    uint16_t tiltPosition = mTilt.PositionValueGet();
    return liftPosition == mLift.OpenLimitGet() && tiltPosition == mTilt.OpenLimitGet();
}

bool WindowCover::IsClosed()
{
    uint16_t liftPosition = mLift.PositionValueGet();
    uint16_t tiltPosition = mTilt.PositionValueGet();
    return liftPosition == mLift.ClosedLimitGet() && tiltPosition == mTilt.ClosedLimitGet();
}

WindowCover::OperationalState WindowCover::ValueToOperationalState(uint8_t value)
{
    switch (value)
    {
    case 0x00:
        return OperationalState::Stall;
    case 0x01:
        return OperationalState::MovingUpOrOpen;
    case 0x02:
        return OperationalState::MovingDownOrClose;
    case 0x03:
    default:
        return OperationalState::Reserved;
    }
}
uint8_t WindowCover::OperationalStateToValue(WindowCover::OperationalState state)
{
    switch (state)
    {
    case OperationalState::Stall:
        return 0x00;
    case OperationalState::MovingUpOrOpen:
        return 0x01;
    case OperationalState::MovingDownOrClose:
        return 0x02;
    case OperationalState::Reserved:
    default:
        return 0x03;
    }
}

//------------------------------------------------------------------------------
// Actuator
//------------------------------------------------------------------------------

template <typename ActuatorImpl>
uint16_t WindowCover::Actuator<ActuatorImpl>::ValueToPercent100ths(uint16_t value)
{
    uint16_t openLimit   = OpenLimitGet();
    uint16_t closedLimit = ClosedLimitGet();
    uint16_t minimum = 0, range = UINT16_MAX;

    if (openLimit > closedLimit)
    {
        minimum = closedLimit;
        range   = static_cast<uint16_t>(openLimit - minimum);
    }
    else
    {
        minimum = openLimit;
        range   = static_cast<uint16_t>(closedLimit - minimum);
    }

    if (value < minimum)
    {
        return 0;
    }

    if (range > 0)
    {
        return static_cast<uint16_t>(WC_PERCENT100THS_MAX * (value - minimum) / range);
    }

    return WC_PERCENT100THS_MAX;
}

template <typename ActuatorImpl>
uint16_t WindowCover::Actuator<ActuatorImpl>::Percent100thsToValue(uint16_t percent100ths)
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

    range = static_cast<uint16_t>(maximum - minimum);

    if (percent100ths > WC_PERCENT100THS_MAX)
        return maximum;

    return static_cast<uint16_t>(minimum + ((range * percent100ths) / WC_PERCENT100THS_MAX));
}

//------------------------------------------------------------------------------
// LiftActuator
//------------------------------------------------------------------------------

void WindowCover::LiftActuator::OpenLimitSet(uint16_t limit)
{
    writeAttribute(mEndPoint, ZCL_WC_INSTALLED_OPEN_LIMIT_LIFT_ATTRIBUTE_ID, (uint8_t *) &limit, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::LiftActuator::OpenLimitGet(void)
{
    uint16_t limit = 0;
    readAttribute(mEndPoint, ZCL_WC_INSTALLED_OPEN_LIMIT_LIFT_ATTRIBUTE_ID, (uint8_t *) &limit, sizeof(uint16_t));
    return limit;
}

void WindowCover::LiftActuator::ClosedLimitSet(uint16_t limit)
{
    writeAttribute(mEndPoint, ZCL_WC_INSTALLED_CLOSED_LIMIT_LIFT_ATTRIBUTE_ID, (uint8_t *) &limit, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::LiftActuator::ClosedLimitGet(void)
{
    uint16_t limit = 0;
    readAttribute(mEndPoint, ZCL_WC_INSTALLED_CLOSED_LIMIT_LIFT_ATTRIBUTE_ID, (uint8_t *) &limit, sizeof(uint16_t));
    return limit;
}

void WindowCover::LiftActuator::PositionSet(uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t value  = Percent100thsToValue(percent100ths);

    writeAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_LIFT_ATTRIBUTE_ID, (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
    writeAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_LIFT_PERCENTAGE_ATTRIBUTE_ID, (uint8_t *) &percent, ZCL_INT8U_ATTRIBUTE_TYPE);
    writeAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths,
                   ZCL_INT16U_ATTRIBUTE_TYPE);
    emberAfWindowCoveringClusterPrint("LiftActuator::PositionSet(%u%%)", percent100ths / 100);
}

uint16_t WindowCover::LiftActuator::PositionGet()
{
    uint16_t percent100ths = 0;
    readAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths,
                  sizeof(uint16_t));
    return percent100ths;
}

void WindowCover::LiftActuator::TargetSet(uint16_t percent100ths)
{
    emberAfWindowCoveringClusterPrint("LiftActuator::TargetSet(%u%%)", percent100ths / 100);
    writeAttribute(mEndPoint, ZCL_WC_TARGET_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths,
                   ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::LiftActuator::TargetGet()
{
    uint16_t percent100ths = 0;
    readAttribute(mEndPoint, ZCL_WC_TARGET_POSITION_LIFT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths, sizeof(uint16_t));
    return percent100ths;
}

void WindowCover::LiftActuator::NumberOfActuationsIncrement()
{
    uint16_t count = static_cast<uint16_t>(NumberOfActuationsGet() + 1);
    writeAttribute(mEndPoint, ZCL_WC_NUMBER_OF_ACTUATIONS_LIFT_ATTRIBUTE_ID, (uint8_t *) &count, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::LiftActuator::NumberOfActuationsGet(void)
{
    uint16_t count = 0;
    readAttribute(mEndPoint, ZCL_WC_NUMBER_OF_ACTUATIONS_LIFT_ATTRIBUTE_ID, (uint8_t *) &count, sizeof(uint16_t));
    return count;
}

//------------------------------------------------------------------------------
// TiltActuator
//------------------------------------------------------------------------------

void WindowCover::TiltActuator::OpenLimitSet(uint16_t limit)
{
    writeAttribute(mEndPoint, ZCL_WC_INSTALLED_OPEN_LIMIT_TILT_ATTRIBUTE_ID, (uint8_t *) &limit, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::TiltActuator::OpenLimitGet(void)
{
    uint16_t limit = 0;
    readAttribute(mEndPoint, ZCL_WC_INSTALLED_OPEN_LIMIT_TILT_ATTRIBUTE_ID, (uint8_t *) &limit, sizeof(uint16_t));
    return limit;
}

void WindowCover::TiltActuator::ClosedLimitSet(uint16_t limit)
{
    writeAttribute(mEndPoint, ZCL_WC_INSTALLED_CLOSED_LIMIT_TILT_ATTRIBUTE_ID, (uint8_t *) &limit, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::TiltActuator::ClosedLimitGet(void)
{
    uint16_t limit = 0;
    readAttribute(mEndPoint, ZCL_WC_INSTALLED_CLOSED_LIMIT_TILT_ATTRIBUTE_ID, (uint8_t *) &limit, sizeof(uint16_t));
    return limit;
}

void WindowCover::TiltActuator::PositionSet(uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t value  = Percent100thsToValue(percent100ths);

    writeAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_TILT_ATTRIBUTE_ID, (uint8_t *) &value, ZCL_INT16U_ATTRIBUTE_TYPE);
    writeAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_TILT_PERCENTAGE_ATTRIBUTE_ID, (uint8_t *) &percent, ZCL_INT8U_ATTRIBUTE_TYPE);
    writeAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths,
                   ZCL_INT16U_ATTRIBUTE_TYPE);
    emberAfWindowCoveringClusterPrint("TiltActuator::PositionSet(%u%%)", percent100ths / 100);
}

uint16_t WindowCover::TiltActuator::PositionGet()
{
    uint16_t percent100ths = 0;
    readAttribute(mEndPoint, ZCL_WC_CURRENT_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths,
                  sizeof(uint16_t));
    return percent100ths;
}

void WindowCover::TiltActuator::TargetSet(uint16_t percent100ths)
{
    emberAfWindowCoveringClusterPrint("Actuator::TargetSet(+%u%%)", percent100ths / 100);
    writeAttribute(mEndPoint, ZCL_WC_TARGET_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths,
                   ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::TiltActuator::TargetGet()
{
    uint16_t percent100ths = 0;
    readAttribute(mEndPoint, ZCL_WC_TARGET_POSITION_TILT_PERCENT100_THS_ATTRIBUTE_ID, (uint8_t *) &percent100ths, sizeof(uint16_t));
    return percent100ths;
}

void WindowCover::TiltActuator::NumberOfActuationsIncrement()
{
    uint16_t count = static_cast<uint16_t>(NumberOfActuationsGet() + 1);
    writeAttribute(mEndPoint, ZCL_WC_NUMBER_OF_ACTUATIONS_TILT_ATTRIBUTE_ID, (uint8_t *) &count, ZCL_INT16U_ATTRIBUTE_TYPE);
}

uint16_t WindowCover::TiltActuator::NumberOfActuationsGet(void)
{
    uint16_t count = 0;
    readAttribute(mEndPoint, ZCL_WC_NUMBER_OF_ACTUATIONS_TILT_ATTRIBUTE_ID, (uint8_t *) &count, sizeof(uint16_t));
    return count;
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

bool emberAfWindowCoveringClusterUpOrOpenCallback(chip::app::CommandHandler * cmmd)
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
bool emberAfWindowCoveringClusterDownOrCloseCallback(chip::app::CommandHandler * commandObj)
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
 * @brief Window Covering Cluster Go To Lift Value Command callback
 * @param liftValue
 */

bool emberAfWindowCoveringClusterGoToLiftValueCallback(chip::app::CommandHandler * cmmd, uint16_t value)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftValue Value command received");
    if (hasLift && isPositionAware)
    {
        cover.Lift().TargetValueSet(value);
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

bool emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::CommandHandler * cmmd, uint8_t percent)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");
    if (hasLift && isPositionAware)
    {
        cover.Lift().TargetSet(static_cast<uint16_t>(percent > 100 ? 10000 : percent * 100));
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or LF=%u", isPositionAware, hasLift);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::app::CommandHandler * cmmd, uint8_t percent,
                                                            uint16_t percent100ths)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");
    if (hasLift && isPositionAware)
    {
        cover.Lift().TargetSet(percent100ths);
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

bool emberAfWindowCoveringClusterGoToTiltValueCallback(chip::app::CommandHandler * cmmd, uint16_t value)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltValue command received");
    if (hasTilt && isPositionAware)
    {
        cover.Tilt().TargetValueSet(value);
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

bool emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::CommandHandler * cmmd, uint8_t percent)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");
    if (hasTilt && isPositionAware)
    {
        cover.Tilt().TargetSet(static_cast<uint16_t>(percent > 100 ? 10000 : percent * 100));
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::app::CommandHandler * cmmd, uint8_t percent,
                                                            uint16_t percent100ths)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");
    if (hasTilt && isPositionAware)
    {
        cover.Tilt().TargetSet(percent100ths);
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
