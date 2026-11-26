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
#include <clusters/PumpConfigurationAndControl/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/odd-sized-integers.h>
#include <system/SystemClock.h>

#include <cstring>

namespace chip::app::Clusters {

using namespace PumpConfigurationAndControl;
using namespace PumpConfigurationAndControl::Attributes;

namespace {

/// Largest non-null value of a nullable int24u attribute (0xFFFFFF is the null value).
constexpr uint32_t kMaxNullableInt24u = 0xFFFFFE;

/// Minimum valid value for MinConstTemp and MaxConstTemp (-273.15 °C in 0.01 °C steps).
constexpr int16_t kMinConstTempValue = -27315;

using PumpStatusBitMask = PumpConfigurationAndControlCluster::PumpStatusBitMask;

/// Bits defined by PumpStatusBitmap.
constexpr PumpStatusBitMask kPumpStatusDefinedBits(PumpStatusBitmap::kDeviceFault, PumpStatusBitmap::kSupplyFault,
                                                   PumpStatusBitmap::kSpeedLow, PumpStatusBitmap::kSpeedHigh,
                                                   PumpStatusBitmap::kLocalOverride, PumpStatusBitmap::kRunning,
                                                   PumpStatusBitmap::kRemotePressure, PumpStatusBitmap::kRemoteFlow,
                                                   PumpStatusBitmap::kRemoteTemperature);

/// Remote sensor bits of PumpStatusBitmap, owned by the cluster.
constexpr PumpStatusBitMask kRemoteSensorStatusBits(PumpStatusBitmap::kRemotePressure, PumpStatusBitmap::kRemoteFlow,
                                                    PumpStatusBitmap::kRemoteTemperature);

/// Minimum time between two reportable changes of a quieter reporting attribute.
constexpr System::Clock::Milliseconds64 kQuietReportingInterval(1000);

bool IsValidNullableInt24u(const DataModel::Nullable<uint32_t> & value)
{
    return value.IsNull() || value.Value() <= kMaxNullableInt24u;
}

template <typename T>
bool IsValidNullable(const DataModel::Nullable<T> & value)
{
    return value.IsNull() || value.ExistingValueInEncodableRange();
}

/// Sets `value` to null if `isValid` is false. The spec requires invalid fixed values to read as null.
template <typename T>
void NullIfInvalid(DataModel::Nullable<T> & value, bool isValid, AttributeId attributeId)
{
    VerifyOrReturn(!isValid);
    ChipLogError(Zcl, "PCC: invalid value for fixed attribute 0x%" PRIx32 ", using null", attributeId);
    value.SetNull();
}

template <typename T>
void NullIfInvalid(DataModel::Nullable<T> & value, AttributeId attributeId)
{
    NullIfInvalid(value, IsValidNullable(value), attributeId);
}

PumpConfigurationAndControlCluster::FixedAttributes
SanitizeFixedAttributes(PumpConfigurationAndControlCluster::FixedAttributes fixed)
{
    NullIfInvalid(fixed.maxPressure, MaxPressure::Id);
    NullIfInvalid(fixed.maxSpeed, MaxSpeed::Id);
    NullIfInvalid(fixed.maxFlow, MaxFlow::Id);
    NullIfInvalid(fixed.minConstPressure, MinConstPressure::Id);
    NullIfInvalid(fixed.maxConstPressure, MaxConstPressure::Id);
    NullIfInvalid(fixed.minCompPressure, MinCompPressure::Id);
    NullIfInvalid(fixed.maxCompPressure, MaxCompPressure::Id);
    NullIfInvalid(fixed.minConstSpeed, MinConstSpeed::Id);
    NullIfInvalid(fixed.maxConstSpeed, MaxConstSpeed::Id);
    NullIfInvalid(fixed.minConstFlow, MinConstFlow::Id);
    NullIfInvalid(fixed.maxConstFlow, MaxConstFlow::Id);
    NullIfInvalid(fixed.minConstTemp, fixed.minConstTemp.IsNull() || fixed.minConstTemp.Value() >= kMinConstTempValue,
                  MinConstTemp::Id);
    // The spec also requires MaxConstTemp >= MinConstTemp.
    NullIfInvalid(fixed.maxConstTemp,
                  fixed.maxConstTemp.IsNull() ||
                      (fixed.maxConstTemp.Value() >= kMinConstTempValue &&
                       (fixed.minConstTemp.IsNull() || fixed.maxConstTemp.Value() >= fixed.minConstTemp.Value())),
                  MaxConstTemp::Id);
    return fixed;
}

bool IsMinimumOrMaximum(OperationModeEnum operationMode)
{
    return operationMode == OperationModeEnum::kMinimum || operationMode == OperationModeEnum::kMaximum;
}

/// `sensorType` is the active remote sensor, which is kNoSensor unless `operationMode` is Normal.
ControlModeEnum ComputeEffectiveControlMode(OperationModeEnum operationMode, ControlModeEnum controlMode,
                                            RemoteSensorType sensorType)
{
    if (IsMinimumOrMaximum(operationMode))
    {
        return ControlModeEnum::kConstantSpeed;
    }
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
}

/// Returns `status` with the remote sensor bits set for `sensorType`.
PumpStatusBitMask WithRemoteSensorBits(PumpStatusBitMask status, RemoteSensorType sensorType)
{
    status.Clear(kRemoteSensorStatusBits);
    switch (sensorType)
    {
    case RemoteSensorType::kFlowSensor:
        return status.Set(PumpStatusBitmap::kRemoteFlow);
    case RemoteSensorType::kPressureSensor:
        return status.Set(PumpStatusBitmap::kRemotePressure);
    case RemoteSensorType::kTemperatureSensor:
        return status.Set(PumpStatusBitmap::kRemoteTemperature);
    case RemoteSensorType::kNoSensor:
        break;
    }
    return status;
}

/// Loads LifetimeRunningHours in either the current 4-byte Nullable<uint32_t> format or the
/// earlier 3-byte int24u format (0xFFFFFF is null). Falls back to `defaultValue` if no valid
/// value is stored. Returns true when a 3-byte value was loaded and should be rewritten in the
/// current format.
bool LoadLifetimeRunningHours(AttributePersistenceProvider & provider, const ConcreteAttributePath & path,
                              DataModel::Nullable<uint32_t> & value, DataModel::Nullable<uint32_t> defaultValue)
{
    using CurrentTraits = NumericAttributeTraits<uint32_t>;
    using Int24uTraits  = NumericAttributeTraits<OddSizedInteger<3, false>>;

    uint8_t raw[sizeof(CurrentTraits::StorageType)];
    MutableByteSpan buffer(raw);
    const CHIP_ERROR err = provider.ReadValue(path, buffer);
    if (err != CHIP_NO_ERROR)
    {
        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogError(Zcl, "PCC: failed to load attribute 0x%" PRIx32 ": %" CHIP_ERROR_FORMAT, path.mAttributeId, err.Format());
        }
        value = defaultValue;
        return false;
    }

    if (buffer.size() == sizeof(CurrentTraits::StorageType))
    {
        CurrentTraits::StorageType storage;
        std::memcpy(&storage, raw, sizeof(storage));
        DataModel::StorageToNullable(storage, value);
        if (!IsValidNullableInt24u(value))
        {
            value = defaultValue;
        }
        return false;
    }

    if (buffer.size() == sizeof(Int24uTraits::StorageType))
    {
        Int24uTraits::StorageType storage;
        std::memcpy(&storage, raw, sizeof(storage));
        if (Int24uTraits::IsNullValue(storage))
        {
            value.SetNull();
        }
        else
        {
            value.SetNonNull(Int24uTraits::StorageToWorking(storage));
        }
        return true;
    }

    value = defaultValue;
    return false;
}

} // namespace

template <typename T>
void PumpConfigurationAndControlCluster::StoreAttribute(AttributeId attributeId, const T & value)
{
    VerifyOrReturn(mContext != nullptr);
    AttributePersistence persistence(mContext->attributeStorage);
    CHIP_ERROR err = persistence.StoreNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, attributeId }, value);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "PCC: failed to store attribute 0x%" PRIx32 ": %" CHIP_ERROR_FORMAT, attributeId, err.Format());
    }
}

template <typename T>
void PumpConfigurationAndControlCluster::UpdatePersistedAttribute(T & dest, const T & value, AttributeId attributeId)
{
    VerifyOrReturn(SetAttributeValue(dest, value, attributeId));
    StoreAttribute(attributeId, dest);
    mDelegate.OnAttributeChanged(mPath.mEndpointId, attributeId);
}

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
    mControlModeEnabled(config.optionalAttributes.Has(OptionalAttributes::kControlMode)),
    mFixedAttributes(SanitizeFixedAttributes(config.fixedAttributes)), mDelegate(config.delegate),
    mOperationMode(IsOperationModeSupported(config.operationMode) ? config.operationMode : OperationModeEnum::kNormal),
    mControlMode(SelectInitialControlMode(config.controlMode)), mEffectiveOperationMode(mOperationMode),
    mEffectiveControlMode(ComputeEffectiveControlMode(mOperationMode, mControlMode, RemoteSensorType::kNoSensor)),
    mLifetimeRunningHours(0), mLifetimeEnergyConsumed(0)
{}

CHIP_ERROR PumpConfigurationAndControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // The spec requires at least one of these features (conformance O.a+).
    if (!mFeatures.HasAny(Feature::kConstantPressure, Feature::kCompensatedPressure, Feature::kConstantFlow,
                          Feature::kConstantSpeed, Feature::kConstantTemperature))
    {
        ChipLogError(Zcl, "PCC: FeatureMap 0x%" PRIx32 " on endpoint %u has no control mode feature", mFeatures.Raw(),
                     mPath.mEndpointId);
    }

    AttributePersistence persistence(context.attributeStorage);
    const EndpointId endpointId = mPath.mEndpointId;
    const ClusterId clusterId   = mPath.mClusterId;

    const OperationModeEnum defaultOperationMode = mOperationMode;
    persistence.LoadNativeEndianValue({ endpointId, clusterId, OperationMode::Id }, mOperationMode, defaultOperationMode);
    if (!IsOperationModeSupported(mOperationMode))
    {
        mOperationMode = defaultOperationMode;
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

    // The effective modes are derived from the restored modes and the remote sensor, so they are
    // not persisted. The delegate is not notified: it may not be ready yet, and Level Control may
    // not be initialized.
    mActiveRemoteSensor     = GetActiveRemoteSensor();
    mEffectiveOperationMode = IsLocalOverrideActive() ? OperationModeEnum::kLocal : mOperationMode;
    mEffectiveControlMode   = ComputeEffectiveControlMode(mOperationMode, mControlMode, mActiveRemoteSensor);
    if (mEnabledOptionalAttributes.IsSet(PumpStatus::Id))
    {
        mPumpStatus = WithRemoteSensorBits(mPumpStatus, mActiveRemoteSensor);
    }

    if (mEnabledOptionalAttributes.IsSet(LifetimeRunningHours::Id))
    {
        const ConcreteAttributePath path(endpointId, clusterId, LifetimeRunningHours::Id);
        if (LoadLifetimeRunningHours(context.attributeStorage, path, mLifetimeRunningHours, mLifetimeRunningHours))
        {
            // Store the value in the current format, so that later loads do not need the conversion.
            StoreAttribute(LifetimeRunningHours::Id, mLifetimeRunningHours);
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
        return encoder.Encode(mCapacity.value());
    case Speed::Id:
        return encoder.Encode(mSpeed.value());
    case LifetimeRunningHours::Id:
        return encoder.Encode(mLifetimeRunningHours);
    case Power::Id:
        return encoder.Encode(mPower.value());
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
    static constexpr DataModel::AttributeEntry kOptionalAttributes[] = {
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
    ReturnErrorOnFailure(listBuilder.Append(Span(kMandatoryMetadata), Span(kOptionalAttributes), mEnabledOptionalAttributes));

    if (mControlModeEnabled)
    {
        return builder.AppendElements({ ControlMode::kMetadataEntry });
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetOperationMode(OperationModeEnum operationMode)
{
    VerifyOrReturnError(IsOperationModeSupported(operationMode), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(operationMode != mOperationMode, CHIP_NO_ERROR);
    // The spec requires a FAILURE status for OperationMode changes while LocalOverride is set.
    VerifyOrReturnError(!IsLocalOverrideActive(), CHIP_IM_GLOBAL_STATUS(Failure));
    SetAttributeValue(mOperationMode, operationMode, OperationMode::Id);
    StoreAttribute(OperationMode::Id, mOperationMode);
    UpdateEffectiveModes(OperationMode::Id, GetActiveRemoteSensor());
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetControlMode(ControlModeEnum controlMode)
{
    VerifyOrReturnError(mControlModeEnabled, CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsControlModeSupported(controlMode), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(SetAttributeValue(mControlMode, controlMode, ControlMode::Id), CHIP_NO_ERROR);
    StoreAttribute(ControlMode::Id, mControlMode);
    UpdateEffectiveModes(ControlMode::Id, GetActiveRemoteSensor());
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetEffectiveControlMode(ControlModeEnum controlMode)
{
    VerifyOrReturnError(IsControlModeSupported(controlMode), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(CanApplicationOwnEffectiveControlMode(mActiveRemoteSensor), CHIP_IM_GLOBAL_STATUS(InvalidInState));
    UpdateAttribute(mEffectiveControlMode, controlMode, EffectiveControlMode::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetEffectiveOperationMode(OperationModeEnum operationMode)
{
    VerifyOrReturnError(IsOperationModeSupported(operationMode), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(!IsLocalOverrideActive() || operationMode == OperationModeEnum::kLocal,
                        CHIP_IM_GLOBAL_STATUS(InvalidInState));
    UpdateAttribute(mEffectiveOperationMode, operationMode, EffectiveOperationMode::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetPumpStatus(PumpStatusBitMask pumpStatus)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(PumpStatus::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError((pumpStatus.Raw() & ~kPumpStatusDefinedBits.Raw()) == 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    // EffectiveOperationMode becomes Local while LocalOverride is set, and the Local operation mode
    // requires the LocalOperation feature.
    if (pumpStatus.Has(PumpStatusBitmap::kLocalOverride) && !mFeatures.Has(Feature::kLocalOperation))
    {
        ChipLogError(Zcl, "PCC: LocalOverride requires the LocalOperation feature on endpoint %u, ignoring it", mPath.mEndpointId);
        pumpStatus.Clear(PumpStatusBitmap::kLocalOverride);
    }

    const bool localOverrideChanged = pumpStatus.Has(PumpStatusBitmap::kLocalOverride) != IsLocalOverrideActive();
    VerifyOrReturnError(SetAttributeValue(mPumpStatus, WithRemoteSensorBits(pumpStatus, mActiveRemoteSensor), PumpStatus::Id),
                        CHIP_NO_ERROR);
    const bool effectiveOperationModeChanged = localOverrideChanged &&
        SetAttributeValue(mEffectiveOperationMode, IsLocalOverrideActive() ? OperationModeEnum::kLocal : mOperationMode,
                          EffectiveOperationMode::Id);

    // All values are written: delegate calls below may call back into the cluster.
    const EndpointId endpointId = mPath.mEndpointId;
    if (effectiveOperationModeChanged && IsMinimumOrMaximum(mEffectiveOperationMode))
    {
        mDelegate.OnOperationModeMinMax(endpointId, mEffectiveOperationMode);
    }
    mDelegate.OnAttributeChanged(endpointId, PumpStatus::Id);
    if (effectiveOperationModeChanged)
    {
        mDelegate.OnAttributeChanged(endpointId, EffectiveOperationMode::Id);
    }
    return CHIP_NO_ERROR;
}

void PumpConfigurationAndControlCluster::NotifyQuietAttributeChanged(AttributeId attributeId, AttributeDirtyState dirtyState)
{
    NotifyAttributeChanged(attributeId,
                           dirtyState == AttributeDirtyState::kMustReport ? DataModel::AttributeChangeType::kReportable
                                                                          : DataModel::AttributeChangeType::kQuiet);
    mDelegate.OnAttributeChanged(mPath.mEndpointId, attributeId);
}

template <typename T>
void PumpConfigurationAndControlCluster::UpdateQuietAttribute(QuieterReportingAttribute<T> & attr,
                                                              const DataModel::Nullable<T> & value, AttributeId attributeId)
{
    VerifyOrReturn(attr.value() != value);
    const AttributeDirtyState dirtyState =
        attr.SetValue(value, System::SystemClock().GetMonotonicMilliseconds64(), [](const auto & candidate) {
            return (candidate.nowTimestamp - candidate.lastDirtyTimestamp) >= kQuietReportingInterval;
        });
    NotifyQuietAttributeChanged(attributeId, dirtyState);
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetCapacity(DataModel::Nullable<int16_t> capacity)
{
    VerifyOrReturnError(IsValidNullable(capacity), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdateQuietAttribute(mCapacity, capacity, Capacity::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetSpeed(DataModel::Nullable<uint16_t> speed)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(Speed::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullable(speed), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdateQuietAttribute(mSpeed, speed, Speed::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetPower(DataModel::Nullable<uint32_t> power)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(Power::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullableInt24u(power), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdateQuietAttribute(mPower, power, Power::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetLifetimeRunningHours(DataModel::Nullable<uint32_t> hours)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(LifetimeRunningHours::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullableInt24u(hours), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdatePersistedAttribute(mLifetimeRunningHours, hours, LifetimeRunningHours::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PumpConfigurationAndControlCluster::SetLifetimeEnergyConsumed(DataModel::Nullable<uint32_t> energy)
{
    VerifyOrReturnError(mEnabledOptionalAttributes.IsSet(LifetimeEnergyConsumed::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(IsValidNullable(energy), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    UpdatePersistedAttribute(mLifetimeEnergyConsumed, energy, LifetimeEnergyConsumed::Id);
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

bool PumpConfigurationAndControlCluster::CanApplicationOwnEffectiveControlMode(RemoteSensorType sensorType) const
{
    if (mOperationMode == OperationModeEnum::kLocal)
    {
        return true;
    }
    return mOperationMode == OperationModeEnum::kNormal && sensorType == RemoteSensorType::kNoSensor &&
        (mControlMode == ControlModeEnum::kAutomatic || !mControlModeEnabled);
}

RemoteSensorType PumpConfigurationAndControlCluster::GetActiveRemoteSensor() const
{
    VerifyOrReturnValue(mOperationMode == OperationModeEnum::kNormal, RemoteSensorType::kNoSensor);
    const RemoteSensorType sensorType = mDelegate.GetRemoteSensorType(mPath.mEndpointId);
    switch (sensorType)
    {
    case RemoteSensorType::kPressureSensor:
        return mFeatures.Has(Feature::kConstantPressure) ? sensorType : RemoteSensorType::kNoSensor;
    case RemoteSensorType::kFlowSensor:
        return mFeatures.Has(Feature::kConstantFlow) ? sensorType : RemoteSensorType::kNoSensor;
    case RemoteSensorType::kTemperatureSensor:
        return mFeatures.Has(Feature::kConstantTemperature) ? sensorType : RemoteSensorType::kNoSensor;
    default:
        return RemoteSensorType::kNoSensor;
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
    VerifyOrReturn(IsStarted());
    const RemoteSensorType sensorType = GetActiveRemoteSensor();
    VerifyOrReturn(sensorType != mActiveRemoteSensor);
    UpdateEffectiveModes(kInvalidAttributeId, sensorType);
}

void PumpConfigurationAndControlCluster::UpdateEffectiveModes(AttributeId trigger, RemoteSensorType sensorType)
{
    const EndpointId endpointId     = mPath.mEndpointId;
    const bool operationModeChanged = (trigger == OperationMode::Id);
    mActiveRemoteSensor             = sensorType;

    const bool effectiveOperationModeChanged =
        operationModeChanged && SetAttributeValue(mEffectiveOperationMode, mOperationMode, EffectiveOperationMode::Id);
    const bool effectiveControlModeChanged = SetAttributeValue(
        mEffectiveControlMode, ComputeEffectiveControlMode(mOperationMode, mControlMode, sensorType), EffectiveControlMode::Id);
    const bool pumpStatusChanged = mEnabledOptionalAttributes.IsSet(PumpStatus::Id) &&
        SetAttributeValue(mPumpStatus, WithRemoteSensorBits(mPumpStatus, sensorType), PumpStatus::Id);

    // All values are written: delegate calls below may call back into the cluster.
    if (operationModeChanged && IsMinimumOrMaximum(mOperationMode))
    {
        mDelegate.OnOperationModeMinMax(endpointId, mOperationMode);
    }
    if (trigger != kInvalidAttributeId)
    {
        mDelegate.OnAttributeChanged(endpointId, trigger);
    }
    if (effectiveOperationModeChanged)
    {
        mDelegate.OnAttributeChanged(endpointId, EffectiveOperationMode::Id);
    }
    if (effectiveControlModeChanged)
    {
        mDelegate.OnAttributeChanged(endpointId, EffectiveControlMode::Id);
    }
    if (pumpStatusChanged)
    {
        mDelegate.OnAttributeChanged(endpointId, PumpStatus::Id);
    }
}

} // namespace chip::app::Clusters
