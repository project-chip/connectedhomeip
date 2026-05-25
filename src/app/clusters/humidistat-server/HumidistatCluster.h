/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/Humidistat/Attributes.h>
#include <clusters/Humidistat/Enums.h>

namespace chip::app::Clusters {

/**
 * Delegate interface for Humidistat cluster state changes.
 * Applications implement this to react to attribute modifications.
 * All callbacks are notifications after the change has been committed;
 * they do not return values and cannot reject changes.
 */
class HumidistatDelegate
{
public:
    virtual ~HumidistatDelegate() = default;

    /**
     * Called after Mode attribute changes.
     * @param newMode The new mode value.
     */
    virtual void OnModeChanged(Humidistat::ModeEnum newMode) {}

    /**
     * Called after SystemState attribute changes.
     * @param newSystemState The new system state value.
     */
    virtual void OnSystemStateChanged(Humidistat::SystemStateEnum newSystemState) {}

    /**
     * Called after UserSetpoint attribute changes.
     * @param newUserSetpoint The new user setpoint value.
     */
    virtual void OnUserSetpointChanged(chip::Percent newUserSetpoint) {}

    /**
     * Called after TargetSetpoint attribute changes.
     * @param newTargetSetpoint The new target setpoint value.
     */
    virtual void OnTargetSetpointChanged(chip::Percent newTargetSetpoint) {}

    /**
     * Called after MistType attribute changes.
     * @param newMistType The new mist type value.
     */
    virtual void OnMistTypeChanged(chip::BitMask<Humidistat::MistTypeBitmap> newMistType) {}

    /**
     * Called after Continuous attribute changes.
     * @param newContinuous The new continuous value.
     */
    virtual void OnContinuousChanged(bool newContinuous) {}

    /**
     * Called after Sleep attribute changes.
     * @param newSleep The new sleep value.
     */
    virtual void OnSleepChanged(bool newSleep) {}

    /**
     * Called after Optimal attribute changes.
     * @param newOptimal The new optimal value.
     */
    virtual void OnOptimalChanged(bool newOptimal) {}
};

/**
 * Server-side implementation of the Humidistat cluster (cluster ID 0x0205).
 *
 * Combined implementation: the cluster owns attribute state, server interface
 * logic, and command handling.
 *
 */
class HumidistatCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet =
        chip::app::OptionalAttributeSet<Humidistat::Attributes::Sleep::Id, Humidistat::Attributes::TargetSetpoint::Id>;
    using FullOptionalAttributeSet = chip::app::OptionalAttributeSet<
        Humidistat::Attributes::UserSetpoint::Id, Humidistat::Attributes::MinSetpoint::Id, Humidistat::Attributes::MaxSetpoint::Id,
        Humidistat::Attributes::Step::Id, Humidistat::Attributes::TargetSetpoint::Id, Humidistat::Attributes::MistType::Id,
        Humidistat::Attributes::Continuous::Id, Humidistat::Attributes::Sleep::Id, Humidistat::Attributes::Optimal::Id>;

    struct StartupConfiguration
    {
        Humidistat::ModeEnum mode               = Humidistat::ModeEnum::kOff;
        Humidistat::SystemStateEnum systemState = Humidistat::SystemStateEnum::kOff;
        chip::Percent userSetpoint              = 50;
        chip::Percent minSetpoint               = 0;
        chip::Percent maxSetpoint               = 100;
        chip::Percent step                      = 1;
        chip::Percent targetSetpoint            = 50;
        chip::BitMask<Humidistat::MistTypeBitmap> mistType{ 0 };
        bool continuous = false;
        bool sleep      = false;
        bool optimal    = false;
    };

    /**
     * @param endpointId         The endpoint this cluster lives on.
     * @param features           Active feature flags from the FeatureMap.
     * @param optionalAttributes Bit-set of user-optional attributes to expose.
     * @param config             Initial values for cluster-owned attribute state.
     */
    HumidistatCluster(EndpointId endpointId, BitFlags<Humidistat::Feature> features,
                      const OptionalAttributeSet & optionalAttributes);

    HumidistatCluster(EndpointId endpointId, BitFlags<Humidistat::Feature> features,
                      const OptionalAttributeSet & optionalAttributes, const StartupConfiguration & config);

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    const BitFlags<Humidistat::Feature> & GetFeatures() const { return mFeatures; }

    Humidistat::ModeEnum GetMode() const { return mMode; }
    Humidistat::SystemStateEnum GetSystemState() const { return mSystemState; }
    chip::Percent GetUserSetpoint() const { return mUserSetpoint; }
    chip::Percent GetMinSetpoint() const { return mMinSetpoint; }
    chip::Percent GetMaxSetpoint() const { return mMaxSetpoint; }
    chip::Percent GetStep() const { return mStep; }
    chip::Percent GetTargetSetpoint() const { return mTargetSetpoint; }
    chip::BitMask<Humidistat::MistTypeBitmap> GetMistType() const { return mMistType; }
    bool GetContinuous() const { return mContinuous; }
    bool GetSleep() const { return mSleep; }
    bool GetOptimal() const { return mOptimal; }

    /**
     * Set the delegate to receive callbacks for attribute state changes.
     * @param delegate Pointer to delegate, or nullptr to clear.
     */
    void SetDelegate(HumidistatDelegate * delegate) { mDelegate = delegate; }

    CHIP_ERROR SetMode(Humidistat::ModeEnum mode);
    CHIP_ERROR SetSystemState(Humidistat::SystemStateEnum systemState);
    CHIP_ERROR SetUserSetpoint(chip::Percent userSetpoint);
    CHIP_ERROR SetTargetSetpoint(chip::Percent targetSetpoint);
    CHIP_ERROR SetMistType(chip::BitMask<Humidistat::MistTypeBitmap> mistType);
    CHIP_ERROR SetContinuous(bool continuous);
    CHIP_ERROR SetSleep(bool sleep);
    CHIP_ERROR SetOptimal(bool optimal);

private:
    const BitFlags<Humidistat::Feature> mFeatures;
    const OptionalAttributeSet mOptionalAttributes;
    const FullOptionalAttributeSet mActiveOptional;

    Humidistat::ModeEnum mMode;
    Humidistat::SystemStateEnum mSystemState;
    chip::Percent mUserSetpoint;
    const chip::Percent mMinSetpoint;
    const chip::Percent mMaxSetpoint;
    const chip::Percent mStep;
    chip::Percent mTargetSetpoint;
    chip::BitMask<Humidistat::MistTypeBitmap> mMistType;
    bool mContinuous;
    bool mSleep;
    bool mOptimal;
    HumidistatDelegate * mDelegate = nullptr;

    bool IsModeSupported(Humidistat::ModeEnum mode) const;
    bool IsSystemStateSupported(Humidistat::SystemStateEnum systemState) const;
    bool IsMistTypeConsistentWithMode(Humidistat::ModeEnum mode, chip::BitMask<Humidistat::MistTypeBitmap> mistType) const;
    bool ShouldTargetSetpointMatchUserSetpoint() const;
    void SyncTargetSetpointToUserSetpoint();
    bool IsMistTypeSupportable(chip::BitMask<Humidistat::MistTypeBitmap> mistType) const;
    chip::Percent SnapToNearestStep(chip::Percent value) const;
    FullOptionalAttributeSet ComputeActiveOptionalAttributes() const;
    void LoadPersistentAttributes();

    DataModel::ActionReturnStatus HandleSetSettings(chip::TLV::TLVReader & input_arguments);
};

} // namespace chip::app::Clusters
