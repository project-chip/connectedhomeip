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

void TypeSet(chip::EndpointId endpoint, Type type)
{
    Attributes::Type::Set(endpoint, type);
}

Type TypeGet(chip::EndpointId endpoint)
{
    Type value;
    Attributes::Type::Get(endpoint, &value);
    return value;
}

void ConfigStatusPrint(const chip::BitMask<ConfigStatus> & configStatus)
{
    emberAfWindowCoveringClusterPrint("ConfigStatus 0x%02X Operational=%u OnlineReserved=%u", configStatus.Raw(),
                                      configStatus.Has(ConfigStatus::kOperational),
                                      configStatus.Has(ConfigStatus::kOnlineReserved));

    emberAfWindowCoveringClusterPrint(
        "Lift(PA=%u Encoder=%u Reversed=%u) Tilt(PA=%u Encoder=%u)", configStatus.Has(ConfigStatus::kLiftPositionAware),
        configStatus.Has(ConfigStatus::kLiftEncoderControlled), configStatus.Has(ConfigStatus::kLiftMovementReversed),
        configStatus.Has(ConfigStatus::kTiltPositionAware), configStatus.Has(ConfigStatus::kTiltEncoderControlled));
}

void ConfigStatusSet(chip::EndpointId endpoint, const chip::BitMask<ConfigStatus> & configStatus)
{
    Attributes::ConfigStatus::Set(endpoint, configStatus);
}

chip::BitMask<ConfigStatus> ConfigStatusGet(chip::EndpointId endpoint)
{
    chip::BitMask<ConfigStatus> configStatus;
    Attributes::ConfigStatus::Get(endpoint, &configStatus);

    return configStatus;
}

void ConfigStatusUpdateFeatures(chip::EndpointId endpoint)
{
    chip::BitMask<ConfigStatus> configStatus = ConfigStatusGet(endpoint);

    configStatus.Set(ConfigStatus::kLiftPositionAware, HasFeaturePaLift(endpoint));
    configStatus.Set(ConfigStatus::kTiltPositionAware, HasFeaturePaTilt(endpoint));

    if (!HasFeaturePaLift(endpoint))
        configStatus.Clear(ConfigStatus::kLiftEncoderControlled);

    if (!HasFeaturePaTilt(endpoint))
        configStatus.Clear(ConfigStatus::kTiltEncoderControlled);

    ConfigStatusSet(endpoint, configStatus);
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

void EndProductTypeSet(chip::EndpointId endpoint, EndProductType type)
{
    Attributes::EndProductType::Set(endpoint, type);
}

EndProductType EndProductTypeGet(chip::EndpointId endpoint)
{
    EndProductType value;
    Attributes::EndProductType::Get(endpoint, &value);

    return value;
}

void ModePrint(const chip::BitMask<Mode> & mode)
{
    emberAfWindowCoveringClusterPrint("Mode 0x%02X MotorDirReversed=%u LedFeedback=%u Maintenance=%u Calibration=%u", mode.Raw(),
                                      mode.Has(Mode::kMotorDirectionReversed), mode.Has(Mode::kLedFeedback),
                                      mode.Has(Mode::kMaintenanceMode), mode.Has(Mode::kCalibrationMode));
}

void ModeSet(chip::EndpointId endpoint, chip::BitMask<Mode> & newMode)
{
    chip::BitMask<ConfigStatus> newStatus;

    chip::BitMask<ConfigStatus> oldStatus = ConfigStatusGet(endpoint);
    chip::BitMask<Mode> oldMode           = ModeGet(endpoint);

    newStatus = oldStatus;

    // Attribute: ConfigStatus reflects the following current mode flags
    newStatus.Set(ConfigStatus::kOperational, !newMode.HasAny(Mode::kMaintenanceMode, Mode::kCalibrationMode));
    newStatus.Set(ConfigStatus::kLiftMovementReversed, newMode.Has(Mode::kMotorDirectionReversed));

    // Verify only one mode supported at once and maintenance lock goes over calibration
    if (newMode.HasAll(Mode::kMaintenanceMode, Mode::kCalibrationMode))
    {
        newMode.Clear(Mode::kCalibrationMode);
    }

    if (oldMode != newMode)
        Attributes::Mode::Set(endpoint, newMode);

    if (oldStatus != newStatus)
        ConfigStatusSet(endpoint, newStatus);
}

chip::BitMask<Mode> ModeGet(chip::EndpointId endpoint)
{
    chip::BitMask<Mode> mode;

    Attributes::Mode::Get(endpoint, &mode);
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

uint16_t Percent100thsToValue(AbsoluteLimits limits, Percent100ths relative)
{
    return ConvertValue(WC_PERCENT100THS_MIN_OPEN, WC_PERCENT100THS_MAX_CLOSED, limits.open, limits.closed, relative);
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

void LiftPositionSet(chip::EndpointId endpoint, NPercent100ths percent100ths)
{
    NPercent percent;
    NAbsolute rawpos;

    if (percent100ths.IsNull())
    {
        percent.SetNull();
        rawpos.SetNull();
        emberAfWindowCoveringClusterPrint("Lift[%u] Position Set to Null", endpoint);
    }
    else
    {
        percent.SetNonNull(static_cast<uint8_t>(percent100ths.Value() / 100));
        rawpos.SetNonNull(Percent100thsToLift(endpoint, percent100ths.Value()));
        emberAfWindowCoveringClusterPrint("Lift[%u] Position Set: %u", endpoint, percent100ths.Value());
    }
    Attributes::CurrentPositionLift::Set(endpoint, rawpos);
    Attributes::CurrentPositionLiftPercentage::Set(endpoint, percent);
    Attributes::CurrentPositionLiftPercent100ths::Set(endpoint, percent100ths);
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

void TiltPositionSet(chip::EndpointId endpoint, NPercent100ths percent100ths)
{
    NPercent percent;
    NAbsolute rawpos;

    if (percent100ths.IsNull())
    {
        percent.SetNull();
        rawpos.SetNull();
        emberAfWindowCoveringClusterPrint("Tilt[%u] Position Set to Null", endpoint);
    }
    else
    {
        percent.SetNonNull(static_cast<uint8_t>(percent100ths.Value() / 100));
        rawpos.SetNonNull(Percent100thsToTilt(endpoint, percent100ths.Value()));
        emberAfWindowCoveringClusterPrint("Tilt[%u] Position Set: %u", endpoint, percent100ths.Value());
    }
    Attributes::CurrentPositionTilt::Set(endpoint, rawpos);
    Attributes::CurrentPositionTiltPercentage::Set(endpoint, percent);
    Attributes::CurrentPositionTiltPercent100ths::Set(endpoint, percent100ths);
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

Percent100ths ComputePercent100thsStep(OperationalState direction, Percent100ths previous, Percent100ths delta)
{
    Percent100ths percent100ths = previous;

    switch (direction)
    {
    case OperationalState::MovingDownOrClose:
        if (percent100ths < (WC_PERCENT100THS_MAX_CLOSED - delta))
        {
            percent100ths = static_cast<Percent100ths>(percent100ths + delta);
        }
        else
        {
            percent100ths = WC_PERCENT100THS_MAX_CLOSED;
        }
        break;
    case OperationalState::MovingUpOrOpen:
        if (percent100ths > (WC_PERCENT100THS_MIN_OPEN + delta))
        {
            percent100ths = static_cast<Percent100ths>(percent100ths - delta);
        }
        else
        {
            percent100ths = WC_PERCENT100THS_MIN_OPEN;
        }
        break;
    default:
        // nothing to do we keep previous value, simple passthrought
        break;
    }

    if (percent100ths > WC_PERCENT100THS_MAX_CLOSED)
        return WC_PERCENT100THS_MAX_CLOSED;

    return percent100ths;
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
            LiftPositionSet(endpoint, position);
        }
    }

    /* Update position to simulate movement to pass the CI */
    if (OperationalState::Stall != opStatus.tilt)
    {
        Attributes::TargetPositionTiltPercent100ths::Get(endpoint, position);
        if (!position.IsNull())
        {
            TiltPositionSet(endpoint, position);
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

void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId)
{
    // all-cluster-app: simulation for the CI testing
    // otherwise it is defined for manufacturer specific implementation */
    BitMask<Mode> mode;
    BitMask<ConfigStatus> configStatus;
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
    /* Mode change is either internal from the application or external from a write request */
    case Attributes::Mode::Id:
        mode = ModeGet(endpoint);
        ModePrint(mode);
        ModeSet(endpoint, mode); // refilter mode if needed
        break;
    case Attributes::ConfigStatus::Id:
        configStatus = ConfigStatusGet(endpoint);
        ConfigStatusPrint(configStatus);
        break;
    default:
        break;
    }

    /* This decides and triggers fake motion for the selected endpoint */
    if ((opStatus.lift != prevOpStatus.lift) || (opStatus.tilt != prevOpStatus.tilt))
        OperationalStatusSetWithGlobalUpdated(endpoint, opStatus);
}

EmberAfStatus GetMotionLockStatus(chip::EndpointId endpoint)
{
    BitMask<Mode> mode                 = ModeGet(endpoint);
    BitMask<ConfigStatus> configStatus = ConfigStatusGet(endpoint);

    // Is the device locked?
    if (!configStatus.Has(ConfigStatus::kOperational))
    {
        if (mode.Has(Mode::kMaintenanceMode))
        {
            // Mainterance Mode
            return EMBER_ZCL_STATUS_BUSY;
        }

        if (mode.Has(Mode::kCalibrationMode))
        {
            // Calibration Mode
            return EMBER_ZCL_STATUS_FAILURE;
        }
    }

    return EMBER_ZCL_STATUS_SUCCESS;
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

    ConfigStatusUpdateFeatures(endpoint);
}

/**
 * @brief  Cluster UpOrOpen Command callback (from client)
 */
bool emberAfWindowCoveringClusterUpOrOpenCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::UpOrOpen::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("UpOrOpen command received");

    EmberAfStatus status = GetMotionLockStatus(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfWindowCoveringClusterPrint("Err device locked");
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

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

    EmberAfStatus status = GetMotionLockStatus(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfWindowCoveringClusterPrint("Err device locked");
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

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
    app::DataModel::Nullable<Percent100ths> current;
    chip::EndpointId endpoint = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("StopMotion command received");

    EmberAfStatus status = GetMotionLockStatus(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfWindowCoveringClusterPrint("Err device locked");
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

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

    emberAfWindowCoveringClusterPrint("GoToLiftValue %u command received", liftValue);

    EmberAfStatus status = GetMotionLockStatus(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfWindowCoveringClusterPrint("Err device locked");
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

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
    Percent100ths percent100ths = commandData.liftPercent100thsValue;
    EndpointId endpoint         = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("GoToLiftPercentage %u command received", percent100ths);

    EmberAfStatus status = GetMotionLockStatus(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfWindowCoveringClusterPrint("Err device locked");
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

    if (HasFeaturePaLift(endpoint))
    {
        if (IsPercent100thsValid(percent100ths))
        {
            Attributes::TargetPositionLiftPercent100ths::Set(endpoint, percent100ths);
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

    emberAfWindowCoveringClusterPrint("GoToTiltValue %u command received", tiltValue);

    EmberAfStatus status = GetMotionLockStatus(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfWindowCoveringClusterPrint("Err device locked");
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

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
    Percent100ths percent100ths = commandData.tiltPercent100thsValue;
    EndpointId endpoint         = commandPath.mEndpointId;

    emberAfWindowCoveringClusterPrint("GoToTiltPercentage %u command received", percent100ths);

    EmberAfStatus status = GetMotionLockStatus(endpoint);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfWindowCoveringClusterPrint("Err device locked");
        emberAfSendImmediateDefaultResponse(status);
        return true;
    }

    if (HasFeaturePaTilt(endpoint))
    {
        if (IsPercent100thsValid(percent100ths))
        {
            Attributes::TargetPositionTiltPercent100ths::Set(endpoint, percent100ths);
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
 *
 * The method is implemented by default as a weak function and it takes care of updating
 * the server attribute values by calling the PostAttributeChange method. If the application overrides
 * this method, it needs to handle updating attributes (ideally by calling PostAttributeChange).
 *
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
