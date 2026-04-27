/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "WindowCoveringCluster.h"
#include <app/data-model-provider/MetadataTypes.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/WindowCovering/Metadata.h>
#include <lib/support/TypeTraits.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/scenes-server.h> // nogncheck
#endif                                                // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using chip::Protocols::InteractionModel::Status;

namespace {

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

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

Percent100ths ValueToPercent100ths(AbsoluteLimits limits, uint16_t absolute)
{
    return ConvertValue(limits.open, limits.closed, WC_PERCENT100THS_MIN_OPEN, WC_PERCENT100THS_MAX_CLOSED, absolute);
}

WindowCoveringCluster::WindowCoveringCluster(EndpointId endpointId, BitFlags<WindowCovering::Feature> features,
                                             OptionalAttributeSet & optionalAttributeSet, Config & config) :
    DefaultServerCluster(ConcreteClusterPath(endpointId, WindowCovering::Id)), mFeatureMap(features),
    mOptionalAttributes(optionalAttributeSet), mType(config.type), mConfigStatus(config.configStatus),
    mOperationalStatus(config.operationalStatus), mEndProductType(config.endProductType), mMode(config.mode)
{}

CHIP_ERROR WindowCoveringCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    // Load persistent attributes from storage. If a value is not present in storage, the
    // existing member value (set by the constructor) is kept.
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::NumberOfActuationsLift::Id),
        mNumberOfActuationsLift, mNumberOfActuationsLift);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::NumberOfActuationsTilt::Id),
        mNumberOfActuationsTilt, mNumberOfActuationsTilt);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionLiftPercentage::Id),
        mCurrentPositionLiftPercentage, mCurrentPositionLiftPercentage);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionTiltPercentage::Id),
        mCurrentPositionTiltPercentage, mCurrentPositionTiltPercentage);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionLiftPercent100ths::Id),
        mCurrentPositionLiftPercentage100ths, mCurrentPositionLiftPercentage100ths);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionTiltPercent100ths::Id),
        mCurrentPositionTiltPercentage100ths, mCurrentPositionTiltPercentage100ths);

    return CHIP_NO_ERROR;
}

void WindowCoveringCluster::Shutdown(ClusterShutdownType shutdownType)
{
    DefaultServerCluster::Shutdown(shutdownType);
}

void WindowCoveringCluster::SetNumberOfActuationsLift(uint16_t numOfLifts)
{
    VerifyOrReturn(SetAttributeValue(mNumberOfActuationsLift, numOfLifts, Attributes::NumberOfActuationsLift::Id));
    VerifyOrReturn(mContext != nullptr);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::NumberOfActuationsLift::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mNumberOfActuationsLift), sizeof(mNumberOfActuationsLift))));
}

void WindowCoveringCluster::SetNumberOfActuationsTilt(uint16_t numOfTilts)
{
    VerifyOrReturn(SetAttributeValue(mNumberOfActuationsTilt, numOfTilts, Attributes::NumberOfActuationsTilt::Id));
    VerifyOrReturn(mContext != nullptr);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::NumberOfActuationsTilt::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&mNumberOfActuationsTilt), sizeof(mNumberOfActuationsTilt))));
}

void WindowCoveringCluster::SetConfigStatus(chip::BitMask<ConfigStatus> status)
{
    SetAttributeValue(mConfigStatus, status, Attributes::ConfigStatus::Id);
}

void WindowCoveringCluster::SetCurrentPositionLiftPercentage(NPercent curLiftPercentage)
{
    VerifyOrReturn(
        SetAttributeValue(mCurrentPositionLiftPercentage, curLiftPercentage, Attributes::CurrentPositionLiftPercentage::Id));
    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<Percent>::StorageType storageValue;
    DataModel::NullableToStorage(curLiftPercentage, storageValue);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionLiftPercentage::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
}

void WindowCoveringCluster::SetCurrentPositionTiltPercentage(NPercent curTiltPercentage)
{
    VerifyOrReturn(
        SetAttributeValue(mCurrentPositionTiltPercentage, curTiltPercentage, Attributes::CurrentPositionTiltPercentage::Id));
    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<Percent>::StorageType storageValue;
    DataModel::NullableToStorage(curTiltPercentage, storageValue);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionTiltPercentage::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
}

void WindowCoveringCluster::SetOperationalStatus(chip::BitMask<OperationalStatus> newStatus)
{
    SetAttributeValue(mOperationalStatus, newStatus, Attributes::OperationalStatus::Id);
}

void WindowCoveringCluster::SetTargetPositionLiftPercent100ths(NPercent100ths newTargetLift)
{
    VerifyOrReturn(
        SetAttributeValue(mTargetPositionLiftPercentage100ths, newTargetLift, Attributes::TargetPositionLiftPercent100ths::Id));

    OperationalState opLift = ComputeOperationalState(mTargetPositionLiftPercentage100ths, mCurrentPositionLiftPercentage100ths);
    UpdateOperationalStateForField(OperationalStatus::kLift, opLift);
}

void WindowCoveringCluster::SetTargetPositionTiltPercent100ths(NPercent100ths newTargetTilt)
{
    VerifyOrReturn(
        SetAttributeValue(mTargetPositionTiltPercentage100ths, newTargetTilt, Attributes::TargetPositionTiltPercent100ths::Id));

    OperationalState opTilt = ComputeOperationalState(mTargetPositionTiltPercentage100ths, mCurrentPositionTiltPercentage100ths);
    UpdateOperationalStateForField(OperationalStatus::kTilt, opTilt);
}

void WindowCoveringCluster::SetEndProductType(EndProductType type)
{
    SetAttributeValue(mEndProductType, type, Attributes::EndProductType::Id);
}

void WindowCoveringCluster::SetCurrentPositionLiftPercentage100ths(NPercent100ths curLiftPercentage100ths)
{
    VerifyOrReturn(SetAttributeValue(mCurrentPositionLiftPercentage100ths, curLiftPercentage100ths,
                                     Attributes::CurrentPositionLiftPercent100ths::Id));
    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<Percent100ths>::StorageType storageValue;
    DataModel::NullableToStorage(curLiftPercentage100ths, storageValue);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionLiftPercent100ths::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));

    OperationalState opLift = static_cast<OperationalState>(mOperationalStatus.GetField(OperationalStatus::kLift));
    if ((OperationalState::Stall != opLift) && (mCurrentPositionLiftPercentage100ths == mTargetPositionLiftPercentage100ths))
    {
        UpdateOperationalStateForField(OperationalStatus::kLift, OperationalState::Stall);
    }
}

void WindowCoveringCluster::SetCurrentPositionTiltPercentage100ths(NPercent100ths curTiltPercentage100ths)
{
    VerifyOrReturn(SetAttributeValue(mCurrentPositionTiltPercentage100ths, curTiltPercentage100ths,
                                     Attributes::CurrentPositionTiltPercent100ths::Id));
    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<Percent100ths>::StorageType storageValue;
    DataModel::NullableToStorage(curTiltPercentage100ths, storageValue);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionTiltPercent100ths::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));

    OperationalState opTilt = static_cast<OperationalState>(mOperationalStatus.GetField(OperationalStatus::kTilt));
    if ((OperationalState::Stall != opTilt) && (mCurrentPositionTiltPercentage100ths == mTargetPositionTiltPercentage100ths))
    {
        UpdateOperationalStateForField(OperationalStatus::kTilt, OperationalState::Stall);
    }
}

void WindowCoveringCluster::UpdateConfigStatusFromMode()
{
    chip::BitMask<ConfigStatus> newStatus = mConfigStatus;
    newStatus.Set(ConfigStatus::kOperational, !mMode.HasAny(Mode::kMaintenanceMode, Mode::kCalibrationMode));
    newStatus.Set(ConfigStatus::kLiftMovementReversed, mMode.Has(Mode::kMotorDirectionReversed));
    SetConfigStatus(newStatus);
}

void WindowCoveringCluster::SetMode(chip::BitMask<Mode> mode)
{
    // Spec: maintenance lock takes priority over calibration
    if (mode.HasAll(Mode::kMaintenanceMode, Mode::kCalibrationMode))
    {
        mode.Clear(Mode::kCalibrationMode);
    }

    VerifyOrReturn(mMode != mode);
    mMode = mode;
    NotifyAttributeChanged(Attributes::Mode::Id);
    UpdateConfigStatusFromMode();
}

void WindowCoveringCluster::SetSafetyStatus(chip::BitMask<SafetyStatus> status)
{
    SetAttributeValue(mSafetyStatus, status, Attributes::SafetyStatus::Id);
}

DataModel::ActionReturnStatus WindowCoveringCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(WindowCovering::kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(GetFeatureMap());
    case Attributes::Type::Id:
        return encoder.Encode(GetType());
    case Attributes::NumberOfActuationsLift::Id:
        return encoder.Encode(GetNumberOfActuationsLift());
    case Attributes::NumberOfActuationsTilt::Id:
        return encoder.Encode(GetNumberOfActuationsTilt());
    case Attributes::ConfigStatus::Id:
        return encoder.Encode(GetConfigStatus());
    case Attributes::CurrentPositionLiftPercentage::Id:
        return encoder.Encode(GetCurrentPositionLiftPercentage());
    case Attributes::CurrentPositionTiltPercentage::Id:
        return encoder.Encode(GetCurrentPositionTiltPercentage());
    case Attributes::OperationalStatus::Id:
        return encoder.Encode(GetOperationalStatus());
    case Attributes::TargetPositionLiftPercent100ths::Id:
        return encoder.Encode(GetTargetPositionLiftPercent100ths());
    case Attributes::TargetPositionTiltPercent100ths::Id:
        return encoder.Encode(GetTargetPositionTiltPercent100ths());
    case Attributes::EndProductType::Id:
        return encoder.Encode(GetEndProductType());
    case Attributes::CurrentPositionLiftPercent100ths::Id:
        return encoder.Encode(GetCurrentPositionLiftPercentage100ths());
    case Attributes::CurrentPositionTiltPercent100ths::Id:
        return encoder.Encode(GetCurrentPositionTiltPercentage100ths());
    case Attributes::Mode::Id:
        return encoder.Encode(GetMode());
    case Attributes::SafetyStatus::Id:
        return encoder.Encode(GetSafetyStatus());
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus WindowCoveringCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                    AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::Mode::Id: {
        chip::BitMask<Mode> mode;
        ReturnErrorOnFailure(decoder.Decode(mode));
        VerifyOrReturnValue(mode.Raw() <= 0x0F, Status::ConstraintError);
        SetMode(mode);
        return Status::Success;
    }
    default:
        return Status::UnsupportedWrite;
    }
}

CHIP_ERROR WindowCoveringCluster::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasFeature(Feature::kLift), Attributes::NumberOfActuationsLift::kMetadataEntry },
        { HasFeature(Feature::kTilt), Attributes::NumberOfActuationsTilt::kMetadataEntry },
        { HasFeaturePaLift(), Attributes::CurrentPositionLiftPercentage::kMetadataEntry },
        { HasFeaturePaLift(), Attributes::TargetPositionLiftPercent100ths::kMetadataEntry },
        { HasFeaturePaLift(), Attributes::CurrentPositionLiftPercent100ths::kMetadataEntry },
        { HasFeaturePaTilt(), Attributes::CurrentPositionTiltPercentage::kMetadataEntry },
        { HasFeaturePaTilt(), Attributes::TargetPositionTiltPercent100ths::kMetadataEntry },
        { HasFeaturePaTilt(), Attributes::CurrentPositionTiltPercent100ths::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::SafetyStatus::Id), Attributes::SafetyStatus::kMetadataEntry }
    };
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
} // namespace WindowCovering
CHIP_ERROR WindowCoveringCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kMandatoryCommands[] = {
        Commands::UpOrOpen::kMetadataEntry,
        Commands::DownOrClose::kMetadataEntry,
        Commands::StopMotion::kMetadataEntry,
    };

    static constexpr DataModel::AcceptedCommandEntry kGoToLiftPercentageCommand[] = {
        Commands::GoToLiftPercentage::kMetadataEntry,
    };

    static constexpr DataModel::AcceptedCommandEntry kGoToTiltPercentageCommand[] = {
        Commands::GoToTiltPercentage::kMetadataEntry,
    };

    if (HasFeaturePaLift())
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kGoToLiftPercentageCommand));
    }

    ReturnErrorOnFailure(builder.ReferenceExisting(kMandatoryCommands));

    if (HasFeaturePaTilt())
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kGoToTiltPercentageCommand));
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> WindowCoveringCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  chip::TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler)
{
    VerifyOrReturnValue(handler != nullptr, Status::Failure);

    switch (request.path.mCommandId)
    {
    case Commands::UpOrOpen::Id:
        return HandleUpOrOpen();
    case Commands::DownOrClose::Id:
        return HandleDownOrClose();
    case Commands::StopMotion::Id: {
        Commands::StopMotion::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleStopMotion(commandData);
    }
    case Commands::GoToLiftPercentage::Id: {
        Commands::GoToLiftPercentage::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleGoToLiftPercentage(commandData);
    }
    case Commands::GoToTiltPercentage::Id: {
        Commands::GoToTiltPercentage::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleGoToTiltPercentage(commandData);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

void WindowCoveringCluster::UpdateOperationalStateForField(chip::BitMask<OperationalStatus> field, OperationalState state)
{
    if ((OperationalStatus::kLift == field) || (OperationalStatus::kTilt == field))
    {
        chip::BitMask<OperationalStatus> status = mOperationalStatus;
        status.SetField(field, static_cast<uint8_t>(state));
        chip::BitMask<OperationalStatus> opGlobal =
            status.HasAny(OperationalStatus::kLift) ? OperationalStatus::kLift : OperationalStatus::kTilt;
        status.SetField(OperationalStatus::kGlobal, status.GetField(opGlobal));
        SetOperationalStatus(status);
    }
}

Status WindowCoveringCluster::GetMotionLockStatus() const
{
    // Is the device locked?
    if (!mConfigStatus.Has(ConfigStatus::kOperational))
    {
        if (mMode.Has(Mode::kMaintenanceMode))
        {
            // Maintenance Mode
            return Status::Busy;
        }

        if (mMode.Has(Mode::kCalibrationMode))
        {
            // Calibration Mode
            return Status::Failure;
        }
    }

    return Status::Success;
}

bool WindowCoveringCluster::HasFeaturePaLift() const
{
    return (HasFeature(Feature::kLift) && HasFeature(Feature::kPositionAwareLift));
}

bool WindowCoveringCluster::HasFeaturePaTilt() const
{
    return (HasFeature(Feature::kTilt) && HasFeature(Feature::kPositionAwareTilt));
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

void OperationalStatusPrint(const chip::BitMask<OperationalStatus> & opStatus)
{
    ChipLogProgress(Zcl, "OperationalStatus raw=0x%02X global=%u lift=%u tilt=%u", opStatus.Raw(),
                    opStatus.GetField(OperationalStatus::kGlobal), opStatus.GetField(OperationalStatus::kLift),
                    opStatus.GetField(OperationalStatus::kTilt));
}

void ModePrint(const chip::BitMask<Mode> & mode)
{
    ChipLogProgress(Zcl, "Mode 0x%02X MotorDirReversed=%u LedFeedback=%u Maintenance=%u Calibration=%u", mode.Raw(),
                    mode.Has(Mode::kMotorDirectionReversed), mode.Has(Mode::kLedFeedback), mode.Has(Mode::kMaintenanceMode),
                    mode.Has(Mode::kCalibrationMode));
}

// Utility functions
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
    return (percent100ths >= WC_PERCENT100THS_MIN_OPEN) && (percent100ths <= WC_PERCENT100THS_MAX_CLOSED);
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

Status WindowCoveringCluster::HandleUpOrOpen()
{
    ChipLogProgress(Zcl, "UpOrOpen command received");

    Status lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    if (HasFeaturePaLift())
    {
        SetTargetPositionLiftPercent100ths(NPercent100ths(WC_PERCENT100THS_MIN_OPEN));
    }
}

Status GetMotionLockStatus(chip::EndpointId endpoint)
{
    BitMask<Mode> mode = ModeGet(endpoint);

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

    return Status::Success;
}

Status WindowCoveringCluster::HandleDownOrClose()
{
    ChipLogProgress(Zcl, "DownOrClose command received");

    Status lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    if (HasFeaturePaLift())
    {
        SetTargetPositionLiftPercent100ths(NPercent100ths(WC_PERCENT100THS_MAX_CLOSED));
    }
    if (HasFeaturePaTilt())
    {
        SetTargetPositionTiltPercent100ths(NPercent100ths(WC_PERCENT100THS_MAX_CLOSED));
    }

    Delegate * delegate = GetDelegate();
    if (delegate != nullptr)
    {
        if (HasFeaturePaLift())
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift));
        }
        if (HasFeaturePaTilt())
        {
            LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt));
        }
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", GetEndpointId());
    }

    return Status::Success;
}

Status WindowCoveringCluster::HandleStopMotion(const Commands::StopMotion::DecodableType & fields)
{
    ChipLogProgress(Zcl, "StopMotion command received");

    Status lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    bool changeTarget = true;

    Delegate * delegate = GetDelegate();
    if (delegate != nullptr)
    {
        CHIP_ERROR err = delegate->HandleStopMotion();
        if (err == CHIP_ERROR_IN_PROGRESS)
        {
            // Delegate reports motion is still in progress, do not latch the current position as target yet.
            changeTarget = false;
        }
        else
        {
            LogErrorOnFailure(err);
        }
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", GetEndpointId());
    }

    if (changeTarget)
    {
        if (HasFeaturePaLift())
        {
            SetTargetPositionLiftPercent100ths(GetCurrentPositionLiftPercentage100ths());
        }
        if (HasFeaturePaTilt())
        {
            SetTargetPositionTiltPercent100ths(GetCurrentPositionTiltPercentage100ths());
        }
    }

    return Status::Success;
}

Status WindowCoveringCluster::HandleGoToLiftValue(const Commands::GoToLiftValue::DecodableType & commandData)
{
    const auto & liftValue = commandData.liftValue;

    ChipLogProgress(Zcl, "GoToLiftValue %u command received", liftValue);

    Status lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    VerifyOrReturnValue(HasFeature(Feature::kAbsolutePosition) && HasFeaturePaLift(), Status::Failure,
                        ChipLogProgress(Zcl, "Err Device is not PA LF"));

    SetTargetPositionLiftPercent100ths(NPercent100ths(LiftToPercent100ths(GetEndpointId(), liftValue)));

    Delegate * delegate = GetDelegate();
    if (delegate != nullptr)
    {
        LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift));
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", GetEndpointId());
    }

    return Status::Success;
}

Status WindowCoveringCluster::HandleGoToLiftPercentage(const Commands::GoToLiftPercentage::DecodableType & fields)
{
    const Percent100ths percent100ths = fields.liftPercent100thsValue;

    ChipLogProgress(Zcl, "GoToLiftPercentage %u command received", percent100ths);

    Status lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    VerifyOrReturnValue(HasFeaturePaLift(), Status::Failure, ChipLogProgress(Zcl, "Err Device is not PA LF"));
    VerifyOrReturnValue(IsPercent100thsValid(percent100ths), Status::ConstraintError);

    SetTargetPositionLiftPercent100ths(NPercent100ths(percent100ths));

    Delegate * delegate = GetDelegate();
    if (delegate != nullptr)
    {
        LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Lift));
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", GetEndpointId());
    }

    return Status::Success;
}

Status WindowCoveringCluster::HandleGoToTiltValue(const Commands::GoToTiltValue::DecodableType & commandData)
{
    const auto & tiltValue = commandData.tiltValue;

    ChipLogProgress(Zcl, "GoToTiltValue %u command received", tiltValue);

    Status lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    VerifyOrReturnValue(HasFeature(Feature::kAbsolutePosition) && HasFeaturePaTilt(), Status::Failure,
                        ChipLogProgress(Zcl, "Err Device is not PA TL"));

    SetTargetPositionTiltPercent100ths(NPercent100ths(TiltToPercent100ths(GetEndpointId(), tiltValue)));

    Delegate * delegate = GetDelegate();
    if (delegate != nullptr)
    {
        LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt));
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", GetEndpointId());
    }

    return Status::Success;
}

Status WindowCoveringCluster::HandleGoToTiltPercentage(const Commands::GoToTiltPercentage::DecodableType & fields)
{
    const Percent100ths percent100ths = fields.tiltPercent100thsValue;

    ChipLogProgress(Zcl, "GoToTiltPercentage %u command received", percent100ths);

    Status lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    VerifyOrReturnValue(HasFeaturePaTilt(), Status::Failure, ChipLogProgress(Zcl, "Err Device is not PA TL"));
    VerifyOrReturnValue(IsPercent100thsValid(percent100ths), Status::ConstraintError);

    SetTargetPositionTiltPercent100ths(NPercent100ths(percent100ths));

    Delegate * delegate = GetDelegate();
    if (delegate != nullptr)
    {
        LogErrorOnFailure(delegate->HandleMovement(WindowCoveringType::Tilt));
    }
    else
    {
        ChipLogProgress(Zcl, "WindowCovering has no delegate set for endpoint:%u", GetEndpointId());
    }

    return Status::Success;
}

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
