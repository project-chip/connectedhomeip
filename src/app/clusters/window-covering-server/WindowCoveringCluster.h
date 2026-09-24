/*
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

#pragma once

#include "WindowCoveringDelegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/WindowCovering/Attributes.h>
#include <clusters/WindowCovering/Enums.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

static constexpr Percent100ths kWcPercent100thsMinOpen   = 0;
static constexpr Percent100ths kWcPercent100thsMaxClosed = 10000;
static constexpr Percent100ths kWcPercent100thsMiddle    = 5000;
static constexpr Percent100ths kWcPercent100thsCOEF      = 100;

typedef DataModel::Nullable<Percent> NPercent;
typedef DataModel::Nullable<Percent100ths> NPercent100ths;

using OptionalAttributeSet =
    app::OptionalAttributeSet<Attributes::NumberOfActuationsLift::Id, Attributes::NumberOfActuationsTilt::Id,
                              Attributes::CurrentPositionLiftPercentage::Id, Attributes::CurrentPositionTiltPercentage::Id,
                              Attributes::SafetyStatus::Id>;

// Match directly with OperationalStatus 2 bits Fields
enum class OperationalState : uint8_t
{
    Stall             = 0x00, // currently not moving
    MovingUpOrOpen    = 0x01, // is currently opening
    MovingDownOrClose = 0x02, // is currently closing
    Reserved          = 0x03, // dont use
};
static_assert(sizeof(OperationalState) == sizeof(uint8_t), "OperationalState Size is not correct");

class WindowCoveringCluster : public DefaultServerCluster
{
public:
    class Config
    {
    public:
        Config(WindowCoveringDelegate & delegate) : mDelegate(delegate) {}

        Config & WithFeatures(BitFlags<Feature> features)
        {
            mFeatures = features;
            return *this;
        }

        Config & WithOptionalAttributes(OptionalAttributeSet optionalAttributes)
        {
            mOptionalAttributes = optionalAttributes;
            return *this;
        }

        Config & WithType(Type type)
        {
            mType = type;
            return *this;
        }

        Config & WithEndProductType(EndProductType endProductType)
        {
            mEndProductType = endProductType;
            return *this;
        }

        WindowCoveringDelegate & mDelegate;
        BitFlags<Feature> mFeatures;
        OptionalAttributeSet mOptionalAttributes;
        Type mType                     = Type::kRollerShade;
        EndProductType mEndProductType = EndProductType::kRollerShade;
    };

    WindowCoveringCluster(EndpointId endpointId, const Config & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    /**
     * @brief Computes the motion lock status from the current Mode and ConfigStatus members.
     *
     * @return Status::Success if motion is allowed.
     *         Status::Busy if maintenance mode is active.
     *         Status::Failure if calibration mode is active (or otherwise not operational).
     */
    std::optional<DataModel::ActionReturnStatus> GetMotionLockStatus() const;

    // Setters and Getters
    BitFlags<Feature> GetFeatureMap() const { return mFeatureMap; }

    Type GetType() const { return mType; }

    void SetNumberOfActuationsLift(uint16_t numOfLifts);
    uint16_t GetNumberOfActuationsLift() const { return mNumberOfActuationsLift; }

    void SetNumberOfActuationsTilt(uint16_t numOfLifts);
    uint16_t GetNumberOfActuationsTilt() const { return mNumberOfActuationsTilt; }

    void SetConfigStatus(chip::BitMask<ConfigStatus> status);
    chip::BitMask<ConfigStatus> GetConfigStatus() const { return mConfigStatus; }

    void SetOperationalStatus(chip::BitMask<OperationalStatus> newStatus);
    chip::BitMask<OperationalStatus> GetOperationalStatus() const { return mOperationalStatus; }

    void SetTargetPositionLiftPercent100ths(NPercent100ths newTargetLift);
    NPercent100ths GetTargetPositionLiftPercent100ths() const { return mTargetPositionLiftPercent100ths; }

    void SetTargetPositionTiltPercent100ths(NPercent100ths newTargetLift);
    NPercent100ths GetTargetPositionTiltPercent100ths() const { return mTargetPositionTiltPercent100ths; }

    EndProductType GetEndProductType() const { return mEndProductType; }

    void SetCurrentPositionLiftPercent100ths(NPercent100ths curLiftPercent100ths);
    NPercent100ths GetCurrentPositionLiftPercent100ths() const { return mCurrentPositionLiftPercent100ths; }

    void SetCurrentPositionTiltPercent100ths(NPercent100ths curTiltPercent100ths);
    NPercent100ths GetCurrentPositionTiltPercent100ths() const { return mCurrentPositionTiltPercent100ths; }

    void SetMode(chip::BitMask<Mode> mode);
    chip::BitMask<Mode> GetMode() const { return mMode; }

    void SetSafetyStatus(chip::BitMask<SafetyStatus> status);
    chip::BitMask<SafetyStatus> GetSafetyStatus() const { return mSafetyStatus; }

protected:
    void SetType(Type type) { SetAttributeValue(mType, type, Attributes::Type::Id); }
    void SetEndProductType(EndProductType type);

private:
    // Percentage attributes are derived from Percent100ths, not stored separately.
    static NPercent PercentFromPercent100ths(NPercent100ths percent100ths);

    void UpdateOperationalStateForField(chip::BitMask<OperationalStatus> field, OperationalState state);

    std::optional<DataModel::ActionReturnStatus> HandleUpOrOpen();
    std::optional<DataModel::ActionReturnStatus> HandleDownOrClose();
    std::optional<DataModel::ActionReturnStatus> HandleStopMotion(const Commands::StopMotion::DecodableType & fields);
    std::optional<DataModel::ActionReturnStatus>
    HandleGoToLiftPercentage(const Commands::GoToLiftPercentage::DecodableType & fields);
    std::optional<DataModel::ActionReturnStatus>
    HandleGoToTiltPercentage(const Commands::GoToTiltPercentage::DecodableType & fields);

    WindowCoveringDelegate & mDelegate;
    const BitFlags<WindowCovering::Feature> mFeatureMap;
    const OptionalAttributeSet mOptionalAttributes;

    // Attributes
    Type mType;
    uint16_t mNumberOfActuationsLift = 0;
    uint16_t mNumberOfActuationsTilt = 0;
    chip::BitMask<ConfigStatus> mConfigStatus;
    NPercent100ths mTargetPositionLiftPercent100ths;
    NPercent100ths mTargetPositionTiltPercent100ths;
    chip::BitMask<OperationalStatus> mOperationalStatus;
    EndProductType mEndProductType;
    NPercent100ths mCurrentPositionLiftPercent100ths;
    NPercent100ths mCurrentPositionTiltPercent100ths;
    chip::BitMask<Mode> mMode;
    chip::BitMask<SafetyStatus> mSafetyStatus;
};

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
