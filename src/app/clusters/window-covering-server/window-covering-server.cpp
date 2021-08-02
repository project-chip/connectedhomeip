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
#include <lib/support/TypeTraits.h>
#include <string.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

using namespace chip::app::Clusters;

#define WC_PERCENT100THS_MAX 10000

//------------------------------------------------------------------------------
// WindowCover
//------------------------------------------------------------------------------

WindowCover WindowCover::sInstance;

WindowCover & WindowCover::Instance()
{
    return sInstance;
}

void WindowCover::TypeSet(EmberAfWcType type)
{
    WindowCovering::Attributes::SetType(mEndpoint, chip::to_underlying(type));
}

EmberAfWcType WindowCover::TypeGet(void)
{
    std::underlying_type<EmberAfWcType>::type value;
    WindowCovering::Attributes::GetType(mEndpoint, &value);
    return static_cast<EmberAfWcType>(value);
}

void WindowCover::ConfigStatusSet(const WindowCover::ConfigStatus & status)
{
    uint8_t value = (status.operational ? 0x01 : 0) | (status.online ? 0x02 : 0) | (status.liftIsReversed ? 0x04 : 0) |
        (status.liftIsPA ? 0x08 : 0) | (status.tiltIsPA ? 0x10 : 0) | (status.liftIsEncoderControlled ? 0x20 : 0) |
        (status.tiltIsEncoderControlled ? 0x40 : 0);
    WindowCovering::Attributes::SetConfigStatus(mEndpoint, value);
}

const WindowCover::ConfigStatus WindowCover::ConfigStatusGet(void)
{
    uint8_t value = 0;
    ConfigStatus status;

    WindowCovering::Attributes::GetConfigStatus(mEndpoint, &value);
    status.operational             = (value & 0x01) ? 1 : 0;
    status.online                  = (value & 0x02) ? 1 : 0;
    status.liftIsReversed          = (value & 0x04) ? 1 : 0;
    status.liftIsPA                = (value & 0x08) ? 1 : 0;
    status.tiltIsPA                = (value & 0x10) ? 1 : 0;
    status.liftIsEncoderControlled = (value & 0x20) ? 1 : 0;
    status.tiltIsEncoderControlled = (value & 0x40) ? 1 : 0;
    return status;
}

void WindowCover::OperationalStatusSet(const WindowCover::OperationalStatus & status)
{
    uint8_t global = OperationalStateToValue(status.global);
    uint8_t lift   = OperationalStateToValue(status.lift);
    uint8_t tilt   = OperationalStateToValue(status.tilt);
    uint8_t value  = (global & 0x03) | static_cast<uint8_t>((lift & 0x03) << 2) | static_cast<uint8_t>((tilt & 0x03) << 4);
    WindowCovering::Attributes::SetOperationalStatus(mEndpoint, value);
}

const WindowCover::OperationalStatus WindowCover::OperationalStatusGet(void)
{
    uint8_t value = 0;
    OperationalStatus status;

    WindowCovering::Attributes::GetOperationalStatus(mEndpoint, &value);
    status.global = ValueToOperationalState(value & 0x03);
    status.lift   = ValueToOperationalState((value >> 2) & 0x03);
    status.tilt   = ValueToOperationalState((value >> 4) & 0x03);
    return status;
}

void WindowCover::EndProductTypeSet(EmberAfWcEndProductType type)
{
    WindowCovering::Attributes::SetEndProductType(mEndpoint, chip::to_underlying(type));
}

EmberAfWcEndProductType WindowCover::EndProductTypeGet(void)
{
    std::underlying_type<EmberAfWcType>::type value;
    WindowCovering::Attributes::GetEndProductType(mEndpoint, &value);
    return static_cast<EmberAfWcEndProductType>(value);
}

void WindowCover::ModeSet(const WindowCover::Mode & mode)
{
    uint8_t value = (mode.motorDirReversed ? 0x01 : 0) | (mode.calibrationMode ? 0x02 : 0) | (mode.maintenanceMode ? 0x04 : 0) |
        (mode.ledDisplay ? 0x08 : 0);
    WindowCovering::Attributes::SetMode(mEndpoint, value);
}

const WindowCover::Mode WindowCover::ModeGet(void)
{
    uint8_t value = 0;
    Mode mode;

    WindowCovering::Attributes::GetMode(mEndpoint, &value);
    mode.motorDirReversed = (value & 0x01) ? 1 : 0;
    mode.calibrationMode  = (value & 0x02) ? 1 : 0;
    mode.maintenanceMode  = (value & 0x04) ? 1 : 0;
    mode.ledDisplay       = (value & 0x08) ? 1 : 0;
    return mode;
}

void WindowCover::SafetyStatusSet(WindowCover::SafetyStatus & status)
{
    uint16_t value = (status.remoteLockout ? 0x0001 : 0) | (status.tamperDetection ? 0x0002 : 0) |
        (status.failedCommunication ? 0x0004 : 0) | (status.positionFailure ? 0x0008 : 0) |
        (status.thermalProtection ? 0x0010 : 0) | (status.obstacleDetected ? 0x0020 : 0) | (status.powerIssue ? 0x0040 : 0) |
        (status.stopInput ? 0x0080 : 0);
    value |= (uint16_t)(status.motorJammed ? 0x0100 : 0) | (uint16_t)(status.hardwareFailure ? 0x0200 : 0) |
        (uint16_t)(status.manualOperation ? 0x0400 : 0);
    WindowCovering::Attributes::SetSafetyStatus(mEndpoint, value);
}

const WindowCover::SafetyStatus WindowCover::SafetyStatusGet(void)
{
    uint16_t value = 0;
    WindowCover::SafetyStatus status;

    WindowCovering::Attributes::GetSafetyStatus(mEndpoint, &value);
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
uint8_t WindowCover::OperationalStateToValue(const WindowCover::OperationalState & state)
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
    WindowCovering::Attributes::SetInstalledOpenLimitLift(mEndpoint, limit);
}

uint16_t WindowCover::LiftActuator::OpenLimitGet(void)
{
    uint16_t limit = 0;
    WindowCovering::Attributes::GetInstalledOpenLimitLift(mEndpoint, &limit);
    return limit;
}

void WindowCover::LiftActuator::ClosedLimitSet(uint16_t limit)
{
    WindowCovering::Attributes::SetInstalledClosedLimitLift(mEndpoint, limit);
}

uint16_t WindowCover::LiftActuator::ClosedLimitGet(void)
{
    uint16_t limit = 0;
    WindowCovering::Attributes::GetInstalledClosedLimitLift(mEndpoint, &limit);
    return limit;
}

void WindowCover::LiftActuator::PositionSet(uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t value  = Percent100thsToValue(percent100ths);

    WindowCovering::Attributes::SetCurrentPositionLift(mEndpoint, value);
    WindowCovering::Attributes::SetCurrentPositionLiftPercentage(mEndpoint, percent);
    WindowCovering::Attributes::SetCurrentPositionLiftPercent100ths(mEndpoint, percent100ths);
    emberAfWindowCoveringClusterPrint("LiftActuator::PositionSet(%u%%)", percent100ths / 100);
}

uint16_t WindowCover::LiftActuator::PositionGet()
{
    uint16_t percent100ths = 0;
    WindowCovering::Attributes::GetCurrentPositionLiftPercent100ths(mEndpoint, &percent100ths);
    return percent100ths;
}

void WindowCover::LiftActuator::TargetSet(uint16_t percent100ths)
{
    emberAfWindowCoveringClusterPrint("LiftActuator::TargetSet(%u%%)", percent100ths / 100);
    WindowCovering::Attributes::SetTargetPositionLiftPercent100ths(mEndpoint, percent100ths);
}

uint16_t WindowCover::LiftActuator::TargetGet()
{
    uint16_t percent100ths = 0;
    WindowCovering::Attributes::GetTargetPositionLiftPercent100ths(mEndpoint, &percent100ths);
    return percent100ths;
}

void WindowCover::LiftActuator::NumberOfActuationsIncrement()
{
    uint16_t count = static_cast<uint16_t>(NumberOfActuationsGet() + 1);
    WindowCovering::Attributes::SetNumberOfActuationsLift(mEndpoint, count);
}

uint16_t WindowCover::LiftActuator::NumberOfActuationsGet(void)
{
    uint16_t count = 0;
    WindowCovering::Attributes::GetNumberOfActuationsLift(mEndpoint, &count);
    return count;
}

//------------------------------------------------------------------------------
// TiltActuator
//------------------------------------------------------------------------------

void WindowCover::TiltActuator::OpenLimitSet(uint16_t limit)
{
    WindowCovering::Attributes::SetInstalledOpenLimitTilt(mEndpoint, limit);
}

uint16_t WindowCover::TiltActuator::OpenLimitGet(void)
{
    uint16_t limit = 0;
    WindowCovering::Attributes::GetInstalledOpenLimitTilt(mEndpoint, &limit);
    return limit;
}

void WindowCover::TiltActuator::ClosedLimitSet(uint16_t limit)
{
    WindowCovering::Attributes::SetInstalledClosedLimitTilt(mEndpoint, limit);
}

uint16_t WindowCover::TiltActuator::ClosedLimitGet(void)
{
    uint16_t limit = 0;
    WindowCovering::Attributes::GetInstalledClosedLimitTilt(mEndpoint, &limit);
    return limit;
}

void WindowCover::TiltActuator::PositionSet(uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t value  = Percent100thsToValue(percent100ths);

    WindowCovering::Attributes::SetCurrentPositionTilt(mEndpoint, value);
    WindowCovering::Attributes::SetCurrentPositionTiltPercentage(mEndpoint, percent);
    WindowCovering::Attributes::SetCurrentPositionTiltPercent100ths(mEndpoint, percent100ths);
    emberAfWindowCoveringClusterPrint("TiltActuator::PositionSet(%u%%)", percent100ths / 100);
}

uint16_t WindowCover::TiltActuator::PositionGet()
{
    uint16_t percent100ths = 0;
    WindowCovering::Attributes::GetCurrentPositionTiltPercent100ths(mEndpoint, &percent100ths);
    return percent100ths;
}

void WindowCover::TiltActuator::TargetSet(uint16_t percent100ths)
{
    emberAfWindowCoveringClusterPrint("TiltActuator::TargetSet(%u%%)", percent100ths / 100);
    WindowCovering::Attributes::SetTargetPositionTiltPercent100ths(mEndpoint, percent100ths);
}

uint16_t WindowCover::TiltActuator::TargetGet()
{
    uint16_t percent100ths = 0;
    WindowCovering::Attributes::GetTargetPositionTiltPercent100ths(mEndpoint, &percent100ths);
    return percent100ths;
}

void WindowCover::TiltActuator::NumberOfActuationsIncrement()
{
    uint16_t count = static_cast<uint16_t>(NumberOfActuationsGet() + 1);
    WindowCovering::Attributes::SetNumberOfActuationsTilt(mEndpoint, count);
}

uint16_t WindowCover::TiltActuator::NumberOfActuationsGet(void)
{
    uint16_t count = 0;
    WindowCovering::Attributes::GetNumberOfActuationsTilt(mEndpoint, &count);
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
 * @brief  Cluster UpOrOpen Command callback (from client)
 */
bool emberAfWindowCoveringClusterUpOrOpenCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj)
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
 * @brief  Cluster DownOrClose Command callback (from client)
 */
bool emberAfWindowCoveringClusterDownOrCloseCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj)
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
 * @brief  Cluster GoToLiftValue Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToLiftValueCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t liftValue)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftValue Value command received");
    if (hasLift && isPositionAware)
    {
        cover.Lift().TargetValueSet(liftValue);
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
 * @brief  Cluster GoToLiftPercentage Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                            uint8_t liftPercentageValue, uint16_t liftPercent100thsValue)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasLift         = cover.hasFeature(WindowCover::Feature::Lift);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");
    if (hasLift && isPositionAware)
    {
        cover.Lift().TargetSet(
            static_cast<uint16_t>(liftPercentageValue > 100 ? liftPercent100thsValue : liftPercentageValue * 100));
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
 * @brief  Cluster GoToTiltValue Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToTiltValueCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t tiltValue)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltValue command received");
    if (hasTilt && isPositionAware)
    {
        cover.Tilt().TargetValueSet(tiltValue);
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
 * @brief  Cluster GoToTiltPercentage Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                            uint8_t tiltPercentageValue, uint16_t tiltPercent100thsValue)
{
    WindowCover & cover  = WindowCover::Instance();
    bool hasTilt         = cover.hasFeature(WindowCover::Feature::Tilt);
    bool isPositionAware = cover.hasFeature(WindowCover::Feature::PositionAware);
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");
    if (hasTilt && isPositionAware)
    {
        cover.Tilt().TargetSet(
            static_cast<uint16_t>(tiltPercentageValue > 100 ? tiltPercent100thsValue : tiltPercentageValue * 100));
    }
    else
    {
        status = EMBER_ZCL_STATUS_ACTION_DENIED;
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
    }
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
