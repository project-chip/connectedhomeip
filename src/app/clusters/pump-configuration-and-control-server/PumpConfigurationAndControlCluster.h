/*
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
#pragma once

#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/clusters/pump-configuration-and-control-server/PumpConfigurationAndControlDelegate.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/PumpConfigurationAndControl/AttributeIds.h>
#include <clusters/PumpConfigurationAndControl/ClusterId.h>
#include <clusters/PumpConfigurationAndControl/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitMask.h>
#include <lib/support/CodeUtils.h>

#include <cstdint>
#include <optional>

namespace chip::app::Clusters {

class PumpConfigurationAndControlCluster : public DefaultServerCluster
{
public:
    /// Optional attributes that can be enabled for a cluster instance.
    ///
    /// A bitmask is used instead of `OptionalAttributeSet` because the ControlMode attribute id
    /// (0x21) exceeds the 32-bit mask range supported by `OptionalAttributeSet`.
    ///
    /// The Min/Max limit attributes have "<Feature>, [AUTO]" conformance: they are always present
    /// when their own feature is enabled, and present with the Automatic feature only when the
    /// corresponding bit is set.
    enum class OptionalAttributes : uint32_t
    {
        kPumpStatus             = 0x0001,
        kSpeed                  = 0x0002,
        kLifetimeRunningHours   = 0x0004,
        kPower                  = 0x0008,
        kLifetimeEnergyConsumed = 0x0010,
        kControlMode            = 0x0020,
        kMinConstPressure       = 0x0040,
        kMaxConstPressure       = 0x0080,
        kMinCompPressure        = 0x0100,
        kMaxCompPressure        = 0x0200,
        kMinConstSpeed          = 0x0400,
        kMaxConstSpeed          = 0x0800,
        kMinConstFlow           = 0x1000,
        kMaxConstFlow           = 0x2000,
        kMinConstTemp           = 0x4000,
        kMaxConstTemp           = 0x8000,
    };

    using PumpStatusBitMask = BitMask<PumpConfigurationAndControl::PumpStatusBitmap>;

    /// Values of the attributes with the Fixed quality. They describe the pump hardware and
    /// do not change at runtime. Out-of-range values (such as reserved null sentinels,
    /// temperatures below -27315, or MaxConstTemp < MinConstTemp) are replaced with null.
    struct FixedAttributes
    {
        DataModel::Nullable<int16_t> maxPressure;
        DataModel::Nullable<uint16_t> maxSpeed;
        DataModel::Nullable<uint16_t> maxFlow;
        DataModel::Nullable<int16_t> minConstPressure;
        DataModel::Nullable<int16_t> maxConstPressure;
        DataModel::Nullable<int16_t> minCompPressure;
        DataModel::Nullable<int16_t> maxCompPressure;
        DataModel::Nullable<uint16_t> minConstSpeed;
        DataModel::Nullable<uint16_t> maxConstSpeed;
        DataModel::Nullable<uint16_t> minConstFlow;
        DataModel::Nullable<uint16_t> maxConstFlow;
        DataModel::Nullable<int16_t> minConstTemp;
        DataModel::Nullable<int16_t> maxConstTemp;
    };

    struct Config
    {
        explicit Config(PumpConfigurationAndControl::Delegate & delegateIn) : delegate(delegateIn) {}

        /// Must outlive the cluster.
        PumpConfigurationAndControl::Delegate & delegate;

        BitFlags<PumpConfigurationAndControl::Feature> features;
        BitMask<OptionalAttributes> optionalAttributes;
        FixedAttributes fixedAttributes;

        /// OperationMode value used when no value is persisted.
        PumpConfigurationAndControl::OperationModeEnum operationMode = PumpConfigurationAndControl::OperationModeEnum::kNormal;

        /// ControlMode value used when no value is persisted. When ControlMode is not enabled,
        /// this is the control mode used to compute EffectiveControlMode.
        ///
        /// If unset, or if the mode requires a feature that is not enabled, the first supported
        /// mode in ControlModeEnum order is used.
        std::optional<PumpConfigurationAndControl::ControlModeEnum> controlMode;
    };

    PumpConfigurationAndControlCluster(EndpointId endpointId, const Config & config);

    // ServerClusterInterface/DefaultServerCluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    BitFlags<PumpConfigurationAndControl::Feature> GetFeatures() const { return mFeatures; }

    /// Returns whether a cluster-specific `attributeId` is enabled (does not cover global attributes).
    bool IsAttributeEnabled(AttributeId attributeId) const;
    const FixedAttributes & GetFixedAttributes() const { return mFixedAttributes; }

    /// Sets OperationMode and updates EffectiveOperationMode, EffectiveControlMode and
    /// the remote sensor bits of PumpStatus.
    ///
    /// Returns ConstraintError if the mode requires a feature that is not enabled.
    CHIP_ERROR SetOperationMode(PumpConfigurationAndControl::OperationModeEnum operationMode);
    PumpConfigurationAndControl::OperationModeEnum GetOperationMode() const { return mOperationMode; }

    /// Sets ControlMode and updates the effective modes, same as SetOperationMode.
    ///
    /// Returns ConstraintError if the mode requires a feature that is not enabled.
    /// Returns UnsupportedAttribute if ControlMode is not enabled.
    CHIP_ERROR SetControlMode(PumpConfigurationAndControl::ControlModeEnum controlMode);
    PumpConfigurationAndControl::ControlModeEnum GetControlMode() const { return mControlMode; }

    /// Asks the delegate for the remote sensor again. If the active remote sensor is different from
    /// the one used for the last computation, updates EffectiveControlMode and the remote sensor bits
    /// of PumpStatus. Call it when the value returned by Delegate::GetRemoteSensorType changes.
    ///
    /// Has no effect unless OperationMode is Normal: a remote sensor only applies in Normal mode.
    /// Never changes EffectiveOperationMode.
    void OnRemoteSensorChanged();

    /// Overrides EffectiveControlMode. The application owns EffectiveControlMode while
    /// OperationMode is Local, and in Normal mode when no remote sensor is active and either
    /// ControlMode is Automatic or the ControlMode attribute is disabled. The next recompute (an
    /// OperationMode or ControlMode change, or a remote sensor change reported with
    /// OnRemoteSensorChanged) overwrites it.
    ///
    /// Returns ConstraintError if the mode requires a feature that is not enabled.
    /// Returns InvalidInState if the application does not own EffectiveControlMode.
    CHIP_ERROR SetEffectiveControlMode(PumpConfigurationAndControl::ControlModeEnum controlMode);
    PumpConfigurationAndControl::ControlModeEnum GetEffectiveControlMode() const { return mEffectiveControlMode; }

    /// Overrides EffectiveOperationMode, e.g. to Local while the pump is physically set to run with
    /// its local settings. The next OperationMode change sets it back to OperationMode. The value is
    /// not persisted: Startup sets it to OperationMode.
    ///
    /// Returns ConstraintError if the mode requires a feature that is not enabled.
    CHIP_ERROR SetEffectiveOperationMode(PumpConfigurationAndControl::OperationModeEnum operationMode);
    PumpConfigurationAndControl::OperationModeEnum GetEffectiveOperationMode() const { return mEffectiveOperationMode; }

    /// Sets the application-controlled bits of PumpStatus. The remote sensor bits
    /// (RemotePressure, RemoteFlow, RemoteTemperature) are owned by the cluster and ignored in
    /// `pumpStatus`.
    ///
    /// Returns ConstraintError if a bit outside of PumpStatusBitmap is set.
    CHIP_ERROR SetPumpStatus(PumpStatusBitMask pumpStatus);
    PumpStatusBitMask GetPumpStatus() const { return mPumpStatus; }

    CHIP_ERROR SetCapacity(DataModel::Nullable<int16_t> capacity);
    DataModel::Nullable<int16_t> GetCapacity() const { return mCapacity.value(); }

    CHIP_ERROR SetSpeed(DataModel::Nullable<uint16_t> speed);
    DataModel::Nullable<uint16_t> GetSpeed() const { return mSpeed.value(); }

    /// Power is an int24u: non-null values above 0xFFFFFE are rejected.
    CHIP_ERROR SetPower(DataModel::Nullable<uint32_t> power);
    DataModel::Nullable<uint32_t> GetPower() const { return mPower.value(); }

    /// LifetimeRunningHours is an int24u: non-null values above 0xFFFFFE are rejected.
    CHIP_ERROR SetLifetimeRunningHours(DataModel::Nullable<uint32_t> hours);
    DataModel::Nullable<uint32_t> GetLifetimeRunningHours() const { return mLifetimeRunningHours; }

    CHIP_ERROR SetLifetimeEnergyConsumed(DataModel::Nullable<uint32_t> energy);
    DataModel::Nullable<uint32_t> GetLifetimeEnergyConsumed() const { return mLifetimeEnergyConsumed; }

    /// Generates one of the cluster alarm events, e.g.
    /// `GenerateAlarmEvent(PumpConfigurationAndControl::Events::GeneralFault::Type{})`.
    ///
    /// Returns the event number, or std::nullopt if the cluster is not started or the event
    /// could not be logged.
    template <typename EventType>
    std::optional<EventNumber> GenerateAlarmEvent(const EventType & event)
    {
        static_assert(EventType::GetClusterId() == PumpConfigurationAndControl::Id,
                      "Only Pump Configuration and Control events can be generated");
        VerifyOrReturnValue(mContext != nullptr, std::nullopt);
        return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

private:
    /// Enabled attributes with ids below 32. ControlMode is tracked by mControlModeEnabled.
    using EnabledAttributeSet = app::OptionalAttributeSet<                  //
        PumpConfigurationAndControl::Attributes::MinConstPressure::Id,      //
        PumpConfigurationAndControl::Attributes::MaxConstPressure::Id,      //
        PumpConfigurationAndControl::Attributes::MinCompPressure::Id,       //
        PumpConfigurationAndControl::Attributes::MaxCompPressure::Id,       //
        PumpConfigurationAndControl::Attributes::MinConstSpeed::Id,         //
        PumpConfigurationAndControl::Attributes::MaxConstSpeed::Id,         //
        PumpConfigurationAndControl::Attributes::MinConstFlow::Id,          //
        PumpConfigurationAndControl::Attributes::MaxConstFlow::Id,          //
        PumpConfigurationAndControl::Attributes::MinConstTemp::Id,          //
        PumpConfigurationAndControl::Attributes::MaxConstTemp::Id,          //
        PumpConfigurationAndControl::Attributes::PumpStatus::Id,            //
        PumpConfigurationAndControl::Attributes::Speed::Id,                 //
        PumpConfigurationAndControl::Attributes::LifetimeRunningHours::Id,  //
        PumpConfigurationAndControl::Attributes::Power::Id,                 //
        PumpConfigurationAndControl::Attributes::LifetimeEnergyConsumed::Id //
        >;

    static EnabledAttributeSet BuildEnabledAttributes(BitFlags<PumpConfigurationAndControl::Feature> features,
                                                      BitMask<OptionalAttributes> optionalAttributes);

    /// Assigns `value` to `dest` and, if the value changed, reports the change and calls
    /// Delegate::OnAttributeChanged. Returns true if the value changed.
    ///
    /// Only for setters where this is the last state change: the delegate may call back into
    /// the cluster.
    template <typename T, typename U>
    bool UpdateAttribute(T & dest, const U & value, AttributeId attributeId)
    {
        VerifyOrReturnValue(SetAttributeValue(dest, value, attributeId), false);
        mDelegate.OnAttributeChanged(mPath.mEndpointId, attributeId);
        return true;
    }

    template <typename T>
    bool UpdateQuietAttribute(QuieterReportingAttribute<T> & attr, const DataModel::Nullable<T> & value, AttributeId attributeId);

    bool IsOperationModeSupported(PumpConfigurationAndControl::OperationModeEnum operationMode) const;
    bool IsControlModeSupported(PumpConfigurationAndControl::ControlModeEnum controlMode) const;
    bool CanApplicationOwnEffectiveControlMode(PumpConfigurationAndControl::RemoteSensorType sensorType) const;
    PumpConfigurationAndControl::RemoteSensorType GetActiveRemoteSensor() const;
    PumpConfigurationAndControl::ControlModeEnum
    SelectInitialControlMode(std::optional<PumpConfigurationAndControl::ControlModeEnum> configured) const;

    /// Attributes changed by one mode update. The delegate is told about them only after all
    /// values of the update are written.
    class ChangedAttributes
    {
    public:
        void Add(AttributeId attributeId)
        {
            VerifyOrDie(mCount < kMaxCount);
            mAttributeIds[mCount++] = attributeId;
        }
        const AttributeId * begin() const { return mAttributeIds; }
        const AttributeId * end() const { return mAttributeIds + mCount; }

    private:
        // The mode attribute that triggered the update, EffectiveOperationMode,
        // EffectiveControlMode and PumpStatus.
        static constexpr size_t kMaxCount = 4;
        AttributeId mAttributeIds[kMaxCount];
        size_t mCount = 0;
    };

    /// Recomputes EffectiveControlMode and the remote sensor bits of PumpStatus for `sensorType`
    /// (the result of GetActiveRemoteSensor), then calls the delegate: OnOperationModeMinMax if
    /// applicable, then OnAttributeChanged for `changed` and for every attribute changed here.
    ///
    /// The delegate is called only after all values are written, and the state is not modified
    /// after the first delegate call. A delegate that calls back into the cluster (e.g.
    /// OnRemoteSensorChanged from OnAttributeChanged) therefore starts from a consistent state,
    /// and its update is not overwritten by this one.
    void UpdateEffectiveModes(ChangedAttributes & changed, PumpConfigurationAndControl::RemoteSensorType sensorType);

    /// Invalidates any persisted EffectiveControlMode override.
    void ClearStoredEffectiveControlModeOverride();

    const BitFlags<PumpConfigurationAndControl::Feature> mFeatures;
    const EnabledAttributeSet mEnabledOptionalAttributes;
    const bool mControlModeEnabled;
    const FixedAttributes mFixedAttributes;
    PumpConfigurationAndControl::Delegate & mDelegate;

    PumpConfigurationAndControl::OperationModeEnum mOperationMode;
    PumpConfigurationAndControl::ControlModeEnum mControlMode;
    PumpConfigurationAndControl::OperationModeEnum mEffectiveOperationMode;
    PumpConfigurationAndControl::ControlModeEnum mEffectiveControlMode;
    bool mHasStoredEffectiveControlModeOverride = false;
    /// Remote sensor used for the last EffectiveControlMode computation. kNoSensor unless
    /// OperationMode is Normal.
    PumpConfigurationAndControl::RemoteSensorType mActiveRemoteSensor = PumpConfigurationAndControl::RemoteSensorType::kNoSensor;
    PumpStatusBitMask mPumpStatus{ 0 };
    QuieterReportingAttribute<int16_t> mCapacity{ DataModel::NullNullable };
    QuieterReportingAttribute<uint16_t> mSpeed{ DataModel::NullNullable };
    QuieterReportingAttribute<uint32_t> mPower{ DataModel::NullNullable };
    DataModel::Nullable<uint32_t> mLifetimeRunningHours;
    DataModel::Nullable<uint32_t> mLifetimeEnergyConsumed;
};

} // namespace chip::app::Clusters
