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

#include "window-covering-server.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
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

using namespace chip;
using namespace chip::app::Clusters::WindowCovering;

#define CHECK_BOUNDS_INVALID(MIN, VAL, MAX) ((VAL < MIN) || (VAL > MAX))
#define CHECK_BOUNDS_VALID(MIN, VAL, MAX) (!CHECK_BOUNDS_INVALID(MIN, VAL, MAX))

#define FAKE_MOTION_DELAY_MS 5000

/*
 * ConvertValue: Converts values from one range to another
 * Range In  -> from  inputLowValue to   inputHighValue
 * Range Out -> from outputLowValue to outputtHighValue
 */
static uint16_t ConvertValue(uint16_t inputLowValue, uint16_t inputHighValue, uint16_t outputLowValue, uint16_t outputHighValue,
                             uint16_t value)
{
    uint16_t inputMin = inputLowValue, inputMax = inputHighValue, inputRange = UINT16_MAX;
    uint16_t outputMin = outputLowValue, outputMax = outputHighValue, outputRange = UINT16_MAX;

    if (inputLowValue > inputHighValue)
    {
        inputMin = inputHighValue;
        inputMax = inputLowValue;
    }

    if (outputLowValue > outputHighValue)
    {
        outputMin = outputHighValue;
        outputMax = outputLowValue;
    }

    inputRange  = static_cast<uint16_t>(inputMax - inputMin);
    outputRange = static_cast<uint16_t>(outputMax - outputMin);

    if (value < inputMin)
    {
        return outputMin;
    }

    if (value > inputMax)
    {
        return outputMax;
    }

    if (inputRange > 0)
    {
        return static_cast<uint16_t>(outputMin + ((outputRange * (value - inputMin) / inputRange)));
    }

    return outputMax;
}

static Percent100ths ValueToPercent100ths(AbsoluteLimits limits, uint16_t absolute)
{
    return ConvertValue(limits.open, limits.closed, WC_PERCENT100THS_MIN_OPEN, WC_PERCENT100THS_MAX_CLOSED, absolute);
}

static uint16_t Percent100thsToValue(AbsoluteLimits limits, Percent100ths relative)
{
    return ConvertValue(WC_PERCENT100THS_MIN_OPEN, WC_PERCENT100THS_MAX_CLOSED, limits.open, limits.closed, relative);
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

bool HasFeature(chip::EndpointId endpoint, WcFeature feature)
{
    bool hasFeature     = false;
    uint32_t featureMap = 0;

    EmberAfStatus status = Attributes::FeatureMap::Get(endpoint, &featureMap);
    if (EMBER_ZCL_STATUS_SUCCESS == status)
    {
        hasFeature = (featureMap & chip::to_underlying(feature));
    }

    return hasFeature;
}

bool HasFeaturePaLift(chip::EndpointId endpoint)
{
    return (HasFeature(endpoint, WcFeature::kLift) && HasFeature(endpoint, WcFeature::kPositionAwareLift));
}

bool HasFeaturePaTilt(chip::EndpointId endpoint)
{
    return (HasFeature(endpoint, WcFeature::kTilt) && HasFeature(endpoint, WcFeature::kPositionAwareTilt));
}

void TypeSet(chip::EndpointId endpoint, EmberAfWcType type)
{
    Attributes::Type::Set(endpoint, chip::to_underlying(type));
}

EmberAfWcType TypeGet(chip::EndpointId endpoint)
{
    std::underlying_type<EmberAfWcType>::type value;
    Attributes::Type::Get(endpoint, &value);
    return static_cast<EmberAfWcType>(value);
}

void ConfigStatusSet(chip::EndpointId endpoint, const ConfigStatus & status)
{
    /* clang-format off */
    uint8_t value = (status.operational ? 0x01 : 0)
                    | (status.online ? 0x02 : 0)
                    | (status.liftIsReversed ? 0x04 : 0)
                    | (status.liftIsPA ? 0x08 : 0)
                    | (status.tiltIsPA ? 0x10 : 0)
                    | (status.liftIsEncoderControlled ? 0x20 : 0)
                    | (status.tiltIsEncoderControlled ? 0x40 : 0);
    /* clang-format on */
    Attributes::ConfigStatus::Set(endpoint, value);
}

const ConfigStatus ConfigStatusGet(chip::EndpointId endpoint)
{
    uint8_t value = 0;
    ConfigStatus status;

    Attributes::ConfigStatus::Get(endpoint, &value);
    status.operational             = (value & 0x01) ? 1 : 0;
    status.online                  = (value & 0x02) ? 1 : 0;
    status.liftIsReversed          = (value & 0x04) ? 1 : 0;
    status.liftIsPA                = (value & 0x08) ? 1 : 0;
    status.tiltIsPA                = (value & 0x10) ? 1 : 0;
    status.liftIsEncoderControlled = (value & 0x20) ? 1 : 0;
    status.tiltIsEncoderControlled = (value & 0x40) ? 1 : 0;
    return status;
}

void OperationalStatusSetWithGlobalUpdated(chip::EndpointId endpoint, OperationalStatus & status)
{
    /* Global Always follow Lift by priority and then fallback to Tilt */
    if (OperationalState::Stall != status.lift)
    {
        status.global = status.lift;
    }
    else
    {
        status.global = status.tilt;
    }

    OperationalStatusSet(endpoint, status);
}

void OperationalStatusSet(chip::EndpointId endpoint, const OperationalStatus & status)
{
    uint8_t global = OperationalStateToValue(status.global);
    uint8_t lift   = OperationalStateToValue(status.lift);
    uint8_t tilt   = OperationalStateToValue(status.tilt);
    uint8_t value  = (global & 0x03) | static_cast<uint8_t>((lift & 0x03) << 2) | static_cast<uint8_t>((tilt & 0x03) << 4);
    Attributes::OperationalStatus::Set(endpoint, value);
}

const OperationalStatus OperationalStatusGet(chip::EndpointId endpoint)
{
    uint8_t value = 0;
    OperationalStatus status;

    Attributes::OperationalStatus::Get(endpoint, &value);
    status.global = ValueToOperationalState(value & 0x03);
    status.lift   = ValueToOperationalState((value >> 2) & 0x03);
    status.tilt   = ValueToOperationalState((value >> 4) & 0x03);
    return status;
}

void EndProductTypeSet(chip::EndpointId endpoint, EmberAfWcEndProductType type)
{
    Attributes::EndProductType::Set(endpoint, chip::to_underlying(type));
}

EmberAfWcEndProductType EndProductTypeGet(chip::EndpointId endpoint)
{
    std::underlying_type<EmberAfWcType>::type value;
    Attributes::EndProductType::Get(endpoint, &value);
    return static_cast<EmberAfWcEndProductType>(value);
}

void ModeSet(chip::EndpointId endpoint, const Mode & mode)
{
    uint8_t value = (mode.motorDirReversed ? 0x01 : 0) | (mode.calibrationMode ? 0x02 : 0) | (mode.maintenanceMode ? 0x04 : 0) |
        (mode.ledDisplay ? 0x08 : 0);
    Attributes::Mode::Set(endpoint, value);
}

const Mode ModeGet(chip::EndpointId endpoint)
{
    uint8_t value = 0;
    Mode mode;

    Attributes::Mode::Get(endpoint, &value);
    mode.motorDirReversed = (value & 0x01) ? 1 : 0;
    mode.calibrationMode  = (value & 0x02) ? 1 : 0;
    mode.maintenanceMode  = (value & 0x04) ? 1 : 0;
    mode.ledDisplay       = (value & 0x08) ? 1 : 0;
    return mode;
}

void SafetyStatusSet(chip::EndpointId endpoint, SafetyStatus & status)
{
    /* clang-format off */
    uint16_t value = (status.remoteLockout ? 0x0001 : 0)
                     | (status.tamperDetection ? 0x0002 : 0)
                     | (status.failedCommunication ? 0x0004 : 0)
                     | (status.positionFailure ? 0x0008 : 0)
                     | (status.thermalProtection ? 0x0010 : 0)
                     | (status.obstacleDetected ? 0x0020 : 0)
                     | (status.powerIssue ? 0x0040 : 0)
                     | (status.stopInput ? 0x0080 : 0);
    value |= (uint16_t) (status.motorJammed ? 0x0100 : 0)
             | (uint16_t) (status.hardwareFailure ? 0x0200 : 0)
             | (uint16_t) (status.manualOperation ? 0x0400 : 0);
    /* clang-format on */
    Attributes::SafetyStatus::Set(endpoint, value);
}

const SafetyStatus SafetyStatusGet(chip::EndpointId endpoint)
{
    uint16_t value = 0;
    SafetyStatus status;

    Attributes::SafetyStatus::Get(endpoint, &value);
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

LimitStatus CheckLimitState(uint16_t position, AbsoluteLimits limits)
{

    if (limits.open > limits.closed)
        return LimitStatus::Inverted;

    if (position == limits.open)
        return LimitStatus::IsUpOrOpen;

    if (position == limits.closed)
        return LimitStatus::IsDownOrClose;

    if ((limits.open > 0) && (position < limits.open))
        return LimitStatus::IsPastUpOrOpen;

    if ((limits.closed > 0) && (position > limits.closed))
        return LimitStatus::IsPastDownOrClose;

    return LimitStatus::Intermediate;
}

bool IsPercent100thsValid(Percent100ths percent100ths)
{
    if (CHECK_BOUNDS_VALID(WC_PERCENT100THS_MIN_OPEN, percent100ths, WC_PERCENT100THS_MAX_CLOSED))
        return true;

    return false;
}

bool IsPercent100thsValid(NPercent100ths percent100ths)
{
    if (!percent100ths.IsNull())
    {
        return IsPercent100thsValid(percent100ths.Value());
    }

    return true;
}

uint16_t LiftToPercent100ths(chip::EndpointId endpoint, uint16_t lift)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::InstalledOpenLimitLift::Get(endpoint, &openLimit);
    Attributes::InstalledClosedLimitLift::Get(endpoint, &closedLimit);

    AbsoluteLimits limits = { .open = openLimit, .closed = closedLimit };
    return ValueToPercent100ths(limits, lift);
}

uint16_t Percent100thsToLift(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::InstalledOpenLimitLift::Get(endpoint, &openLimit);
    Attributes::InstalledClosedLimitLift::Get(endpoint, &closedLimit);

    AbsoluteLimits limits = { .open = openLimit, .closed = closedLimit };
    return Percent100thsToValue(limits, percent100ths);
}

void LiftPositionSet(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t lift   = Percent100thsToLift(endpoint, percent100ths);

    Attributes::CurrentPositionLift::Set(endpoint, lift);
    Attributes::CurrentPositionLiftPercentage::Set(endpoint, percent);
    Attributes::CurrentPositionLiftPercent100ths::Set(endpoint, percent100ths);
    emberAfWindowCoveringClusterPrint("Lift Position Set: %u%%", percent);
}

uint16_t TiltToPercent100ths(chip::EndpointId endpoint, uint16_t tilt)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::InstalledOpenLimitTilt::Get(endpoint, &openLimit);
    Attributes::InstalledClosedLimitTilt::Get(endpoint, &closedLimit);

    AbsoluteLimits limits = { .open = openLimit, .closed = closedLimit };

    return ValueToPercent100ths(limits, tilt);
}

uint16_t Percent100thsToTilt(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint16_t openLimit   = 0;
    uint16_t closedLimit = 0;
    Attributes::InstalledOpenLimitTilt::Get(endpoint, &openLimit);
    Attributes::InstalledClosedLimitTilt::Get(endpoint, &closedLimit);

    AbsoluteLimits limits = { .open = openLimit, .closed = closedLimit };

    return Percent100thsToValue(limits, percent100ths);
}

void TiltPositionSet(chip::EndpointId endpoint, uint16_t percent100ths)
{
    uint8_t percent = static_cast<uint8_t>(percent100ths / 100);
    uint16_t tilt   = Percent100thsToTilt(endpoint, percent100ths);

    Attributes::CurrentPositionTilt::Set(endpoint, tilt);
    Attributes::CurrentPositionTiltPercentage::Set(endpoint, percent);
    Attributes::CurrentPositionTiltPercent100ths::Set(endpoint, percent100ths);
    emberAfWindowCoveringClusterPrint("Tilt Position Set: %u%%", percent);
}

OperationalState ComputeOperationalState(uint16_t target, uint16_t current)
{
    OperationalState opState = OperationalState::Stall;

    if (current != target)
    {
        opState = (current < target) ? OperationalState::MovingDownOrClose : OperationalState::MovingUpOrOpen;
    }
    return opState;
}

OperationalState ComputeOperationalState(NPercent100ths target, NPercent100ths current)
{
    if (!current.IsNull() && !target.IsNull())
    {
        return ComputeOperationalState(target.Value(), current.Value());
    }
    return OperationalState::Stall;
}

void emberAfPluginWindowCoveringFinalizeFakeMotionEventHandler(EndpointId endpoint)
{
    NPercent100ths position;
    OperationalStatus opStatus = OperationalStatusGet(endpoint);
    emberAfWindowCoveringClusterPrint("WC DELAYED CALLBACK 100ms w/ OpStatus=0x%02X", (unsigned char) opStatus.global);

    /* Update position to simulate movement to pass the CI */
    if (OperationalState::Stall != opStatus.lift)
    {
        Attributes::TargetPositionLiftPercent100ths::Get(endpoint, position);
        if (!position.IsNull())
        {
            LiftPositionSet(endpoint, position.Value());
        }
    }

    /* Update position to simulate movement to pass the CI */
    if (OperationalState::Stall != opStatus.tilt)
    {
        Attributes::TargetPositionTiltPercent100ths::Get(endpoint, position);
        if (!position.IsNull())
        {
            TiltPositionSet(endpoint, position.Value());
        }
    }
}

/**
 * @brief Get event control object for an endpoint
 *
 * @param[in] endpoint
 * @return EmberEventControl*
 */
EmberEventControl * GetEventControl(EndpointId endpoint)
{
    static EmberEventControl eventControls[EMBER_AF_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT];
    uint16_t index            = emberAfFindClusterServerEndpointIndex(endpoint, WindowCovering::Id);
    EmberEventControl * event = nullptr;

    if (index < ArraySize(eventControls))
    {
        event = &eventControls[index];
    }
    return event;
}

/**
 * @brief Configure Fake Motion event control object for an endpoint
 *
 * @param[in] endpoint
 * @return EmberEventControl*
 */
EmberEventControl * ConfigureFakeMotionEventControl(EndpointId endpoint)
{
    EmberEventControl * controller = GetEventControl(endpoint);

    controller->endpoint = endpoint;
    controller->callback = &emberAfPluginWindowCoveringFinalizeFakeMotionEventHandler;

    return controller;
}

/**
 * @brief PostAttributeChange is called when an Attribute is modified
 *
 * @param[in] endpoint
 * @param[in] attributeId
 */
void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId)
{
    // all-cluster-app: simulation for the CI testing
    // otherwise it is defined for manufacturer specific implementation */
    NPercent100ths current, target;
    OperationalStatus prevOpStatus = OperationalStatusGet(endpoint);
    OperationalStatus opStatus     = prevOpStatus;

    emberAfWindowCoveringClusterPrint("WC POST ATTRIBUTE=%u OpStatus global=0x%02X lift=0x%02X tilt=0x%02X",
                                      (unsigned int) attributeId, (unsigned int) opStatus.global, (unsigned int) opStatus.lift,
                                      (unsigned int) opStatus.tilt);

    switch (attributeId)
    {
    /* RO OperationalStatus */
    case Attributes::OperationalStatus::Id:
        if (OperationalState::Stall != opStatus.global)
        {
            // Finish the fake motion attribute update:
            emberEventControlSetDelayMS(ConfigureFakeMotionEventControl(endpoint), FAKE_MOTION_DELAY_MS);
        }
        break;
    /* ============= Positions for Position Aware ============= */
    case Attributes::CurrentPositionLiftPercent100ths::Id:
        if (OperationalState::Stall != opStatus.lift)
        {
            opStatus.lift = OperationalState::Stall;
            emberAfWindowCoveringClusterPrint("Lift stop");
        }
        break;
    case Attributes::CurrentPositionTiltPercent100ths::Id:
        if (OperationalState::Stall != opStatus.tilt)
        {
            opStatus.tilt = OperationalState::Stall;
            emberAfWindowCoveringClusterPrint("Tilt stop");
        }
        break;
    /* For a device supporting Position Awareness : Changing the Target triggers motions on the real or simulated device */
    case Attributes::TargetPositionLiftPercent100ths::Id:
        Attributes::TargetPositionLiftPercent100ths::Get(endpoint, target);
        Attributes::CurrentPositionLiftPercent100ths::Get(endpoint, current);
        opStatus.lift = ComputeOperationalState(target, current);
        break;
    /* For a device supporting Position Awareness : Changing the Target triggers motions on the real or simulated device */
    case Attributes::TargetPositionTiltPercent100ths::Id:
        Attributes::TargetPositionTiltPercent100ths::Get(endpoint, target);
        Attributes::CurrentPositionTiltPercent100ths::Get(endpoint, current);
        opStatus.tilt = ComputeOperationalState(target, current);
        break;
    default:
        break;
    }

    /* This decides and triggers fake motion for the selected endpoint */
    if ((opStatus.lift != prevOpStatus.lift) || (opStatus.tilt != prevOpStatus.tilt))
        OperationalStatusSetWithGlobalUpdated(endpoint, opStatus);
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
bool emberAfWindowCoveringClusterUpOrOpenCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::UpOrOpen::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("UpOrOpen command received");
    if (HasFeature(endpoint, WcFeature::kLift))
    {
        Attributes::TargetPositionLiftPercent100ths::Set(endpoint, WC_PERCENT100THS_MIN_OPEN);
    }
    if (HasFeature(endpoint, WcFeature::kTilt))
    {
        Attributes::TargetPositionTiltPercent100ths::Set(endpoint, WC_PERCENT100THS_MIN_OPEN);
    }
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief  Cluster DownOrClose Command callback (from client)
 */
bool emberAfWindowCoveringClusterDownOrCloseCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::DownOrClose::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("DownOrClose command received");
    if (HasFeature(endpoint, WcFeature::kLift))
    {
        Attributes::TargetPositionLiftPercent100ths::Set(endpoint, WC_PERCENT100THS_MAX_CLOSED);
    }
    if (HasFeature(endpoint, WcFeature::kTilt))
    {
        Attributes::TargetPositionTiltPercent100ths::Set(endpoint, WC_PERCENT100THS_MAX_CLOSED);
    }
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

/**
 * @brief  Cluster StopMotion Command callback (from client)
 */
bool emberAfWindowCoveringClusterStopMotionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::StopMotion::DecodableType & fields)
{
    emberAfWindowCoveringClusterPrint("StopMotion command received");
    app::DataModel::Nullable<Percent100ths> current;
    chip::EndpointId endpoint = commandPath.mEndpointId;

    if (HasFeaturePaLift(endpoint))
    {
        (void) Attributes::CurrentPositionLiftPercent100ths::Get(endpoint, current);
        (void) Attributes::TargetPositionLiftPercent100ths::Set(endpoint, current);
    }

    if (HasFeaturePaTilt(endpoint))
    {
        (void) Attributes::CurrentPositionTiltPercent100ths::Get(endpoint, current);
        (void) Attributes::TargetPositionTiltPercent100ths::Set(endpoint, current);
    }

    return EMBER_SUCCESS == emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
}

/**
 * @brief  Cluster GoToLiftValue Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToLiftValueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::GoToLiftValue::DecodableType & commandData)
{
    auto & liftValue = commandData.liftValue;

    EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("GoToLiftValue Value command received");
    if (HasFeature(endpoint, WcFeature::kAbsolutePosition) && HasFeaturePaLift(endpoint))
    {
        Attributes::TargetPositionLiftPercent100ths::Set(endpoint, LiftToPercent100ths(endpoint, liftValue));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA LF");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

/**
 * @brief  Cluster GoToLiftPercentage Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToLiftPercentageCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::GoToLiftPercentage::DecodableType & commandData)
{
    auto & liftPercentageValue    = commandData.liftPercentageValue;
    auto & liftPercent100thsValue = commandData.liftPercent100thsValue;

    EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage Percentage command received");
    if (HasFeaturePaLift(endpoint))
    {
        if (IsPercent100thsValid(liftPercent100thsValue))
        {
            Attributes::TargetPositionLiftPercent100ths::Set(
                endpoint, static_cast<uint16_t>(liftPercentageValue > 100 ? liftPercent100thsValue : liftPercentageValue * 100));
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        }
        else
        {
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
        }
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA LF");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

/**
 * @brief  Cluster GoToTiltValue Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToTiltValueCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::GoToTiltValue::DecodableType & commandData)
{
    auto & tiltValue = commandData.tiltValue;

    EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("GoToTiltValue command received");
    if (HasFeature(endpoint, WcFeature::kAbsolutePosition) && HasFeaturePaTilt(endpoint))
    {
        Attributes::TargetPositionTiltPercent100ths::Set(endpoint, TiltToPercent100ths(endpoint, tiltValue));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA TL");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

/**
 * @brief  Cluster GoToTiltPercentage Command callback (from client)
 */
bool emberAfWindowCoveringClusterGoToTiltPercentageCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::GoToTiltPercentage::DecodableType & commandData)
{
    auto & tiltPercentageValue    = commandData.tiltPercentageValue;
    auto & tiltPercent100thsValue = commandData.tiltPercent100thsValue;

    EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage command received");
    if (HasFeaturePaTilt(endpoint))
    {
        if (IsPercent100thsValid(tiltPercent100thsValue))
        {
            Attributes::TargetPositionTiltPercent100ths::Set(
                endpoint, static_cast<uint16_t>(tiltPercentageValue > 100 ? tiltPercent100thsValue : tiltPercentageValue * 100));
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
        }
        else
        {
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
        }
    }
    else
    {
        emberAfWindowCoveringClusterPrint("Err Device is not PA TL");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

/**
 * @brief Cluster Attribute Changed Callback
 */
void __attribute__((weak))
MatterWindowCoveringClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
    PostAttributeChange(attributePath.mEndpointId, attributePath.mAttributeId);
}

/**
 * @brief Cluster Plugin Init Callback
 */
void MatterWindowCoveringPluginServerInitCallback() {}
