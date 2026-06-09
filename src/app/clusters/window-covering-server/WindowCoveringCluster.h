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
typedef DataModel::Nullable<uint16_t> NAbsolute;
typedef Optional<Percent> OPercent;
typedef Optional<Percent100ths> OPercent100ths;

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

// Declare Position Limit Status
enum class LimitStatus : uint8_t
{
    Intermediate      = 0x00,
    IsUpOrOpen        = 0x01,
    IsDownOrClose     = 0x02,
    Inverted          = 0x03,
    IsPastUpOrOpen    = 0x04,
    IsPastDownOrClose = 0x05,
};
static_assert(sizeof(LimitStatus) == sizeof(uint8_t), "LimitStatus Size is not correct");

struct AbsoluteLimits
{
    uint16_t open;
    uint16_t closed;
};

class WindowCoveringCluster : public DefaultServerCluster
{
public:
    class Config
    {
    public:
        Config() = default;

        Config & WithLift()
        {
            mFeatures.Set(Feature::kLift);
            return *this;
        }
        Config & WithTilt()
        {
            mFeatures.Set(Feature::kTilt);
            return *this;
        }
        Config & WithPositionAwareLift()
        {
            mFeatures.Set(Feature::kLift);
            mFeatures.Set(Feature::kPositionAwareLift);
            return *this;
        }
        Config & WithPositionAwareTilt()
        {
            mFeatures.Set(Feature::kTilt);
            mFeatures.Set(Feature::kPositionAwareTilt);
            return *this;
        }

        Config & WithOptionalAttributes(OptionalAttributeSet optionalAttributes)
        {
            mOptionalAttributes = optionalAttributes;
            return *this;
        }

    private:
        friend class WindowCoveringCluster;
        BitFlags<Feature> mFeatures;
        OptionalAttributeSet mOptionalAttributes;
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

    // Setters and Getters
    BitFlags<Feature> GetFeatureMap() const { return mFeatureMap; }

    Type GetType() const { return mType; }

    void SetNumberOfActuationsLift(uint16_t numOfLifts);
    uint16_t GetNumberOfActuationsLift() const { return mNumberOfActuationsLift; }

    void SetNumberOfActuationsTilt(uint16_t numOfLifts);
    uint16_t GetNumberOfActuationsTilt() const { return mNumberOfActuationsTilt; }

    void SetConfigStatus(chip::BitMask<ConfigStatus> status);
    chip::BitMask<ConfigStatus> GetConfigStatus() const { return mConfigStatus; }

    void SetCurrentPositionLiftPercentage(NPercent curLiftPercentage);
    NPercent GetCurrentPositionLiftPercentage() const { return mCurrentPositionLiftPercentage; };

    void SetCurrentPositionTiltPercentage(NPercent curTiltPercentage);
    NPercent GetCurrentPositionTiltPercentage() const { return mCurrentPositionTiltPercentage; };

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

    void SetDelegate(WindowCoveringDelegate * delegate) { mDelegate = delegate; }
    WindowCoveringDelegate * GetDelegate() const { return mDelegate; }

protected:
    void SetType(Type type) { SetAttributeValue(mType, type, Attributes::Type::Id); }
    void SetEndProductType(EndProductType type);

private:
    /**
     * @brief Computes the motion lock status from the current Mode and ConfigStatus members.
     *
     * @return Status::Success if motion is allowed.
     *         Status::Busy if maintenance mode is active.
     *         Status::Failure if calibration mode is active (or otherwise not operational).
     */
    std::optional<DataModel::ActionReturnStatus> GetMotionLockStatus() const;
    void UpdateOperationalStateForField(chip::BitMask<OperationalStatus> field, OperationalState state);

    std::optional<DataModel::ActionReturnStatus> HandleUpOrOpen();
    std::optional<DataModel::ActionReturnStatus> HandleDownOrClose();
    std::optional<DataModel::ActionReturnStatus> HandleStopMotion(const Commands::StopMotion::DecodableType & fields);
    std::optional<DataModel::ActionReturnStatus> HandleGoToLiftValue(const Commands::GoToLiftValue::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus> HandleGoToTiltValue(const Commands::GoToTiltValue::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleGoToLiftPercentage(const Commands::GoToLiftPercentage::DecodableType & fields);
    std::optional<DataModel::ActionReturnStatus>
    HandleGoToTiltPercentage(const Commands::GoToTiltPercentage::DecodableType & fields);

    EndpointId GetEndpointId() { return mPath.mEndpointId; }

    WindowCoveringDelegate * mDelegate = nullptr;
    const BitFlags<WindowCovering::Feature> mFeatureMap;
    const OptionalAttributeSet mOptionalAttributes;

    // Attributes
    Type mType;
    uint16_t mNumberOfActuationsLift = 0;
    uint16_t mNumberOfActuationsTilt = 0;
    chip::BitMask<ConfigStatus> mConfigStatus;
    NPercent mCurrentPositionLiftPercentage;
    NPercent mCurrentPositionTiltPercentage;
    NPercent100ths mTargetPositionLiftPercent100ths;
    NPercent100ths mTargetPositionTiltPercent100ths;
    chip::BitMask<OperationalStatus> mOperationalStatus;
    EndProductType mEndProductType;
    NPercent100ths mCurrentPositionLiftPercent100ths;
    NPercent100ths mCurrentPositionTiltPercent100ths;
    chip::BitMask<Mode> mMode;
    chip::BitMask<SafetyStatus> mSafetyStatus;
};

// Helper functions
LimitStatus CheckLimitState(uint16_t position, AbsoluteLimits limits);
bool IsPercent100thsValid(Percent100ths percent100ths);
bool IsPercent100thsValid(NPercent100ths npercent100ths);

OperationalState ComputeOperationalState(uint16_t target, uint16_t current);
OperationalState ComputeOperationalState(NPercent100ths target, NPercent100ths current);
Percent100ths ComputePercent100thsStep(OperationalState direction, Percent100ths previous, Percent100ths delta);

uint16_t Percent100thsToValue(AbsoluteLimits limits, Percent100ths relative);
Percent100ths ValueToPercent100ths(AbsoluteLimits limits, uint16_t absolute);

uint16_t LiftToPercent100ths(chip::EndpointId endpoint, uint16_t lift);
uint16_t Percent100thsToLift(chip::EndpointId endpoint, uint16_t percent100ths);
z

    uint16_t
    TiltToPercent100ths(chip::EndpointId endpoint, uint16_t tilt);
uint16_t Percent100thsToTilt(chip::EndpointId endpoint, uint16_t percent100ths);

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
