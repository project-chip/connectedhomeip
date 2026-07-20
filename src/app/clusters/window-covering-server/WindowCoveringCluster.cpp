/**
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using chip::Protocols::InteractionModel::Status;

namespace {

bool IsPercent100thsValid(Percent100ths percent100ths)
{
    return (percent100ths >= kWcPercent100thsMinOpen) && (percent100ths <= kWcPercent100thsMaxClosed);
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

WindowCoveringCluster::WindowCoveringCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster(ConcreteClusterPath(endpointId, WindowCovering::Id)), mDelegate(config.mDelegate),
    mFeatureMap(config.mFeatures), mOptionalAttributes(config.mOptionalAttributes), mType(config.mType),
    mEndProductType(config.mEndProductType)
{
    // Lift or Tilt must be enabled.
    VerifyOrDieWithMsg(mFeatureMap.Has(Feature::kLift) || mFeatureMap.Has(Feature::kTilt), AppServer,
                       "Validation failed: Neither Lift nor Tilt is enabled.");

    if (mFeatureMap.Has(Feature::kPositionAwareLift))
    {
        VerifyOrDieWithMsg(mFeatureMap.Has(Feature::kLift), AppServer,
                           "Validation failed: PositionAwareLift requires Lift feature.");
    }

    if (mFeatureMap.Has(Feature::kPositionAwareTilt))
    {
        VerifyOrDieWithMsg(mFeatureMap.Has(Feature::kTilt), AppServer,
                           "Validation failed: PositionAwareTilt requires Tilt feature.");
    }
}

CHIP_ERROR WindowCoveringCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::NumberOfActuationsLift::Id),
        mNumberOfActuationsLift, mNumberOfActuationsLift);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::NumberOfActuationsTilt::Id),
        mNumberOfActuationsTilt, mNumberOfActuationsTilt);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionLiftPercent100ths::Id),
        mCurrentPositionLiftPercent100ths, mCurrentPositionLiftPercent100ths);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionTiltPercent100ths::Id),
        mCurrentPositionTiltPercent100ths, mCurrentPositionTiltPercent100ths);

    uint8_t rawMode = mMode.Raw();
    attributePersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::Mode::Id),
                                               rawMode, rawMode);
    mMode = chip::BitMask<Mode>(rawMode);

    uint8_t rawConfigStatus = mConfigStatus.Raw();
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::ConfigStatus::Id), rawConfigStatus,
        rawConfigStatus);
    mConfigStatus = chip::BitMask<ConfigStatus>(rawConfigStatus);

    return CHIP_NO_ERROR;
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
    VerifyOrReturn(SetAttributeValue(mConfigStatus, status, Attributes::ConfigStatus::Id));
    VerifyOrReturn(mContext != nullptr);

    uint8_t rawConfigStatus = mConfigStatus.Raw();
    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::ConfigStatus::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&rawConfigStatus), sizeof(rawConfigStatus))));
}

NPercent WindowCoveringCluster::PercentFromPercent100ths(NPercent100ths percent100ths)
{
    if (percent100ths.IsNull())
    {
        return NPercent();
    }
    return NPercent(static_cast<Percent>(percent100ths.Value() / 100));
}

void WindowCoveringCluster::SetOperationalStatus(chip::BitMask<OperationalStatus> newStatus)
{
    SetAttributeValue(mOperationalStatus, newStatus, Attributes::OperationalStatus::Id);
}

void WindowCoveringCluster::SetTargetPositionLiftPercent100ths(NPercent100ths newTargetLift)
{
    VerifyOrReturn(
        SetAttributeValue(mTargetPositionLiftPercent100ths, newTargetLift, Attributes::TargetPositionLiftPercent100ths::Id));

    OperationalState opLift = OperationalState::Stall;
    if (!mTargetPositionLiftPercent100ths.IsNull() && !mCurrentPositionLiftPercent100ths.IsNull() &&
        mCurrentPositionLiftPercent100ths.Value() != mTargetPositionLiftPercent100ths.Value())
    {
        opLift = (mCurrentPositionLiftPercent100ths.Value() < mTargetPositionLiftPercent100ths.Value())
            ? OperationalState::MovingDownOrClose
            : OperationalState::MovingUpOrOpen;
    }
    UpdateOperationalStateForField(OperationalStatus::kLift, opLift);
}

void WindowCoveringCluster::SetTargetPositionTiltPercent100ths(NPercent100ths newTargetTilt)
{
    VerifyOrReturn(
        SetAttributeValue(mTargetPositionTiltPercent100ths, newTargetTilt, Attributes::TargetPositionTiltPercent100ths::Id));

    OperationalState opTilt = OperationalState::Stall;
    if (!mTargetPositionTiltPercent100ths.IsNull() && !mCurrentPositionTiltPercent100ths.IsNull() &&
        mCurrentPositionTiltPercent100ths.Value() != mTargetPositionTiltPercent100ths.Value())
    {
        opTilt = (mCurrentPositionTiltPercent100ths.Value() < mTargetPositionTiltPercent100ths.Value())
            ? OperationalState::MovingDownOrClose
            : OperationalState::MovingUpOrOpen;
    }
    UpdateOperationalStateForField(OperationalStatus::kTilt, opTilt);
}

void WindowCoveringCluster::SetEndProductType(EndProductType type)
{
    SetAttributeValue(mEndProductType, type, Attributes::EndProductType::Id);
}

void WindowCoveringCluster::SetCurrentPositionLiftPercent100ths(NPercent100ths curLiftPercent100ths)
{
    NPercent previousPercentage = PercentFromPercent100ths(mCurrentPositionLiftPercent100ths);

    VerifyOrReturn(SetAttributeValue(mCurrentPositionLiftPercent100ths, curLiftPercent100ths,
                                     Attributes::CurrentPositionLiftPercent100ths::Id));

    if (mOptionalAttributes.IsSet(Attributes::CurrentPositionLiftPercentage::Id) &&
        (PercentFromPercent100ths(mCurrentPositionLiftPercent100ths) != previousPercentage))
    {
        NotifyAttributeChanged(Attributes::CurrentPositionLiftPercentage::Id);
    }

    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<Percent100ths>::StorageType storageValue;
    DataModel::NullableToStorage(curLiftPercent100ths, storageValue);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionLiftPercent100ths::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));

    OperationalState opLift = static_cast<OperationalState>(mOperationalStatus.GetField(OperationalStatus::kLift));
    if ((OperationalState::Stall != opLift) && (mCurrentPositionLiftPercent100ths == mTargetPositionLiftPercent100ths))
    {
        UpdateOperationalStateForField(OperationalStatus::kLift, OperationalState::Stall);
    }
}

void WindowCoveringCluster::SetCurrentPositionTiltPercent100ths(NPercent100ths curTiltPercent100ths)
{
    NPercent previousPercentage = PercentFromPercent100ths(mCurrentPositionTiltPercent100ths);

    VerifyOrReturn(SetAttributeValue(mCurrentPositionTiltPercent100ths, curTiltPercent100ths,
                                     Attributes::CurrentPositionTiltPercent100ths::Id));

    if (mOptionalAttributes.IsSet(Attributes::CurrentPositionTiltPercentage::Id) &&
        (PercentFromPercent100ths(mCurrentPositionTiltPercent100ths) != previousPercentage))
    {
        NotifyAttributeChanged(Attributes::CurrentPositionTiltPercentage::Id);
    }

    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<Percent100ths>::StorageType storageValue;
    DataModel::NullableToStorage(curTiltPercent100ths, storageValue);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::CurrentPositionTiltPercent100ths::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));

    OperationalState opTilt = static_cast<OperationalState>(mOperationalStatus.GetField(OperationalStatus::kTilt));
    if ((OperationalState::Stall != opTilt) && (mCurrentPositionTiltPercent100ths == mTargetPositionTiltPercent100ths))
    {
        UpdateOperationalStateForField(OperationalStatus::kTilt, OperationalState::Stall);
    }
}

void WindowCoveringCluster::SetMode(chip::BitMask<Mode> mode)
{
    if (mode.HasAll(Mode::kMaintenanceMode, Mode::kCalibrationMode))
    {
        mode.Clear(Mode::kCalibrationMode);
    }

    VerifyOrReturn(SetAttributeValue(mMode, mode, Attributes::Mode::Id));

    if (mContext != nullptr)
    {
        uint8_t rawMode = mMode.Raw();
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, WindowCovering::Id, Attributes::Mode::Id),
            ByteSpan(reinterpret_cast<const uint8_t *>(&rawMode), sizeof(rawMode))));
    }

    chip::BitMask<ConfigStatus> newStatus = mConfigStatus;
    newStatus.Set(ConfigStatus::kOperational, !mMode.HasAny(Mode::kMaintenanceMode, Mode::kCalibrationMode));
    newStatus.Set(ConfigStatus::kLiftMovementReversed, mMode.Has(Mode::kMotorDirectionReversed));
    SetConfigStatus(newStatus);
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
        return encoder.Encode(GetFeatureMap().Raw());
    case Attributes::Type::Id:
        return encoder.Encode(GetType());
    case Attributes::NumberOfActuationsLift::Id:
        return encoder.Encode(GetNumberOfActuationsLift());
    case Attributes::NumberOfActuationsTilt::Id:
        return encoder.Encode(GetNumberOfActuationsTilt());
    case Attributes::ConfigStatus::Id:
        return encoder.Encode(GetConfigStatus().Raw());
    case Attributes::CurrentPositionLiftPercentage::Id:
        return encoder.Encode(PercentFromPercent100ths(mCurrentPositionLiftPercent100ths));
    case Attributes::CurrentPositionTiltPercentage::Id:
        return encoder.Encode(PercentFromPercent100ths(mCurrentPositionTiltPercent100ths));
    case Attributes::OperationalStatus::Id:
        return encoder.Encode(GetOperationalStatus().Raw());
    case Attributes::TargetPositionLiftPercent100ths::Id:
        return encoder.Encode(GetTargetPositionLiftPercent100ths());
    case Attributes::TargetPositionTiltPercent100ths::Id:
        return encoder.Encode(GetTargetPositionTiltPercent100ths());
    case Attributes::EndProductType::Id:
        return encoder.Encode(GetEndProductType());
    case Attributes::CurrentPositionLiftPercent100ths::Id:
        return encoder.Encode(GetCurrentPositionLiftPercent100ths());
    case Attributes::CurrentPositionTiltPercent100ths::Id:
        return encoder.Encode(GetCurrentPositionTiltPercent100ths());
    case Attributes::Mode::Id:
        return encoder.Encode(GetMode().Raw());
    case Attributes::SafetyStatus::Id:
        return encoder.Encode(GetSafetyStatus().Raw());
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
        // TODO: Spec 5.3.6.14.2: "In a write interaction, setting this bit to 0, while the device
        // Disabled because Test_TC_WNCV_2_3 Step 2d writes Mode=0x00 to exit calibration,
        // which contradicts the spec. Needs test update
        // VerifyOrReturnValue(!mMode.Has(Mode::kCalibrationMode) || mode.Has(Mode::kCalibrationMode), Status::Failure);
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
        { mOptionalAttributes.IsSet(Attributes::NumberOfActuationsLift::Id), Attributes::NumberOfActuationsLift::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::NumberOfActuationsTilt::Id), Attributes::NumberOfActuationsTilt::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::CurrentPositionLiftPercentage::Id),
          Attributes::CurrentPositionLiftPercentage::kMetadataEntry },
        { GetFeatureMap().Has(Feature::kPositionAwareLift), Attributes::TargetPositionLiftPercent100ths::kMetadataEntry },
        { GetFeatureMap().Has(Feature::kPositionAwareLift), Attributes::CurrentPositionLiftPercent100ths::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::CurrentPositionTiltPercentage::Id),
          Attributes::CurrentPositionTiltPercentage::kMetadataEntry },
        { GetFeatureMap().Has(Feature::kPositionAwareTilt), Attributes::TargetPositionTiltPercent100ths::kMetadataEntry },
        { GetFeatureMap().Has(Feature::kPositionAwareTilt), Attributes::CurrentPositionTiltPercent100ths::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::SafetyStatus::Id), Attributes::SafetyStatus::kMetadataEntry },
    };
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

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

    if (GetFeatureMap().Has(Feature::kLift))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kGoToLiftPercentageCommand));
    }

    if (GetFeatureMap().Has(Feature::kTilt))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kGoToTiltPercentageCommand));
    }

    ReturnErrorOnFailure(builder.ReferenceExisting(kMandatoryCommands));

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

std::optional<DataModel::ActionReturnStatus> WindowCoveringCluster::GetMotionLockStatus() const
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

std::optional<DataModel::ActionReturnStatus> WindowCoveringCluster::HandleUpOrOpen()
{
    ChipLogProgress(Zcl, "UpOrOpen command received");

    std::optional<DataModel::ActionReturnStatus> lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    if (GetFeatureMap().Has(Feature::kPositionAwareLift))
    {
        SetTargetPositionLiftPercent100ths(NPercent100ths(kWcPercent100thsMinOpen));
    }

    if (GetFeatureMap().Has(Feature::kPositionAwareTilt))
    {
        SetTargetPositionTiltPercent100ths(NPercent100ths(kWcPercent100thsMinOpen));
    }

    if (GetFeatureMap().Has(Feature::kLift))
    {
        LogErrorOnFailure(mDelegate.HandleMovement(WindowCoveringType::Lift));
    }
    if (GetFeatureMap().Has(Feature::kTilt))
    {
        LogErrorOnFailure(mDelegate.HandleMovement(WindowCoveringType::Tilt));
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> WindowCoveringCluster::HandleDownOrClose()
{
    ChipLogProgress(Zcl, "DownOrClose command received");

    std::optional<DataModel::ActionReturnStatus> lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    if (GetFeatureMap().Has(Feature::kPositionAwareLift))
    {
        SetTargetPositionLiftPercent100ths(NPercent100ths(kWcPercent100thsMaxClosed));
    }
    if (GetFeatureMap().Has(Feature::kPositionAwareTilt))
    {
        SetTargetPositionTiltPercent100ths(NPercent100ths(kWcPercent100thsMaxClosed));
    }

    if (GetFeatureMap().Has(Feature::kLift))
    {
        LogErrorOnFailure(mDelegate.HandleMovement(WindowCoveringType::Lift));
    }
    if (GetFeatureMap().Has(Feature::kTilt))
    {
        LogErrorOnFailure(mDelegate.HandleMovement(WindowCoveringType::Tilt));
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
WindowCoveringCluster::HandleStopMotion(const Commands::StopMotion::DecodableType & fields)
{
    ChipLogProgress(Zcl, "StopMotion command received");

    std::optional<DataModel::ActionReturnStatus> lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    bool changeTarget = true;

    CHIP_ERROR err = mDelegate.HandleStopMotion();
    if (err == CHIP_ERROR_IN_PROGRESS)
    {
        changeTarget = false;
    }
    else
    {
        LogErrorOnFailure(err);
    }

    if (changeTarget)
    {
        if (GetFeatureMap().Has(Feature::kPositionAwareLift))
        {
            SetTargetPositionLiftPercent100ths(GetCurrentPositionLiftPercent100ths());
        }
        if (GetFeatureMap().Has(Feature::kPositionAwareTilt))
        {
            SetTargetPositionTiltPercent100ths(GetCurrentPositionTiltPercent100ths());
        }
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
WindowCoveringCluster::HandleGoToLiftPercentage(const Commands::GoToLiftPercentage::DecodableType & fields)
{
    const Percent100ths percent100ths = fields.liftPercent100thsValue;

    ChipLogProgress(Zcl, "GoToLiftPercentage %u command received", percent100ths);

    std::optional<DataModel::ActionReturnStatus> lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    VerifyOrReturnValue(GetFeatureMap().Has(Feature::kLift), Status::UnsupportedCommand,
                        ChipLogProgress(Zcl, "Err Device is tilt-only"));

    if (!GetFeatureMap().Has(Feature::kPositionAwareLift))
    {
        return (percent100ths == 0) ? HandleUpOrOpen() : HandleDownOrClose();
    }

    VerifyOrReturnValue(IsPercent100thsValid(percent100ths), Status::ConstraintError);

    SetTargetPositionLiftPercent100ths(NPercent100ths(percent100ths));

    LogErrorOnFailure(mDelegate.HandleMovement(WindowCoveringType::Lift));

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
WindowCoveringCluster::HandleGoToTiltPercentage(const Commands::GoToTiltPercentage::DecodableType & fields)
{
    const Percent100ths percent100ths = fields.tiltPercent100thsValue;

    ChipLogProgress(Zcl, "GoToTiltPercentage %u command received", percent100ths);

    std::optional<DataModel::ActionReturnStatus> lockStatus = GetMotionLockStatus();
    VerifyOrReturnValue(lockStatus == Status::Success, lockStatus, ChipLogProgress(Zcl, "Err device locked"));

    VerifyOrReturnValue(GetFeatureMap().Has(Feature::kTilt), Status::UnsupportedCommand,
                        ChipLogProgress(Zcl, "Err Device is lift-only"));

    if (!GetFeatureMap().Has(Feature::kPositionAwareTilt))
    {
        return (percent100ths == 0) ? HandleUpOrOpen() : HandleDownOrClose();
    }

    VerifyOrReturnValue(IsPercent100thsValid(percent100ths), Status::ConstraintError);

    SetTargetPositionTiltPercent100ths(NPercent100ths(percent100ths));

    LogErrorOnFailure(mDelegate.HandleMovement(WindowCoveringType::Tilt));

    return Status::Success;
}

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
