/*
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
#include <app/clusters/pump-configuration-and-control-server/PumpConfigurationAndControlCluster.h>

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/PumpConfigurationAndControl/EnumsCheck.h>
#include <clusters/PumpConfigurationAndControl/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/odd-sized-integers.h>

namespace chip::app::Clusters {

using namespace PumpConfigurationAndControl;
using namespace PumpConfigurationAndControl::Attributes;

namespace {

/// Largest non-null value of a nullable int24u attribute (0xFFFFFF is the null value).
constexpr uint32_t kMaxNullableInt24u = 0xFFFFFE;

/// Bits defined by PumpStatusBitmap.
constexpr uint16_t kPumpStatusDefinedBits = 0x01FF;

bool IsValidNullableInt24u(const DataModel::Nullable<uint32_t> & value)
{
    return value.IsNull() || value.Value() <= kMaxNullableInt24u;
}

template <typename T>
bool IsValidNullable(const DataModel::Nullable<T> & value)
{
    return value.IsNull() || value.ExistingValueInEncodableRange();
}

ControlModeEnum ComputeEffectiveControlMode(OperationModeEnum operationMode, ControlModeEnum controlMode,
                                            RemoteSensorType sensorType)
{
    switch (operationMode)
    {
    case OperationModeEnum::kMinimum:
    case OperationModeEnum::kMaximum:
        return ControlModeEnum::kConstantSpeed;
    case OperationModeEnum::kNormal:
        switch (sensorType)
        {
        case RemoteSensorType::kFlowSensor:
            return ControlModeEnum::kConstantFlow;
        case RemoteSensorType::kPressureSensor:
            return ControlModeEnum::kConstantPressure;
        case RemoteSensorType::kTemperatureSensor:
            return ControlModeEnum::kConstantTemperature;
        case RemoteSensorType::kNoSensor:
            break;
        }
        return controlMode;
    default:
        // In Local mode the application owns EffectiveControlMode and may override it.
        return controlMode;
    }
}

PumpConfigurationAndControlCluster::PumpStatusBitMask RemoteSensorStatusBits(RemoteSensorType sensorType)
{
    switch (sensorType)
    {
    case RemoteSensorType::kFlowSensor:
        return PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRemoteFlow);
    case RemoteSensorType::kPressureSensor:
        return PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRemotePressure);
    case RemoteSensorType::kTemperatureSensor:
        return PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRemoteTemperature);
    case RemoteSensorType::kNoSensor:
        break;
    }
    return PumpConfigurationAndControlCluster::PumpStatusBitMask();
}

/// Loads a LifetimeRunningHours value stored in the 3-byte int24u attribute format (0xFFFFFF is
/// null), as written by earlier implementations of this cluster. Returns false if there is no such
/// value.
bool LoadInt24uLifetimeRunningHours(AttributePersistenceProvider & provider, const ConcreteAttributePath & path,
                                    DataModel::Nullable<uint32_t> & value)
{
    using Traits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    Traits::StorageType storage;
    MutableByteSpan buffer(storage);
    VerifyOrReturnValue(provider.ReadValue(path, buffer) == CHIP_NO_ERROR && buffer.size() == sizeof(storage), false);
    if (Traits::IsNullValue(storage))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull(Traits::StorageToWorking(storage));
    }
    return true;
}

template <typename T>
void StoreAttribute(ServerClusterContext * context, const ConcreteClusterPath & clusterPath, AttributeId attributeId,
                    const T & value)
{
    VerifyOrReturn(context != nullptr);
    AttributePersistence persistence(context->attributeStorage);
    CHIP_ERROR err = persistence.StoreNativeEndianValue({ clusterPath.mEndpointId, clusterPath.mClusterId, attributeId }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PCC: failed to store attribute 0x%" PRIx32 ": %" CHIP_ERROR_FORMAT, attributeId, err.Format());
    }
}

} // namespace

PumpConfigurationAndControlCluster::EnabledAttributeSet
PumpConfigurationAndControlCluster::BuildEnabledAttributes(BitFlags<Feature> features,
                                                           BitMask<OptionalAttributes> optionalAttributes)
{
    // Limit attributes are mandatory with their feature, optional with the Automatic feature
    // and disallowed otherwise.
    const bool automatic = features.Has(Feature::kAutomatic);
    auto limitEnabled    = [automatic](bool hasFeature, bool requested) { return hasFeature || (automatic && requested); };

    const bool constantPressure    = features.Has(Feature::kConstantPressure);
    const bool compensatedPressure = features.Has(Feature::kCompensatedPressure);
    const bool constantSpeed       = features.Has(Feature::kConstantSpeed);
    const bool constantFlow        = features.Has(Feature::kConstantFlow);
    const bool constantTemperature = features.Has(Feature::kConstantTemperature);

    EnabledAttributeSet enabled;
    enabled.Set<MinConstPressure::Id>(limitEnabled(constantPressure, optionalAttributes.Has(OptionalAttributes::kMinConstPressure)))
        .Set<MaxConstPressure::Id>(limitEnabled(constantPressure, optionalAttributes.Has(OptionalAttributes::kMaxConstPressure)))
        .Set<MinCompPressure::Id>(limitEnabled(compensatedPressure, optionalAttributes.Has(OptionalAttributes::kMinCompPressure)))
        .Set<MaxCompPressure::Id>(limitEnabled(compensatedPressure, optionalAttributes.Has(OptionalAttributes::kMaxCompPressure)))
        .Set<MinConstSpeed::Id>(limitEnabled(constantSpeed, optionalAttributes.Has(OptionalAttributes::kMinConstSpeed)))
        .Set<MaxConstSpeed::Id>(limitEnabled(constantSpeed, optionalAttributes.Has(OptionalAttributes::kMaxConstSpeed)))
        .Set<MinConstFlow::Id>(limitEnabled(constantFlow, optionalAttributes.Has(OptionalAttributes::kMinConstFlow)))
        .Set<MaxConstFlow::Id>(limitEnabled(constantFlow, optionalAttributes.Has(OptionalAttributes::kMaxConstFlow)))
        .Set<MinConstTemp::Id>(limitEnabled(constantTemperature, optionalAttributes.Has(OptionalAttributes::kMinConstTemp)))
        .Set<MaxConstTemp::Id>(limitEnabled(constantTemperature, optionalAttributes.Has(OptionalAttributes::kMaxConstTemp)))
        .Set<PumpStatus::Id>(optionalAttributes.Has(OptionalAttributes::kPumpStatus))
        .Set<Speed::Id>(optionalAttributes.Has(OptionalAttributes::kSpeed))
        .Set<LifetimeRunningHours::Id>(optionalAttributes.Has(OptionalAttributes::kLifetimeRunningHours))
        .Set<Power::Id>(optionalAttributes.Has(OptionalAttributes::kPower))
        .Set<LifetimeEnergyConsumed::Id>(optionalAttributes.Has(OptionalAttributes::kLifetimeEnergyConsumed));
    return enabled;
}

PumpConfigurationAndControlCluster::PumpConfigurationAndControlCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, PumpConfigurationAndControl::Id }), mFeatures(config.features),
    mEnabledOptionalAttributes(BuildEnabledAttributes(config.features, config.optionalAttributes)),
    mControlModeEnabled(config.optionalAttributes.Has(OptionalAttributes::kControlMode)), mFixedAttributes(config.fixedAttributes),
    mDelegate(config.delegate),
    mOperationMode(IsOperationModeSupported(config.operationMode) ? config.operationMode : OperationModeEnum::kNormal),
    mControlMode(SelectInitialControlMode(config.controlMode)), mEffectiveOperationMode(mOperationMode),
    mEffectiveControlMode(ComputeEffectiveControlMode(mOperationMode, mControlMode, RemoteSensorType::kNoSensor)),
    mLifetimeRunningHours(0), mLifetimeEnergyConsumed(0)
{}

CHIP_ERROR PumpConfigurationAndControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence persistence(context.attributeStorage);
    const EndpointId endpointId = mPath.mEndpointId;
    const ClusterId clusterId   = mPath.mClusterId;

    const OperationModeEnum defaultOperationMode = mOperationMode;
    persistence.LoadNativeEndianValue({ endpointId, clusterId, OperationMode::Id }, mOperationMode, defaultOperationMode);
    if (!IsOperationModeSupported(mOperationMode))
    {
        mOperationMode = OperationModeEnum::kNormal;
    }

    if (mControlModeEnabled)
    {
        const ControlModeEnum defaultControlMode = mControlMode;
        persistence.LoadNativeEndianValue({ endpointId, clusterId, ControlMode::Id }, mControlMode, defaultControlMode);
        if (!IsControlModeSupported(mControlMode))
        {
            mControlMode = defaultControlMode;
        }
    }

    // The effective modes are rebuilt from the restored modes instead of trusting the persisted
    // value, which can be out of date after an interrupted update or a FeatureMap change. Only the
    // EffectiveControlMode set by the application in Local mode is kept. EffectiveOperationMode is
    // not persisted: it always follows the persisted OperationMode. The delegate is not notified:
    // it may not be ready yet, and Level Control may not be initialized.
    ControlModeEnum storedEffectiveControlMode = ControlModeEnum::kUnknownEnumValue;
    const bool hasStoredEffectiveControlMode   = persistence.LoadNativeEndianValue(
        { endpointId, clusterId, EffectiveControlMode::Id }, storedEffectiveControlMode, ControlModeEnum::kUnknownEnumValue);

    const RemoteSensorType sensorType =
        (mOperationMode == OperationModeEnum::kNormal) ? mDelegate.GetRemoteSensorType(endpointId) : RemoteSensorType::kNoSensor;
    mEffectiveOperationMode = mOperationMode;
    mEffectiveControlMode   = (mOperationMode == OperationModeEnum::kLocal && IsControlModeSupported(storedEffectiveControlMode))
          ? storedEffectiveControlMode
          : ComputeEffectiveControlMode(mOperationMode, mControlMode, sensorType);
    if (mEnabledOptionalAttributes.IsSet(PumpStatus::Id))
    {
        mPumpStatus.Clear(PumpStatusBitmap::kRemotePressure)
            .Clear(PumpStatusBitmap::kRemoteFlow)
            .Clear(PumpStatusBitmap::kRemoteTemperature)
            .Set(RemoteSensorStatusBits(sensorType));
    }
    if (hasStoredEffectiveControlMode && mEffectiveControlMode != storedEffectiveControlMode)
    {
        StoreAttribute(mContext, mPath, EffectiveControlMode::Id, mEffectiveControlMode);
    }

    if (mEnabledOptionalAttributes.IsSet(LifetimeRunningHours::Id))
    {
        const DataModel::Nullable<uint32_t> defaultHours = mLifetimeRunningHours;
        const ConcreteAttributePath path(endpointId, clusterId, LifetimeRunningHours::Id);
        if (!persistence.LoadNativeEndianValue(path, mLifetimeRunningHours, defaultHours) &&
            LoadInt24uLifetimeRunningHours(context.attributeStorage, path, mLifetimeRunningHours))
        {
            // Store the value in the current format, so that later loads do not need the conversion.
            StoreAttribute(mContext, mPath, LifetimeRunningHours::Id, mLifetimeRunningHours);
        }
        if (!IsValidNullableInt24u(mLifetimeRunningHours))
        {
            mLifetimeRunningHours = defaultHours;
        }
    }

    if (mEnabledOptionalAttributes.IsSet(LifetimeEnergyConsumed::Id))
    {
        const DataModel::Nullable<uint32_t> defaultEnergy = mLifetimeEnergyConsumed;
        persistence.LoadNativeEndianValue({ endpointId, clusterId, LifetimeEnergyConsumed::Id }, mLifetimeEnergyConsumed,
                                          defaultEnergy);
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus PumpConfigurationAndControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(PumpConfigurationAndControl::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case MaxPressure::Id:
        return encoder.Encode(mFixedAttributes.maxPressure);
    case MaxSpeed::Id:
        return encoder.Encode(mFixedAttributes.maxSpeed);
    case MaxFlow::Id:
        return encoder.Encode(mFixedAttributes.maxFlow);
    case MinConstPressure::Id:
        return encoder.Encode(mFixedAttributes.minConstPressure);
    case MaxConstPressure::Id:
        return encoder.Encode(mFixedAttributes.maxConstPressure);
    case MinCompPressure::Id:
        return encoder.Encode(mFixedAttributes.minCompPressure);
    case MaxCompPressure::Id:
        return encoder.Encode(mFixedAttributes.maxCompPressure);
    case MinConstSpeed::Id:
        return encoder.Encode(mFixedAttributes.minConstSpeed);
    case MaxConstSpeed::Id:
        return encoder.Encode(mFixedAttributes.maxConstSpeed);
    case MinConstFlow::Id:
        return encoder.Encode(mFixedAttributes.minConstFlow);
    case MaxConstFlow::Id:
        return encoder.Encode(mFixedAttributes.maxConstFlow);
    case MinConstTemp::Id:
        return encoder.Encode(mFixedAttributes.minConstTemp);
    case MaxConstTemp::Id:
        return encoder.Encode(mFixedAttributes.maxConstTemp);
    case PumpStatus::Id:
        return encoder.Encode(mPumpStatus);
    case EffectiveOperationMode::Id:
        return encoder.Encode(mEffectiveOperationMode);
    case EffectiveControlMode::Id:
        return encoder.Encode(mEffectiveControlMode);
    case Capacity::Id:
        return encoder.Encode(mCapacity);
    case Speed::Id:
        return encoder.Encode(mSpeed);
    case LifetimeRunningHours::Id:
        return encoder.Encode(mLifetimeRunningHours);
    case Power::Id:
        return encoder.Encode(mPower);
    case LifetimeEnergyConsumed::Id:
        return encoder.Encode(mLifetimeEnergyConsumed);
    case OperationMode::Id:
        return encoder.Encode(mOperationMode);
    case ControlMode::Id:
        return encoder.Encode(mControlMode);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus PumpConfigurationAndControlCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                                 AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case OperationMode::Id: {
        OperationModeEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetOperationMode(value);
    }
    case ControlMode::Id: {
        ControlModeEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetControlMode(value);
    }
    case LifetimeRunningHours::Id: {
        DataModel::Nullable<uint32_t> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetLifetimeRunningHours(value);
    }
    case LifetimeEnergyConsumed::Id: {
        DataModel::Nullable<uint32_t> value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetLifetimeEnergyConsumed(value);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR PumpConfigurationAndControlCluster::Attributes(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    constexpr DataModel::AttributeEntry optionalAttributes[] = {
        MinConstPressure::kMetadataEntry,       //
        MaxConstPressure::kMetadataEntry,       //
        MinCompPressure::kMetadataEntry,        //
        MaxCompPressure::kMetadataEntry,        //
        MinConstSpeed::kMetadataEntry,          //
        MaxConstSpeed::kMetadataEntry,          //
        MinConstFlow::kMetadataEntry,           //
        MaxConstFlow::kMetadataEntry,           //
        MinConstTemp::kMetadataEntry,           //
        MaxConstTemp::kMetadataEntry,           //
        PumpStatus::kMetadataEntry,             //
        Speed::kMetadataEntry,                  //
        LifetimeRunningHours::kMetadataEntry,   //
        Power::kMetadataEntry,                  //
        LifetimeEnergyConsumed::kMetadataEntry, //
    };

    AttributeListBuilder listBuilder(builder);
    ReturnErrorOnFailure(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mEnabledOptionalAttributes));

    if (mControlModeEnabled)
    {
        return builder.AppendElements({ ControlMode::kMetadataEntry });
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetOperationMode(OperationModeEnum operationMode)
{
    VerifyOrReturnError(IsOperationModeSupported(operationMode), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(SetAttributeValue(mOperationMode, operationMode, OperationMode::Id), CHIP_NO_ERROR);
    StoreAttribute(mContext, mPath, OperationMode::Id, mOperationMode);

    ChangedAttributes changed;
    changed.Add(OperationMode::Id);
    UpdateEffectiveModes(changed);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetControlMode(ControlModeEnum controlMode)
{
    VerifyOrReturnError(mControlModeEnabled, CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsControlModeSupported(controlMode), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(SetAttributeValue(mControlMode, controlMode, ControlMode::Id), CHIP_NO_ERROR);
    StoreAttribute(mContext, mPath, ControlMode::Id, mControlMode);

    ChangedAttributes changed;
    changed.Add(ControlMode::Id);
    UpdateEffectiveModes(changed);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetEffectiveControlMode(ControlModeEnum controlMode)
{
    VerifyOrReturnError(EnsureKnownEnumValue(controlMode) != ControlModeEnum::kUnknownEnumValue,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (WriteEffectiveControlMode(controlMode))
    {
        mDelegate.OnAttributeChanged(mPath.mEndpointId, EffectiveControlMode::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetEffectiveOperationMode(OperationModeEnum operationMode)
{
    VerifyOrReturnError(EnsureKnownEnumValue(operationMode) != OperationModeEnum::kUnknownEnumValue,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mEffectiveOperationMode, operationMode, EffectiveOperationMode::Id))
    {
        mDelegate.OnAttributeChanged(mPath.mEndpointId, EffectiveOperationMode::Id);
    }
    return CHIP_NO_ERROR;
}

bool PumpConfigurationAndControlCluster::WriteEffectiveControlMode(ControlModeEnum controlMode)
{
    VerifyOrReturnValue(SetAttributeValue(mEffectiveControlMode, controlMode, EffectiveControlMode::Id), false);
    StoreAttribute(mContext, mPath, EffectiveControlMode::Id, mEffectiveControlMode);
    return true;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetPumpStatus(PumpStatusBitMask pumpStatus)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(PumpStatus::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError((pumpStatus.Raw() & ~kPumpStatusDefinedBits) == 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdateAttribute(mPumpStatus, pumpStatus, PumpStatus::Id);
    return CHIP_NO_ERROR;
}

// TODO: Capacity, Speed and Power have the Q quality since cluster revision 6: report changes at
// most once per second (and on null transitions) using QuieterReportingAttribute.
CHIP_ERROR PumpConfigurationAndControlCluster::SetCapacity(DataModel::Nullable<int16_t> capacity)
{
    VerifyOrReturnError(IsValidNullable(capacity), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdateAttribute(mCapacity, capacity, Capacity::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetSpeed(DataModel::Nullable<uint16_t> speed)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(Speed::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullable(speed), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdateAttribute(mSpeed, speed, Speed::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetPower(DataModel::Nullable<uint32_t> power)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(Power::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullableInt24u(power), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdateAttribute(mPower, power, Power::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetLifetimeRunningHours(DataModel::Nullable<uint32_t> hours)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(LifetimeRunningHours::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullableInt24u(hours), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mLifetimeRunningHours, hours, LifetimeRunningHours::Id))
    {
        StoreAttribute(mContext, mPath, LifetimeRunningHours::Id, mLifetimeRunningHours);
        mDelegate.OnAttributeChanged(mPath.mEndpointId, LifetimeRunningHours::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetLifetimeEnergyConsumed(DataModel::Nullable<uint32_t> energy)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(LifetimeEnergyConsumed::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullable(energy), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mLifetimeEnergyConsumed, energy, LifetimeEnergyConsumed::Id))
    {
        StoreAttribute(mContext, mPath, LifetimeEnergyConsumed::Id, mLifetimeEnergyConsumed);
        mDelegate.OnAttributeChanged(mPath.mEndpointId, LifetimeEnergyConsumed::Id);
    }
    return CHIP_NO_ERROR;
}

bool PumpConfigurationAndControlCluster::IsOperationModeSupported(OperationModeEnum operationMode) const
{
    switch (operationMode)
    {
    case OperationModeEnum::kNormal:
        return true;
    case OperationModeEnum::kMinimum:
    case OperationModeEnum::kMaximum:
        return mFeatures.Has(Feature::kConstantSpeed);
    case OperationModeEnum::kLocal:
        return mFeatures.Has(Feature::kLocalOperation);
    default:
        return false;
    }
}

bool PumpConfigurationAndControlCluster::IsControlModeSupported(ControlModeEnum controlMode) const
{
    switch (controlMode)
    {
    case ControlModeEnum::kConstantSpeed:
        return mFeatures.Has(Feature::kConstantSpeed);
    case ControlModeEnum::kConstantPressure:
        return mFeatures.Has(Feature::kConstantPressure);
    case ControlModeEnum::kProportionalPressure:
        return mFeatures.Has(Feature::kCompensatedPressure);
    case ControlModeEnum::kConstantFlow:
        return mFeatures.Has(Feature::kConstantFlow);
    case ControlModeEnum::kConstantTemperature:
        return mFeatures.Has(Feature::kConstantTemperature);
    case ControlModeEnum::kAutomatic:
        return mFeatures.Has(Feature::kAutomatic);
    default:
        return false;
    }
}

ControlModeEnum PumpConfigurationAndControlCluster::SelectInitialControlMode(std::optional<ControlModeEnum> configured) const
{
    if (configured.has_value() && IsControlModeSupported(*configured))
    {
        return *configured;
    }
    for (ControlModeEnum mode :
         { ControlModeEnum::kConstantSpeed, ControlModeEnum::kConstantPressure, ControlModeEnum::kProportionalPressure,
           ControlModeEnum::kConstantFlow, ControlModeEnum::kConstantTemperature, ControlModeEnum::kAutomatic })
    {
        if (IsControlModeSupported(mode))
        {
            return mode;
        }
    }
    // No mode feature is enabled, which the spec does not allow. Use the spec fallback value.
    return ControlModeEnum::kConstantSpeed;
}

bool PumpConfigurationAndControlCluster::IsAttributeEnabled(AttributeId attributeId) const
{
    if (attributeId == ControlMode::Id)
    {
        return mControlModeEnabled;
    }
    if (attributeId < 32 && (EnabledAttributeSet::All() & (1u << attributeId)) != 0)
    {
        return mEnabledOptionalAttributes.IsSet(attributeId);
    }
    for (const auto & entry : kMandatoryMetadata)
    {
        if (entry.attributeId == attributeId)
        {
            return true;
        }
    }
    return false;
}

void PumpConfigurationAndControlCluster::OnRemoteSensorChanged()
{
    VerifyOrReturn(mOperationMode == OperationModeEnum::kNormal);
    ChangedAttributes changed;
    UpdateEffectiveModes(changed);
}

void PumpConfigurationAndControlCluster::UpdateEffectiveModes(ChangedAttributes & changed)
{
    const EndpointId endpointId = mPath.mEndpointId;
    const RemoteSensorType sensorType =
        (mOperationMode == OperationModeEnum::kNormal) ? mDelegate.GetRemoteSensorType(endpointId) : RemoteSensorType::kNoSensor;

    if (SetAttributeValue(mEffectiveOperationMode, mOperationMode, EffectiveOperationMode::Id))
    {
        changed.Add(EffectiveOperationMode::Id);
    }
    if (WriteEffectiveControlMode(ComputeEffectiveControlMode(mOperationMode, mControlMode, sensorType)))
    {
        changed.Add(EffectiveControlMode::Id);
    }
    if (mEnabledOptionalAttributes.IsSet(PumpStatus::Id))
    {
        PumpStatusBitMask pumpStatus = mPumpStatus;
        pumpStatus.Clear(PumpStatusBitmap::kRemotePressure)
            .Clear(PumpStatusBitmap::kRemoteFlow)
            .Clear(PumpStatusBitmap::kRemoteTemperature)
            .Set(RemoteSensorStatusBits(sensorType));
        if (SetAttributeValue(mPumpStatus, pumpStatus, PumpStatus::Id))
        {
            changed.Add(PumpStatus::Id);
        }
    }

    // All values are written: delegate calls below may call back into the cluster.
    if (mOperationMode == OperationModeEnum::kMinimum || mOperationMode == OperationModeEnum::kMaximum)
    {
        mDelegate.OnOperationModeMinMax(endpointId, mOperationMode);
    }
    for (AttributeId attributeId : changed)
    {
        mDelegate.OnAttributeChanged(endpointId, attributeId);
    }
}

} // namespace chip::app::Clusters
