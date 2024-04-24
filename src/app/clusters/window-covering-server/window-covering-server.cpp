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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/support/TypeTraits.h>
#include <string.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/scenes-server.h>
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using chip::Protocols::InteractionModel::Status;

#define CHECK_BOUNDS_INVALID(MIN, VAL, MAX) ((VAL < MIN) || (VAL > MAX))
#define CHECK_BOUNDS_VALID(MIN, VAL, MAX) (!CHECK_BOUNDS_INVALID(MIN, VAL, MAX))

#define FAKE_MOTION_DELAY_MS 5000

namespace {

constexpr size_t kWindowCoveringDelegateTableSize =
    MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kWindowCoveringDelegateTableSize <= kEmberInvalidEndpointIndex, "WindowCovering Delegate table size error");

Delegate * gDelegateTable[kWindowCoveringDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, WindowCovering::Id, MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kWindowCoveringDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

/*
 * ConvertValue: Converts values from one range to another
 * Range In  -> from  inputLowValue to   inputHighValue
 * Range Out -> from outputLowValue to outputtHighValue
 */
uint16_t ConvertValue(uint16_t inputLowValue, uint16_t inputHighValue, uint16_t outputLowValue, uint16_t outputHighValue,
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

Percent100ths ValueToPercent100ths(AbsoluteLimits limits, uint16_t absolute)
{
    return ConvertValue(limits.open, limits.closed, WC_PERCENT100THS_MIN_OPEN, WC_PERCENT100THS_MAX_CLOSED, absolute);
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

bool HasFeature(chip::EndpointId endpoint, Feature feature)
{
    bool hasFeature     = false;
    uint32_t featureMap = 0;

    Status status = Attributes::FeatureMap::Get(endpoint, &featureMap);
    if (Status::Success == status)
    {
        hasFeature = (featureMap & chip::to_underlying(feature));
    }

    return hasFeature;
}

bool HasFeaturePaLift(chip::EndpointId endpoint)
{
    return (HasFeature(endpoint, Feature::kLift) && HasFeature(endpoint, Feature::kPositionAwareLift));
}

bool HasFeaturePaTilt(chip::EndpointId endpoint)
{
    return (HasFeature(endpoint, Feature::kTilt) && HasFeature(endpoint, Feature::kPositionAwareTilt));
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
    ChipLogProgress(Zcl, "ConfigStatus 0x%02X Operational=%u OnlineReserved=%u", configStatus.Raw(),
                    configStatus.Has(ConfigStatus::kOperational), configStatus.Has(ConfigStatus::kOnlineReserved));

    ChipLogProgress(Zcl, "Lift(PA=%u Encoder=%u Reversed=%u) Tilt(PA=%u Encoder=%u)",
                    configStatus.Has(ConfigStatus::kLiftPositionAware), configStatus.Has(ConfigStatus::kLiftEncoderControlled),
                    configStatus.Has(ConfigStatus::kLiftMovementReversed), configStatus.Has(ConfigStatus::kTiltPositionAware),
                    configStatus.Has(ConfigStatus::kTiltEncoderControlled));
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

void OperationalStatusPrint(const chip::BitMask<OperationalStatus> & opStatus)
{
    ChipLogProgress(Zcl, "OperationalStatus raw=0x%02X global=%u lift=%u tilt=%u", opStatus.Raw(),
                    opStatus.GetField(OperationalStatus::kGlobal), opStatus.GetField(OperationalStatus::kLift),
                    opStatus.GetField(OperationalStatus::kTilt));
}

chip::BitMask<OperationalStatus> OperationalStatusGet(chip::EndpointId endpoint)
{
    chip::BitMask<OperationalStatus> status;

    Attributes::OperationalStatus::Get(endpoint, &status);

    return status;
}

void OperationalStatusSet(chip::EndpointId endpoint, chip::BitMask<OperationalStatus> newStatus)
{
    chip::BitMask<OperationalStatus> prevStatus;
    Attributes::OperationalStatus::Get(endpoint, &prevStatus);

    // Filter changes
    if (newStatus != prevStatus)
    {
        OperationalStatusPrint(newStatus);
        Attributes::OperationalStatus::Set(endpoint, newStatus);
    }
}

void OperationalStateSet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field, OperationalState state)
{
    chip::BitMask<OperationalStatus> status;
    Attributes::OperationalStatus::Get(endpoint, &status);

    /* Filter only Lift or Tilt action since we cannot allow global reflecting a state alone */
    if ((OperationalStatus::kLift == field) || (OperationalStatus::kTilt == field))
    {
        status.SetField(field, static_cast<uint8_t>(state));
        status.SetField(OperationalStatus::kGlobal, static_cast<uint8_t>(state));

        /* Global Always follow Lift by priority or therefore fallback to Tilt */
        chip::BitMask<OperationalStatus> opGlobal =
            status.HasAny(OperationalStatus::kLift) ? OperationalStatus::kLift : OperationalStatus::kTilt;
        status.SetField(OperationalStatus::kGlobal, status.GetField(opGlobal));

        OperationalStatusSet(endpoint, status);
    }
}

OperationalState OperationalStateGet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field)
{
    chip::BitMask<OperationalStatus> status;

    Attributes::OperationalStatus::Get(endpoint, &status);

    return static_cast<OperationalState>(status.GetField(field));
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
    ChipLogProgress(Zcl, "Mode 0x%02X MotorDirReversed=%u LedFeedback=%u Maintenance=%u Calibration=%u", mode.Raw(),
                    mode.Has(Mode::kMotorDirectionReversed), mode.Has(Mode::kLedFeedback), mode.Has(Mode::kMaintenanceMode),
                    mode.Has(Mode::kCalibrationMode));
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

void SafetyStatusSet(chip::EndpointId endpoint, chip::BitMask<SafetyStatus> & newSafetyStatus)
{
    Attributes::SafetyStatus::Set(endpoint, newSafetyStatus);
}

chip::BitMask<SafetyStatus> SafetyStatusGet(chip::EndpointId endpoint)
{
    chip::BitMask<SafetyStatus> safetyStatus;

    Attributes::SafetyStatus::Get(endpoint, &safetyStatus);
    return safetyStatus;
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
        ChipLogProgress(Zcl, "Lift[%u] Position Set to Null", endpoint);
    }
    else
    {
        percent.SetNonNull(static_cast<uint8_t>(percent100ths.Value() / 100));
        rawpos.SetNonNull(Percent100thsToLift(endpoint, percent100ths.Value()));
        ChipLogProgress(Zcl, "Lift[%u] Position Set: %u", endpoint, percent100ths.Value());
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
        ChipLogProgress(Zcl, "Tilt[%u] Position Set to Null", endpoint);
    }
    else
    {
        percent.SetNonNull(static_cast<uint8_t>(percent100ths.Value() / 100));
        rawpos.SetNonNull(Percent100thsToTilt(endpoint, percent100ths.Value()));
        ChipLogProgress(Zcl, "Tilt[%u] Position Set: %u", endpoint, percent100ths.Value());
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

void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId)
{
    // all-cluster-app: simulation for the CI testing
    // otherwise it is defined for manufacturer specific implementation */
    BitMask<Mode> mode;
    BitMask<ConfigStatus> configStatus;
    NPercent100ths current, target;

    ChipLogProgress(Zcl, "WC POST ATTRIBUTE=%u", (unsigned int) attributeId);

    OperationalState opLift = OperationalStateGet(endpoint, OperationalStatus::kLift);
    OperationalState opTilt = OperationalStateGet(endpoint, OperationalStatus::kTilt);

    switch (attributeId)
    {
    /* ============= Positions for Position Aware ============= */
    case Attributes::CurrentPositionLiftPercent100ths::Id:
        Attributes::TargetPositionLiftPercent100ths::Get(endpoint, target);
        Attributes::CurrentPositionLiftPercent100ths::Get(endpoint, current);
        if ((OperationalState::Stall != opLift) && (current == target))
        {
            ChipLogProgress(Zcl, "Lift stop");
            OperationalStateSet(endpoint, OperationalStatus::kLift, OperationalState::Stall);
        }
        break;
    case Attributes::CurrentPositionTiltPercent100ths::Id:
        Attributes::TargetPositionTiltPercent100ths::Get(endpoint, target);
        Attributes::CurrentPositionTiltPercent100ths::Get(endpoint, current);
        if ((OperationalState::Stall != opTilt) && (current == target))
        {
            ChipLogProgress(Zcl, "Tilt stop");
            OperationalStateSet(endpoint, OperationalStatus::kTilt, OperationalState::Stall);
        }
        break;
    /* For a device supporting Position Awareness : Changing the Target triggers motions on the real or simulated device */
    case Attributes::TargetPositionLiftPercent100ths::Id:
        Attributes::TargetPositionLiftPercent100ths::Get(endpoint, target);
        Attributes::CurrentPositionLiftPercent100ths::Get(endpoint, current);
        opLift = ComputeOperationalState(target, current);
        OperationalStateSet(endpoint, OperationalStatus::kLift, opLift);
        break;
    /* For a device supporting Position Awareness : Changing the Target triggers motions on the real or simulated device */
    case Attributes::TargetPositionTiltPercent100ths::Id:
        Attributes::TargetPositionTiltPercent100ths::Get(endpoint, target);
        Attributes::CurrentPositionTiltPercent100ths::Get(endpoint, current);
        opTilt = ComputeOperationalState(target, current);
        OperationalStateSet(endpoint, OperationalStatus::kTilt, opTilt);
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
}

Status GetMotionLockStatus(chip::EndpointId endpoint)
{
    BitMask<Mode> mode                 = ModeGet(endpoint);
    BitMask<ConfigStatus> configStatus = ConfigStatusGet(endpoint);

    // Is the device locked?
    if (!configStatus.Has(ConfigStatus::kOperational))
    {
        if (mode.Has(Mode::kMaintenanceMode))
        {
            // Mainterance Mode
            return Status::Busy;
        }

        if (mode.Has(Mode::kCalibrationMode))
        {
            // Calibration Mode
            return Status::Failure;
        }
    }

    return Status::Success;
}

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, WindowCovering::Id, MATTER_DM_WINDOW_COVERING_CLUSTER_SERVER_ENDPOINT_COUNT);

    // if endpoint is found
    if (ep < kWindowCoveringDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
        ChipLogProgress(Zcl, "Failed to set WindowCovering delegate for endpoint:%u", endpoint);
    }
}

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip

//------------------------------------------------------------------------------
// Callbacks
//------------------------------------------------------------------------------

/**
 * @brief  Cluster UpOrOpen Command callback (from client)
 */
bool emberAfWindowCoveringClusterUpOrOpenCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::UpOrOpen::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    ChipLogProgress(Zcl, "UpOrOpen command received");

    Status status = GetMotionLockStatus(endpoint);
    if (Status::Success != status)
    {
        ChipLogProgress(Zcl, "Err device locked");
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    if (HasFeature(endpoint, Feature::kPositionAwareLift))
    {
        Attributes::TargetPositionLiftPercent100ths::Set(endpoint, WC_PERCENT100THS_MIN_OPEN);
    }
    if (HasFeature(endpoint, Feature::kPositionAwareTilt))
    {
        Attributes::TargetPositionTiltPercent100ths::Set(endpoint, WC_PERCENT100THS_MIN_OPEN);
    }

    Delegate * delegate = GetDelegate(endpoint);
    if (delegate)
    {
        if (HasFeature(endpoint, Feature::kPositionAwareLift))
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift));
        }

        if (HasFeature(endpoint, Feature::kPositionAwareTilt))
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt));
        }
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", endpoint);
    }

    commandObj->AddStatus(commandPath, Status::Success);

    return true;
}

/**
 * @brief  Cluster DownOrClose Command callback (from client)
 */
bool emberAfWindowCoveringClusterDownOrCloseCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::DownOrClose::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    ChipLogProgress(Zcl, "DownOrClose command received");

    Status status = GetMotionLockStatus(endpoint);
    if (Status::Success != status)
    {
        ChipLogProgress(Zcl, "Err device locked");
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    if (HasFeature(endpoint, Feature::kPositionAwareLift))
    {
        Attributes::TargetPositionLiftPercent100ths::Set(endpoint, WC_PERCENT100THS_MAX_CLOSED);
    }
    if (HasFeature(endpoint, Feature::kPositionAwareTilt))
    {
        Attributes::TargetPositionTiltPercent100ths::Set(endpoint, WC_PERCENT100THS_MAX_CLOSED);
    }
    commandObj->AddStatus(commandPath, Status::Success);

    Delegate * delegate = GetDelegate(endpoint);
    if (delegate)
    {
        if (HasFeature(endpoint, Feature::kPositionAwareLift))
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift));
        }

        if (HasFeature(endpoint, Feature::kPositionAwareTilt))
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt));
        }
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", endpoint);
    }

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

    ChipLogProgress(Zcl, "StopMotion command received");

    Status status = GetMotionLockStatus(endpoint);
    if (Status::Success != status)
    {
        ChipLogProgress(Zcl, "Err device locked");
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    bool changeTarget = true;

    Delegate * delegate = GetDelegate(endpoint);
    if (delegate)
    {
        CHIP_ERROR err = delegate->HandleStopMotion();
        if (err == CHIP_ERROR_IN_PROGRESS)
        {
            changeTarget = false;
        }
        else
        {
            LogErrorOnFailure(err);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", endpoint);
    }

    if (changeTarget)
    {
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
    }

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
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

    ChipLogProgress(Zcl, "GoToLiftValue %u command received", liftValue);

    Status status = GetMotionLockStatus(endpoint);
    if (Status::Success != status)
    {
        ChipLogProgress(Zcl, "Err device locked");
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    if (HasFeature(endpoint, Feature::kAbsolutePosition) && HasFeaturePaLift(endpoint))
    {
        Attributes::TargetPositionLiftPercent100ths::Set(endpoint, LiftToPercent100ths(endpoint, liftValue));
        Delegate * delegate = GetDelegate(endpoint);
        if (delegate)
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift));
        }
        else
        {
            ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", endpoint);
        }
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else
    {
        ChipLogProgress(Zcl, "Err Device is not PA LF");
        commandObj->AddStatus(commandPath, Status::Failure);
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

    ChipLogProgress(Zcl, "GoToLiftPercentage %u command received", percent100ths);

    Status status = GetMotionLockStatus(endpoint);
    if (Status::Success != status)
    {
        ChipLogProgress(Zcl, "Err device locked");
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    if (HasFeaturePaLift(endpoint))
    {
        if (IsPercent100thsValid(percent100ths))
        {
            Attributes::TargetPositionLiftPercent100ths::Set(endpoint, percent100ths);
            Delegate * delegate = GetDelegate(endpoint);
            if (delegate)
            {
                LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift));
            }
            else
            {
                ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", endpoint);
            }
            commandObj->AddStatus(commandPath, Status::Success);
        }
        else
        {
            commandObj->AddStatus(commandPath, Status::ConstraintError);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "Err Device is not PA LF");
        commandObj->AddStatus(commandPath, Status::Failure);
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

    ChipLogProgress(Zcl, "GoToTiltValue %u command received", tiltValue);

    Status status = GetMotionLockStatus(endpoint);
    if (Status::Success != status)
    {
        ChipLogProgress(Zcl, "Err device locked");
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    if (HasFeature(endpoint, Feature::kAbsolutePosition) && HasFeaturePaTilt(endpoint))
    {
        Attributes::TargetPositionTiltPercent100ths::Set(endpoint, TiltToPercent100ths(endpoint, tiltValue));
        Delegate * delegate = GetDelegate(endpoint);
        if (delegate)
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt));
        }
        else
        {
            ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", endpoint);
        }
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else
    {
        ChipLogProgress(Zcl, "Err Device is not PA TL");
        commandObj->AddStatus(commandPath, Status::Failure);
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

    ChipLogProgress(Zcl, "GoToTiltPercentage %u command received", percent100ths);

    Status status = GetMotionLockStatus(endpoint);
    if (Status::Success != status)
    {
        ChipLogProgress(Zcl, "Err device locked");
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    if (HasFeaturePaTilt(endpoint))
    {
        if (IsPercent100thsValid(percent100ths))
        {
            Attributes::TargetPositionTiltPercent100ths::Set(endpoint, percent100ths);
            Delegate * delegate = GetDelegate(endpoint);
            if (delegate)
            {
                LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt));
            }
            else
            {
                ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", endpoint);
            }
            commandObj->AddStatus(commandPath, Status::Success);
        }
        else
        {
            commandObj->AddStatus(commandPath, Status::ConstraintError);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "Err Device is not PA TL");
        commandObj->AddStatus(commandPath, Status::Failure);
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
