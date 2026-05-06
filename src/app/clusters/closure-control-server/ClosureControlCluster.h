/*
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ClosureControl/Attributes.h>
#include <clusters/ClosureControl/Commands.h>
#include <clusters/ClosureControl/Metadata.h>

#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/clusters/closure-control-server/ClosureControlClusterDelegate.h>
#include <app/clusters/closure-control-server/ClosureControlClusterObjects.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TimerDelegate.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

using OptionalAttributesSet = OptionalAttributeSet<ClosureControl::Attributes::CountdownTime::Id>;

// As per the spec, the maximum allowed CurrentErrorList size is 10.
constexpr int kCurrentErrorListMaxSize = 10;

class ClosureControlCluster : public DefaultServerCluster
{
public:
    struct Config
    {
        Config(EndpointId endpoint, ClosureControlClusterDelegate & delegate, TimerDelegate & timerDelegate) :
            mEndpointId(endpoint), mDelegate(delegate), mTimerDelegate(timerDelegate)
        {}

        Config & WithPositioning()
        {
            mFeatureMap.Set(Feature::kPositioning);
            return *this;
        }
        Config & WithMotionLatching(const BitFlags<LatchControlModesBitmap> & latchControlModes)
        {
            mFeatureMap.Set(Feature::kMotionLatching);
            mLatchControlModes = latchControlModes;
            return *this;
        }
        Config & WithInstantaneous()
        {
            mFeatureMap.Set(Feature::kInstantaneous);
            return *this;
        }
        Config & WithSpeed()
        {
            mFeatureMap.Set(Feature::kSpeed);
            return *this;
        }
        Config & WithVentilation()
        {
            mFeatureMap.Set(Feature::kVentilation);
            return *this;
        }
        Config & WithPedestrian()
        {
            mFeatureMap.Set(Feature::kPedestrian);
            return *this;
        }
        Config & WithCalibration()
        {
            mFeatureMap.Set(Feature::kCalibration);
            return *this;
        }
        Config & WithProtection()
        {
            mFeatureMap.Set(Feature::kProtection);
            return *this;
        }
        Config & WithManuallyOperable()
        {
            mFeatureMap.Set(Feature::kManuallyOperable);
            return *this;
        }
        Config & WithCountdownTime(DataModel::Nullable<ElapsedS> initial = DataModel::NullNullable)
        {
            mInitialCountdownTime = initial;
            mOptionalAttributes.Set<ClosureControl::Attributes::CountdownTime::Id>();
            return *this;
        }
        Config & WithInitialMainState(MainStateEnum mainState)
        {
            mInitialMainState = mainState;
            return *this;
        }
        Config & WithInitialOverallCurrentState(const DataModel::Nullable<GenericOverallCurrentState> & overallCurrentState)
        {
            mInitialOverallCurrentState = overallCurrentState;
            return *this;
        }

        EndpointId mEndpointId;
        ClosureControlClusterDelegate & mDelegate;
        TimerDelegate & mTimerDelegate;
        BitFlags<Feature> mFeatureMap;
        OptionalAttributesSet mOptionalAttributes;
        BitFlags<LatchControlModesBitmap> mLatchControlModes;
        DataModel::Nullable<ElapsedS> mInitialCountdownTime                         = DataModel::NullNullable;
        MainStateEnum mInitialMainState                                             = MainStateEnum::kStopped;
        DataModel::Nullable<GenericOverallCurrentState> mInitialOverallCurrentState = DataModel::NullNullable;
    };

    /**
     * Creates a Closure Control Cluster instance from a fully-built Config.
     * @param config The configuration carrying delegate, timer delegate, feature map, optional
     *               attributes and initial state for this cluster instance.
     */
    ClosureControlCluster(const Config & config);
    ~ClosureControlCluster();

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    DataModel::Nullable<ElapsedS> GetCountdownTime() const { return mCountdownTime.value(); }
    MainStateEnum GetMainState() const { return mMainState; }
    DataModel::Nullable<GenericOverallCurrentState> GetOverallCurrentState() const { return mOverallCurrentState; }
    DataModel::Nullable<GenericOverallTargetState> GetOverallTargetState() const { return mOverallTargetState; }
    BitFlags<LatchControlModesBitmap> GetLatchControlModes() const { return mLatchControlModes; }
    BitFlags<Feature> GetFeatureMap() const { return mFeatureMap; }

    /**
     * @brief Gets the current error list.
     *        This method is used to retrieve the current error list.
     *        The outputSpan must initially be of size kCurrentErrorListMaxSize and will be resized to the correct size for the
     * list.
     * @param[out] outputSpan The span to fill with the current error list.
     *
     * @return CHIP_NO_ERROR if the retrieval was successful.
     *         CHIP_ERROR_BUFFER_TOO_SMALL if the outputSpan size is not equal to kCurrentErrorListMaxSize.
     */
    CHIP_ERROR GetCurrentErrorList(Span<ClosureErrorEnum> & outputSpan);

    /**
     * @brief Set SetOverallCurrentState.
     *
     * @param[in] overallCurrentState SetOverallCurrentState Position, Latch and Speed.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetOverallCurrentState(const DataModel::Nullable<GenericOverallCurrentState> & overallCurrentState);

    /**
     * @brief Set OverallTargetState.
     *
     * @param[in] overallTarget OverallTargetState Position, Latch and Speed.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetOverallTargetState(const DataModel::Nullable<GenericOverallTargetState> & overallTarget);

    /**
     * @brief Sets the main state of the cluster.
     *        This method also generates the EngageStateChanged event based on MainState transition.
     *        This method also updates the CountdownTime attribute based on MainState
     *
     * @param[in] mainState - The new main state to be set.
     *
     * @return CHIP_NO_ERROR if the main state is set successfully.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if new MainState is not supported.
     *         CHIP_ERROR_INCORRECT_STATE if the transition to new MainState is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetMainState(MainStateEnum mainState);

    /**
     * @brief Triggers an update to report a new countdown time from application.
     *        This method should be called whenever the application needs to update the countdown time.
     *
     * @param[in] countdownTime - Updated countdown time to be reported.
     *
     * @return CHIP_NO_ERROR if the countdown time is set successfully.
     *         Returns an appropriate error code if the countdown time update fails
     */
    inline CHIP_ERROR SetCountdownTimeFromDelegate(const DataModel::Nullable<ElapsedS> & countdownTime)
    {
        return SetCountdownTime(countdownTime, true);
    }

    /**
     * @brief Adds error to current error list.
     *
     * @param[in] error The error to be added to the current error list.
     *
     * @return CHIP_NO_ERROR if the error was added successfully.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR AddErrorToCurrentErrorList(ClosureErrorEnum error);

    /**
     * @brief Clears the current error list.
     *        This method should be called whenever the current error list needs to be reset.
     */
    void ClearCurrentErrorList();

    /**
     *  @brief Calls delegate HandleStopCommand function after validating MainState, parameters and conformance.
     *
     *  @return Success if the Stop command not supported from present Mainstate.
     *          UnsupportedCommand if Instantaneous feature is supported.
     *          Success on succesful handling or Error Otherwise
     */
    Protocols::InteractionModel::Status HandleStop();

    /**
     *  @brief Calls delegate HandleMoveToCommand function after validating the parameters and conformance.
     *
     *  @param [in] position target position
     *  @param [in] latch Target latch
     *  @param [in] speed Target speed
     *
     *  @return ConstraintError if the input values are out of range.
     *          InvalidInState if the MoveTo command not supported from present Mainstate.
     *          Success on succesful handling.
     */
    Protocols::InteractionModel::Status HandleMoveTo(Optional<TargetPositionEnum> position, Optional<bool> latch,
                                                     Optional<Globals::ThreeLevelAutoEnum> speed);

    /**
     *  @brief Calls delegate HandleCalibrateCommand function after validating the parameters and conformance.
     *
     *  @return ConstraintError if the input values are out of range.
     *          InvalidInState if the Calibrate command not supported from present Mainstate.
     *          Success on succesful handling.
     */
    Protocols::InteractionModel::Status HandleCalibrate();

    /**
     * @brief Generates OperationalError event.
     *        This method should be called whenever when a reportable error condition is detected
     *
     * @param [in] errorState current error list
     *
     * @return CHIP_NO_ERROR if the event is generated successfully
     *         Returns an appropriate error code if event generation fails
     */
    CHIP_ERROR GenerateOperationalErrorEvent(const DataModel::List<const ClosureErrorEnum> & errorState);

    /**
     * @brief Generates MovementCompleted event.
     *        This method should be called whenever when the overall operation ends either successfully or otherwise.
     *
     * @return CHIP_NO_ERROR if the event is generated successfull
     *         CHIP_NO_ERROR if the Positioning feature is not supported.
     *         Returns an appropriate error code if event generation fails
     */
    CHIP_ERROR GenerateMovementCompletedEvent();

    /**
     * @brief Generates EngageStateChanged event.
     *        This method should be called whenever when the MainStateEnum attribute changes state to and from disengaged
     *
     * @param[in] EngageValue will indicate if the actuator is Engaged or Disengaged
     *
     * @return CHIP_NO_ERROR if the event is generated successfull
     *         CHIP_NO_ERROR if the ManuallyOperable feature is not supported.
     *         Returns an appropriate error code if event generation fails
     */
    CHIP_ERROR GenerateEngageStateChangedEvent(const bool engageValue);

    /**
     * @brief Generates EngageStateChanged event.
     *        This method should be called whenever when the SecureState field in the OverallCurrentState attribute changes.
     *
     * @param[in] secureValue will indicate whether a closure is securing a space against possible unauthorized entry.
     *
     * @return CHIP_NO_ERROR if the event is generated successfull
     *         CHIP_NO_ERROR if the feature conformance is not supported
     *         Returns an appropriate error code if event generation fails.
     */
    CHIP_ERROR GenerateSecureStateChangedEvent(const bool secureValue);

private:
    ClosureControlClusterDelegate & mDelegate;
    TimerDelegate & mTimerDelegate;
    const BitFlags<Feature> mFeatureMap;
    const OptionalAttributesSet mOptionalAttributes;

    QuieterReportingAttribute<ElapsedS> mCountdownTime;
    MainStateEnum mMainState                                             = MainStateEnum::kUnknownEnumValue;
    DataModel::Nullable<GenericOverallCurrentState> mOverallCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericOverallTargetState> mOverallTargetState   = DataModel::NullNullable;
    const BitFlags<LatchControlModesBitmap> mLatchControlModes;
    ClosureErrorEnum mCurrentErrorList[kCurrentErrorListMaxSize] = {};

    // The current error count is used to track the number of errors in the CurrentErrorList.
    size_t mCurrentErrorCount = 0;

    /**
     * @brief Function validates if the requested mainState is supported by the closure.
     *        Function validates agaisnt the FeatureMap conformance to validate support.
     *
     *        - Stopped, Moving, WaitingForMotion, Error and SetupRequired always return true since they are mandatory.
     *        - Calibrating returns true if the Calibration feature is supported, false otherwise.
     *        - Protected returns true if the Proitection feature is supported, false otherwise.
     *        - Disengaged returns true if the ManuallyOperable feature is supported, false otherwise.
     *        - Returns true if the requested MainState is not a known state.
     *
     * @param mainState requested MainState to validate
     *
     * @return true, if the requested MainState is supported
     *         false, otherwise
     */
    bool IsSupportedMainState(MainStateEnum mainState) const;

    /**
     * @brief Function validates if the requested overallCurrentState positioning is supported by the closure.
     *        Function validates against the FeatureMap conformance to validate support.
     *
     * @param positioning requested Positioning to validate
     *
     * @return true if the requested Positioning is supported
     *        false, otherwise
     */
    bool IsSupportedOverallCurrentStatePositioning(CurrentPositionEnum positioning) const;

    /**
     * @brief Function validates if the requested OverallTargetState positioning is supported by the closure.
     *        Function validates agaisnt the FeatureMap conformance to validate support.
     *
     * @param positioning requested Positioning to validate
     *
     * @return true if the requested Positioning is supported
     *        false, otherwise
     */
    bool IsSupportedOverallTargetStatePositioning(TargetPositionEnum positioning) const;

    /**
     * @brief Updates the countdown time based on the Quiet reporting conditions of the attribute.
     *
     * @param fromDelegate true if the countdown time is being configured by the delegate, false otherwise
     */
    CHIP_ERROR SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime, bool fromDelegate);

    /**
     * @brief Updates the countdown time from cluster logic.
     *        This method should be invoked whenever the cluster logic needs to update the countdown time.
     *        This includes:
     *         - When the tracked operation changes due to an update in the MainState attribute
     *
     * @param[in] countdownTime - Updated countdown time to be reported.
     *
     * @return CHIP_NO_ERROR if the countdown time is set successfully.
     *         Returns an appropriate error code if the countdown time update fails
     */
    inline CHIP_ERROR SetCountdownTimeFromCluster(const DataModel::Nullable<ElapsedS> & countdownTime)
    {
        return SetCountdownTime(countdownTime, false);
    }

    /**
     * @brief Reads the CurrentErrorList attribute.
     *        This method is used to read the CurrentErrorList attribute and encode it using the provided encoder.
     *
     * @param[in] encoder The encoder to use for encoding the CurrentErrorList attribute.
     *
     * @return CHIP_NO_ERROR if the read was successful.
     */
    CHIP_ERROR ReadCurrentErrorListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder);

    EndpointId GetEndpointId() { return mPath.mEndpointId; }
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
