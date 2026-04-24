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

static constexpr chip::Percent100ths WC_PERCENT100THS_MIN_OPEN   = 0;
static constexpr chip::Percent100ths WC_PERCENT100THS_MAX_CLOSED = 10000;
static constexpr chip::Percent100ths WC_PERCENT100THS_MIDDLE     = 5000;
static constexpr chip::Percent100ths WC_PERCENT100THS_COEF       = 100;

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

typedef DataModel::Nullable<Percent> NPercent;
typedef DataModel::Nullable<Percent100ths> NPercent100ths;
typedef DataModel::Nullable<uint16_t> NAbsolute;
typedef Optional<Percent> OPercent;
typedef Optional<Percent100ths> OPercent100ths;

using OptionalAttributeSet = app::OptionalAttributeSet<Attributes::SafetyStatus::Id>;

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

struct Config
{
    Type type;
    chip::BitMask<ConfigStatus> configStatus;
    chip::BitMask<OperationalStatus> operationalStatus;
    EndProductType endProductType;
    chip::BitMask<Mode> mode;
};

class WindowCoveringCluster : public DefaultServerCluster
{
public:
    WindowCoveringCluster(EndpointId endpointId, BitFlags<WindowCovering::Feature> features,
                          OptionalAttributeSet & optionalAttributeSet, Config & config);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    //    Commands
    /**
     * @brief Handles the UpOrOpen command.
     *        Sets the target lift/tilt positions to fully open and requests movement from the delegate.
     *
     * @return Status::Success on successful handling.
     *         Status::Busy if the device is in maintenance mode.
     *         Status::Failure if the device is in calibration mode or otherwise not operational.
     */
    Protocols::InteractionModel::Status HandleUpOrOpen();

    /**
     * @brief Handles the DownOrClose command.
     *        Sets the target lift/tilt positions to fully closed and requests movement from the delegate.
     *
     * @return Status::Success on successful handling.
     *         Status::Busy if the device is in maintenance mode.
     *         Status::Failure if the device is in calibration mode or otherwise not operational.
     */
    Protocols::InteractionModel::Status HandleDownOrClose();

    /**
     * @brief Handles the StopMotion command.
     *        Asks the delegate to stop any ongoing motion and (unless the delegate reports motion is still in progress)
     *        latches the current position as the new target.
     */
    Protocols::InteractionModel::Status HandleStopMotion(const Commands::StopMotion::DecodableType & fields);

    /**
     * @brief Handles the GoToLiftValue command.
     *        Only supported when both AbsolutePosition and PositionAwareLift features are present.
     */
    Protocols::InteractionModel::Status HandleGoToLiftValue(const Commands::GoToLiftValue::DecodableType & commandData);

    /**
     * @brief Handles the GoToTiltValue command.
     *        Only supported when both AbsolutePosition and PositionAwareTilt features are present.
     */
    Protocols::InteractionModel::Status HandleGoToTiltValue(const Commands::GoToTiltValue::DecodableType & commandData);

    /**
     * @brief Handles the GoToLiftPercentage command.
     *        Validates the percent100ths value and requests lift movement from the delegate.
     *
     * @return Status::ConstraintError if the percentage is out of range.
     *         Status::Failure if the PositionAwareLift feature is not supported.
     */
    Protocols::InteractionModel::Status HandleGoToLiftPercentage(const Commands::GoToLiftPercentage::DecodableType & fields);

    /**
     * @brief Handles the GoToTiltPercentage command.
     *        Validates the percent100ths value and requests tilt movement from the delegate.
     *
     * @return Status::ConstraintError if the percentage is out of range.
     *         Status::Failure if the PositionAwareTilt feature is not supported.
     */
    Protocols::InteractionModel::Status HandleGoToTiltPercentage(const Commands::GoToTiltPercentage::DecodableType & fields);
    // Setters and Getters
    bool HasFeature(Feature feature) const { return mFeatureMap.Has(feature); }
    bool HasFeaturePaLift() const;
    bool HasFeaturePaTilt() const;
    BitFlags<Feature> GetFeatureMap() const { return mFeatureMap; }

    void SetType(Type type) { mType = type; }
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
    NPercent100ths GetTargetPositionLiftPercent100ths() const { return mTargetPositionLiftPercentage100ths; }

    void SetTargetPositionTiltPercent100ths(NPercent100ths newTargetLift);
    NPercent100ths GetTargetPositionTiltPercent100ths() const { return mTargetPositionTiltPercentage100ths; }

    void SetEndProductType(EndProductType type);
    EndProductType GetEndProductType() const { return mEndProductType; }

    void SetCurrentPositionLiftPercentage100ths(NPercent100ths curLiftPercentage100ths);
    NPercent100ths GetCurrentPositionLiftPercentage100ths() const { return mCurrentPositionLiftPercentage100ths; }

    void SetCurrentPositionTiltPercentage100ths(NPercent100ths curTiltPercentage100ths);
    NPercent100ths GetCurrentPositionTiltPercentage100ths() const { return mCurrentPositionTiltPercentage100ths; }

    void SetMode(chip::BitMask<Mode> mode);
    chip::BitMask<Mode> GetMode() const { return mMode; }

    void SetSafetyStatus(chip::BitMask<SafetyStatus> status);
    chip::BitMask<SafetyStatus> GetSafetyStatus() const { return mSafetyStatus; }

    void SetDelegate(Delegate * delegate) { mDelegate = delegate; }
    Delegate * GetDelegate() const { return mDelegate; }

private:
    /**
     * @brief Updates ConfigStatus to reflect the current Mode.
     *        Called after mMode is committed.
     */
    void UpdateConfigStatusFromMode();

    /**
     * @brief Computes the motion lock status from the current Mode and ConfigStatus members.
     *
     * @return Status::Success if motion is allowed.
     *         Status::Busy if maintenance mode is active.
     *         Status::Failure if calibration mode is active (or otherwise not operational).
     */
    Protocols::InteractionModel::Status GetMotionLockStatus() const;
    void UpdateOperationalStateForField(chip::BitMask<OperationalStatus> field, OperationalState state);

    EndpointId GetEndpointId() { return mPath.mEndpointId; }

    Delegate * mDelegate = nullptr;
    BitFlags<WindowCovering::Feature> mFeatureMap;
    OptionalAttributeSet mOptionalAttributes;

    // Attributes
    Type mType;
    uint16_t mNumberOfActuationsLift = 0;
    uint16_t mNumberOfActuationsTilt = 0;
    chip::BitMask<ConfigStatus> mConfigStatus;
    NPercent mCurrentPositionLiftPercentage;
    NPercent mCurrentPositionTiltPercentage;
    NPercent100ths mTargetPositionLiftPercentage100ths;
    NPercent100ths mTargetPositionTiltPercentage100ths;
    chip::BitMask<OperationalStatus> mOperationalStatus;
    EndProductType mEndProductType;
    NPercent100ths mCurrentPositionLiftPercentage100ths;
    NPercent100ths mCurrentPositionTiltPercentage100ths;
    chip::BitMask<Mode> mMode;
    chip::BitMask<SafetyStatus> mSafetyStatus;
};

// These functions are only kept for backwards compatibility with example apps and should not be used by new code.
// New code should interact with the cluster directly through the WindowCoveringCluster API.
void TypeSet(chip::EndpointId endpoint, Type type);
Type TypeGet(chip::EndpointId endpoint);

void ConfigStatusPrint(const chip::BitMask<ConfigStatus> & configStatus);
void ConfigStatusSet(chip::EndpointId endpoint, const chip::BitMask<ConfigStatus> & status);
chip::BitMask<ConfigStatus> ConfigStatusGet(chip::EndpointId endpoint);
void ConfigStatusUpdateFeatures(chip::EndpointId endpoint);

void OperationalStatusPrint(const chip::BitMask<OperationalStatus> & opStatus);
void OperationalStatusSet(chip::EndpointId endpoint, chip::BitMask<OperationalStatus> newStatus);
chip::BitMask<OperationalStatus> OperationalStatusGet(chip::EndpointId endpoint);
void OperationalStateSet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field, OperationalState state);
OperationalState OperationalStateGet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field);

OperationalState ComputeOperationalState(uint16_t target, uint16_t current);
OperationalState ComputeOperationalState(NPercent100ths target, NPercent100ths current);
Percent100ths ComputePercent100thsStep(OperationalState direction, Percent100ths previous, Percent100ths delta);

void EndProductTypeSet(chip::EndpointId endpoint, EndProductType type);
EndProductType EndProductTypeGet(chip::EndpointId endpoint);

void ModePrint(const chip::BitMask<Mode> & mode);
void ModeSet(chip::EndpointId endpoint, chip::BitMask<Mode> & mode);
chip::BitMask<Mode> ModeGet(chip::EndpointId endpoint);

void SafetyStatusSet(chip::EndpointId endpoint, const chip::BitMask<SafetyStatus> & status);
chip::BitMask<SafetyStatus> SafetyStatusGet(chip::EndpointId endpoint);

LimitStatus CheckLimitState(uint16_t position, AbsoluteLimits limits);
bool IsPercent100thsValid(Percent100ths percent100ths);
bool IsPercent100thsValid(NPercent100ths npercent100ths);

uint16_t Percent100thsToValue(AbsoluteLimits limits, Percent100ths relative);

uint16_t LiftToPercent100ths(chip::EndpointId endpoint, uint16_t lift);
uint16_t Percent100thsToLift(chip::EndpointId endpoint, uint16_t percent100ths);
void LiftPositionSet(chip::EndpointId endpoint, NPercent100ths position);

uint16_t TiltToPercent100ths(chip::EndpointId endpoint, uint16_t tilt);
uint16_t Percent100thsToTilt(chip::EndpointId endpoint, uint16_t percent100ths);
void TiltPositionSet(chip::EndpointId endpoint, NPercent100ths position);

/**
 * @brief PostAttributeChange is called when an Attribute is modified.
 *
 * The method is called by MatterWindowCoveringClusterServerAttributeChangedCallback
 * to update cluster attributes values. If the application overrides MatterWindowCoveringClusterServerAttributeChangedCallback,
 * it should call the PostAttributeChange on its own.
 *
 * @param[in] endpoint
 * @param[in] attributeId
 */
void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId);

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
