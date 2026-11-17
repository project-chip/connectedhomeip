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

namespace chip::app::Clusters {

using namespace PumpConfigurationAndControl;
using namespace PumpConfigurationAndControl::Attributes;

namespace {

/// Largest non-null value of a nullable int24u attribute (0xFFFFFF is the null value).
constexpr uint32_t kMaxNullableInt24u = 0xFFFFFE;

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
                                                           const OptionalAttributes & optionalAttributes)
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
    enabled.Set<MinConstPressure::Id>(limitEnabled(constantPressure, optionalAttributes.minConstPressure))
        .Set<MaxConstPressure::Id>(limitEnabled(constantPressure, optionalAttributes.maxConstPressure))
        .Set<MinCompPressure::Id>(limitEnabled(compensatedPressure, optionalAttributes.minCompPressure))
        .Set<MaxCompPressure::Id>(limitEnabled(compensatedPressure, optionalAttributes.maxCompPressure))
        .Set<MinConstSpeed::Id>(limitEnabled(constantSpeed, optionalAttributes.minConstSpeed))
        .Set<MaxConstSpeed::Id>(limitEnabled(constantSpeed, optionalAttributes.maxConstSpeed))
        .Set<MinConstFlow::Id>(limitEnabled(constantFlow, optionalAttributes.minConstFlow))
        .Set<MaxConstFlow::Id>(limitEnabled(constantFlow, optionalAttributes.maxConstFlow))
        .Set<MinConstTemp::Id>(limitEnabled(constantTemperature, optionalAttributes.minConstTemp))
        .Set<MaxConstTemp::Id>(limitEnabled(constantTemperature, optionalAttributes.maxConstTemp))
        .Set<PumpStatus::Id>(optionalAttributes.pumpStatus)
        .Set<Speed::Id>(optionalAttributes.speed)
        .Set<LifetimeRunningHours::Id>(optionalAttributes.lifetimeRunningHours)
        .Set<Power::Id>(optionalAttributes.power)
        .Set<LifetimeEnergyConsumed::Id>(optionalAttributes.lifetimeEnergyConsumed);
    return enabled;
}

PumpConfigurationAndControlCluster::PumpConfigurationAndControlCluster(EndpointId endpointId, const Config & config) :
    DefaultServerCluster({ endpointId, PumpConfigurationAndControl::Id }), mFeatures(config.features),
    mEnabledOptionalAttributes(BuildEnabledAttributes(config.features, config.optionalAttributes)),
    mControlModeEnabled(config.optionalAttributes.controlMode), mFixedAttributes(config.fixedAttributes),
    mDelegate(config.delegate), mOperationMode(config.operationMode), mControlMode(config.controlMode),
    mEffectiveOperationMode(config.operationMode),
    mEffectiveControlMode(ComputeEffectiveControlMode(config.operationMode, config.controlMode, RemoteSensorType::kNoSensor)),
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
        if (EnsureKnownEnumValue(mControlMode) == ControlModeEnum::kUnknownEnumValue)
        {
            mControlMode = defaultControlMode;
        }
    }

    const OperationModeEnum defaultEffectiveOperationMode = mOperationMode;
    const ControlModeEnum defaultEffectiveControlMode =
        ComputeEffectiveControlMode(mOperationMode, mControlMode, RemoteSensorType::kNoSensor);
    persistence.LoadNativeEndianValue({ endpointId, clusterId, EffectiveOperationMode::Id }, mEffectiveOperationMode,
                                      defaultEffectiveOperationMode);
    persistence.LoadNativeEndianValue({ endpointId, clusterId, EffectiveControlMode::Id }, mEffectiveControlMode,
                                      defaultEffectiveControlMode);
    if (EnsureKnownEnumValue(mEffectiveOperationMode) == OperationModeEnum::kUnknownEnumValue)
    {
        mEffectiveOperationMode = defaultEffectiveOperationMode;
    }
    if (EnsureKnownEnumValue(mEffectiveControlMode) == ControlModeEnum::kUnknownEnumValue)
    {
        mEffectiveControlMode = defaultEffectiveControlMode;
    }

    if (mEnabledOptionalAttributes.IsSet(LifetimeRunningHours::Id))
    {
        const DataModel::Nullable<uint32_t> defaultHours = mLifetimeRunningHours;
        persistence.LoadNativeEndianValue({ endpointId, clusterId, LifetimeRunningHours::Id }, mLifetimeRunningHours, defaultHours);
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
    UpdateEffectiveModes();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetControlMode(ControlModeEnum controlMode)
{
    VerifyOrReturnError(IsControlModeSupported(controlMode), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(SetAttributeValue(mControlMode, controlMode, ControlMode::Id), CHIP_NO_ERROR);
    StoreAttribute(mContext, mPath, ControlMode::Id, mControlMode);
    UpdateEffectiveModes();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetEffectiveControlMode(ControlModeEnum controlMode)
{
    VerifyOrReturnError(EnsureKnownEnumValue(controlMode) != ControlModeEnum::kUnknownEnumValue,
                        CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mEffectiveControlMode, controlMode, EffectiveControlMode::Id))
    {
        StoreAttribute(mContext, mPath, EffectiveControlMode::Id, mEffectiveControlMode);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetPumpStatus(PumpStatusBitMask pumpStatus)
{
    SetAttributeValue(mPumpStatus, pumpStatus, PumpStatus::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetCapacity(DataModel::Nullable<int16_t> capacity)
{
    VerifyOrReturnError(IsValidNullable(capacity), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mCapacity, capacity, Capacity::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetSpeed(DataModel::Nullable<uint16_t> speed)
{
    VerifyOrReturnError(IsValidNullable(speed), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mSpeed, speed, Speed::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetPower(DataModel::Nullable<uint32_t> power)
{
    VerifyOrReturnError(IsValidNullableInt24u(power), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    SetAttributeValue(mPower, power, Power::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetLifetimeRunningHours(DataModel::Nullable<uint32_t> hours)
{
    VerifyOrReturnError(IsValidNullableInt24u(hours), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mLifetimeRunningHours, hours, LifetimeRunningHours::Id))
    {
        StoreAttribute(mContext, mPath, LifetimeRunningHours::Id, mLifetimeRunningHours);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetLifetimeEnergyConsumed(DataModel::Nullable<uint32_t> energy)
{
    VerifyOrReturnError(IsValidNullable(energy), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    if (SetAttributeValue(mLifetimeEnergyConsumed, energy, LifetimeEnergyConsumed::Id))
    {
        StoreAttribute(mContext, mPath, LifetimeEnergyConsumed::Id, mLifetimeEnergyConsumed);
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

void PumpConfigurationAndControlCluster::UpdateEffectiveModes()
{
    const EndpointId endpointId       = mPath.mEndpointId;
    const RemoteSensorType sensorType = (mOperationMode == OperationModeEnum::kNormal && mDelegate != nullptr)
        ? mDelegate->GetRemoteSensorType(endpointId)
        : RemoteSensorType::kNoSensor;

    if (SetAttributeValue(mEffectiveOperationMode, mOperationMode, EffectiveOperationMode::Id))
    {
        StoreAttribute(mContext, mPath, EffectiveOperationMode::Id, mEffectiveOperationMode);
    }
    LogErrorOnFailure(SetEffectiveControlMode(ComputeEffectiveControlMode(mOperationMode, mControlMode, sensorType)));

    if (mEnabledOptionalAttributes.IsSet(PumpStatus::Id))
    {
        PumpStatusBitMask pumpStatus = mPumpStatus;
        pumpStatus.Clear(PumpStatusBitmap::kRemotePressure)
            .Clear(PumpStatusBitmap::kRemoteFlow)
            .Clear(PumpStatusBitmap::kRemoteTemperature)
            .Set(RemoteSensorStatusBits(sensorType));
        SetAttributeValue(mPumpStatus, pumpStatus, PumpStatus::Id);
    }

    if (mDelegate != nullptr && (mOperationMode == OperationModeEnum::kMinimum || mOperationMode == OperationModeEnum::kMaximum))
    {
        mDelegate->OnOperationModeMinMax(endpointId, mOperationMode);
    }
}

} // namespace chip::app::Clusters
