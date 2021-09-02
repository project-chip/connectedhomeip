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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandHandler.h>
#include <app/reporting/reporting.h>
#include <app/util/af-event.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/support/TypeTraits.h>
#include <string.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

using namespace chip::app::Clusters::WindowCovering;

#define WC_PERCENT100THS_MAX 10000

static bool HasFeature(chip::EndpointId endpoint, Features feature)
{
    return true;
}

static uint16_t ValueToPercent100ths(uint16_t openLimit, uint16_t closedLimit, uint16_t value)
{
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

static uint16_t Percent100thsToValue(uint16_t openLimit, uint16_t closedLimit, uint16_t percent100ths)
{
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
    {
        return maximum;
    }

    return static_cast<uint16_t>(minimum + ((range * percent100ths) / WC_PERCENT100THS_MAX));
}

static OperationalState ValueToOperationalState(uint8_t value)
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
static uint8_t OperationalStateToValue(const OperationalState & state)
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

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

bool IsOpen(chip::EndpointId endpoint)
{
    uint16_t liftPosition = 0;
    uint16_t liftLimit    = 0;
    uint16_t tiltPosition = 0;
    uint16_t tiltLimit    = 0;

    Attributes::GetTargetPositionLiftPercent100ths(endpoint, &liftPosition);
    Attributes::GetInstalledOpenLimitLift(endpoint, &liftLimit);
    Attributes::GetTargetPositionTiltPercent100ths(endpoint, &tiltPosition);
    Attributes::GetInstalledOpenLimitTilt(endpoint, &tiltLimit);
    return liftPosition == liftLimit && tiltPosition == tiltLimit;
}

bool IsClosed(chip::EndpointId endpoint)
{
    uint16_t liftPosition = 0;
    uint16_t liftLimit    = 0;
    uint16_t tiltPosition = 0;
    uint16_t tiltLimit    = 0;

    Attributes::GetTargetPositionLiftPercent100ths(endpoint, &liftPosition);
    Attributes::GetInstalledClosedLimitLift(endpoint, &liftLimit);
    Attributes::GetTargetPositionTiltPercent100ths(endpoint, &tiltPosition);
    Attributes::GetInstalledClosedLimitTilt(endpoint, &tiltLimit);
    return liftPosition == liftLimit && tiltPosition == tiltLimit;
}

void TypeSet(chip::EndpointId endpoint, EmberAfWcType type)
{
    Attributes::SetType(endpoint, chip::to_underlying(type));
}

EmberAfWcType TypeGet(chip::EndpointId endpoint)
{
    std::underlying_type<EmberAfWcType>::type value;
    Attributes::GetType(endpoint, &value);
    return static_cast<EmberAfWcType>(value);
}

void ConfigStatusSet(chip::EndpointId endpoint, const ConfigStatus & status)
{
    uint8_t value = (status.operational ? 0x01 : 0) | (status.online ? 0x02 : 0) | (status.liftIsReversed ? 0x04 : 0) |
        (status.liftIsPA ? 0x08 : 0) | (status.tiltIsPA ? 0x10 : 0) | (status.liftIsEncoderControlled ? 0x20 : 0) |
        (status.tiltIsEncoderControlled ? 0x40 : 0);
    Attributes::SetConfigStatus(endpoint, value);
}

const ConfigStatus ConfigStatusGet(chip::EndpointId endpoint)
{
    uint8_t value = 0;
    ConfigStatus status;

    Attributes::GetConfigStatus(endpoint, &value);
    status.operational             = (value & 0x01) ? 1 : 0;
    status.online                  = (value & 0x02) ? 1 : 0;
    status.liftIsReversed          = (value & 0x04) ? 1 : 0;
    status.liftIsPA                = (value & 0x08) ? 1 : 0;
    status.tiltIsPA                = (value & 0x10) ? 1 : 0;
    status.liftIsEncoderControlled = (value & 0x20) ? 1 : 0;
    status.tiltIsEncoderControlled = (value & 0x40) ? 1 : 0;
    return status;
}

void OperationalStatusSet(chip::EndpointId endpoint, const OperationalStatus & status)
{
    uint8_t global = OperationalStateToValue(status.global);
    uint8_t lift   = OperationalStateToValue(status.lift);
    uint8_t tilt   = OperationalStateToValue(status.tilt);
    uint8_t value  = (global & 0x03) | static_cast<uint8_t>((lift & 0x03) << 2) | static_cast<uint8_t>((tilt & 0x03) << 4);
    Attributes::SetOperationalStatus(endpoint, value);
}

const OperationalStatus OperationalStatusGet(chip::EndpointId endpoint)
{
    uint8_t value = 0;
    OperationalStatus status;

    Attributes::GetOperationalStatus(endpoint, &value);
    status.global = ValueToOperationalState(value & 0x03);
    status.lift   = ValueToOperationalState((value >> 2) & 0x03);
    status.tilt   = ValueToOperationalState((value >> 4) & 0x03);
    return status;
}

void EndProductTypeSet(chip::EndpointId endpoint, EmberAfWcEndProductType type)
{
    Attributes::SetEndProductType(endpoint, chip::to_underlying(type));
}

EmberAfWcEndProductType EndProductTypeGet(chip::EndpointId endpoint)
{
    std::underlying_type<EmberAfWcType>::type value;
    Attributes::GetEndProductType(endpoint, &value);
    return static_cast<EmberAfWcEndProductType>(value);
}

void ModeSet(chip::EndpointId endpoint, const Mode & mode)
{
    uint8_t value = (mode.motorDirReversed ? 0x01 : 0) | (mode.calibrationMode ? 0x02 : 0) | (mode.maintenanceMode ? 0x04 : 0) |
        (mode.ledDisplay ? 0x08 : 0);
    Attributes::SetMode(endpoint, value);
}

const Mode ModeGet(chip::EndpointId endpoint)
{
    uint8_t value = 0;
    Mode mode;

    Attributes::GetMode(endpoint, &value);
    mode.motorDirReversed = (value & 0x01) ? 1 : 0;
    mode.calibrationMode  = (value & 0x02) ? 1 : 0;
    mode.maintenanceMode  = (value & 0x04) ? 1 : 0;
    mode.ledDisplay       = (value & 0x08) ? 1 : 0;
    return mode;
}

void SafetyStatusSet(chip::EndpointId endpoint, SafetyStatus & status)
{
    uint16_t value = (status.remoteLockout ? 0x0001 : 0) | (status.tamperDetection ? 0x0002 : 0) |
        (status.failedCommunication ? 0x0004 : 0) | (status.positionFailure ? 0x0008 : 0) |
        (status.thermalProtection ? 0x0010 : 0) | (status.obstacleDetected ? 0x0020 : 0) | (status.powerIssue ? 0x0040 : 0) |
        (status.stopInput ? 0x0080 : 0);
    value |= (uint16_t)(status.motorJammed ? 0x0100 : 0) | (uint16_t)(status.hardwareFailure ? 0x0200 : 0) |
        (uint16_t)(status.manualOperation ? 0x0400 : 0);
    Attributes::SetSafetyStatus(endpoint, value);
}

const SafetyStatus SafetyStatusGet(chip::EndpointId endpoint)
{
    uint16_t value = 0;
    SafetyStatus status;

    Attributes::GetSafetyStatus(endpoint, &value);
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

uint16_t LiftToPercent100ths(chip::EndpointId endpoint, uint16_t lift)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::GetInstalledOpenLimitLift(endpoint, &openLimit);
    Attributes::GetInstalledClosedLimitLift(endpoint, &closedLimit);
    return ValueToPercent100ths(openLimit, closedLimit, lift);
}

uint16_t Percent100thsToLift(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::GetInstalledOpenLimitLift(endpoint, &openLimit);
    Attributes::GetInstalledClosedLimitLift(endpoint, &closedLimit);
    return Percent100thsToValue(openLimit, closedLimit, percent100ths);
}

void LiftPositionSet(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t lift   = Percent100thsToLift(endpoint, percent100ths);

    Attributes::SetCurrentPositionLift(endpoint, lift);
    Attributes::SetCurrentPositionLiftPercentage(endpoint, percent);
    Attributes::SetCurrentPositionLiftPercent100ths(endpoint, percent100ths);
    emberAfWindowCoveringClusterPrint("Lift Position Set: %u%%", percent);
}

uint16_t TiltToPercent100ths(chip::EndpointId endpoint, uint16_t tilt)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::GetInstalledOpenLimitTilt(endpoint, &openLimit);
    Attributes::GetInstalledClosedLimitTilt(endpoint, &closedLimit);
    return ValueToPercent100ths(openLimit, closedLimit, tilt);
}

uint16_t Percent100thsToTilt(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::GetInstalledOpenLimitTilt(endpoint, &openLimit);
    Attributes::GetInstalledClosedLimitTilt(endpoint, &closedLimit);
    return Percent100thsToValue(openLimit, closedLimit, percent100ths);
}

void TiltPositionSet(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t tilt   = Percent100thsToTilt(endpoint, percent100ths);

    Attributes::SetCurrentPositionTilt(endpoint, tilt);
    Attributes::SetCurrentPositionTiltPercentage(endpoint, percent);
    Attributes::SetCurrentPositionTiltPercent100ths(endpoint, percent100ths);
    emberAfWindowCoveringClusterPrint("Tilt Position Set: %u%%", percent);
}

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip

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
    emberAfWindowCoveringClusterPrint("UpOrOpen command received");
    if (HasFeature(endpoint, Features::Lift))
    {
        Attributes::SetTargetPositionLiftPercent100ths(endpoint, 0);
    }
    if (HasFeature(endpoint, Features::Tilt))
    {
        Attributes::SetTargetPositionTiltPercent100ths(endpoint, 0);
    }
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief  Cluster DownOrClose Command callback (from client)
 */
bool emberAfWindowCoveringClusterDownOrCloseCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj)
{
    emberAfWindowCoveringClusterPrint("DownOrClose command received");
    if (HasFeature(endpoint, Features::Lift))
    {
        Attributes::SetTargetPositionLiftPercent100ths(endpoint, WC_PERCENT100THS_MAX);
    }
    if (HasFeature(endpoint, Features::Tilt))
    {
        Attributes::SetTargetPositionTiltPercent100ths(endpoint, WC_PERCENT100THS_MAX);
    }
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief  Cluster GoToLiftValue Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToLiftValueCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t liftValue)
{
    bool hasLift         = HasFeature(endpoint, Features::Lift);
    bool isPositionAware = HasFeature(endpoint, Features::PositionAware);

    emberAfWindowCoveringClusterPrint("GoToLiftValue Value command received");
    if (hasLift && isPositionAware)
    {
        Attributes::SetTargetPositionLiftPercent100ths(endpoint, LiftToPercent100ths(endpoint, liftValue));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or LF=%u", isPositionAware, hasLift);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_ACTION_DENIED);
    }
    return true;
}

/**
 * @brief  Cluster GoToLiftPercentage Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToLiftPercentageCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                            uint8_t liftPercentageValue, uint16_t liftPercent100thsValue)
{
    bool hasLift         = HasFeature(endpoint, Features::Lift);
    bool isPositionAware = HasFeature(endpoint, Features::PositionAware);

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");
    if (hasLift && isPositionAware)
    {
        Attributes::SetTargetPositionLiftPercent100ths(
            endpoint, static_cast<uint16_t>(liftPercentageValue > 100 ? liftPercent100thsValue : liftPercentageValue * 100));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or LF=%u", isPositionAware, hasLift);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_ACTION_DENIED);
    }
    return true;
}

/**
 * @brief  Cluster GoToTiltValue Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToTiltValueCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t tiltValue)
{
    bool hasTilt         = HasFeature(endpoint, Features::Tilt);
    bool isPositionAware = HasFeature(endpoint, Features::PositionAware);

    emberAfWindowCoveringClusterPrint("GoToTiltValue command received");
    if (hasTilt && isPositionAware)
    {
        Attributes::SetTargetPositionTiltPercent100ths(endpoint, TiltToPercent100ths(endpoint, tiltValue));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_ACTION_DENIED);
    }
    return true;
}

/**
 * @brief  Cluster GoToTiltPercentage Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToTiltPercentageCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                            uint8_t tiltPercentageValue, uint16_t tiltPercent100thsValue)
{
    bool hasTilt         = HasFeature(endpoint, Features::Tilt);
    bool isPositionAware = HasFeature(endpoint, Features::PositionAware);

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");
    if (hasTilt && isPositionAware)
    {
        Attributes::SetTargetPositionTiltPercent100ths(
            endpoint, static_cast<uint16_t>(tiltPercentageValue > 100 ? tiltPercent100thsValue : tiltPercentageValue * 100));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA=%u or TL=%u", isPositionAware, hasTilt);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_ACTION_DENIED);
    }
    return true;
}
