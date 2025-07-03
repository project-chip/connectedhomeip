/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under the License.
 */

/**
 * @file Cross-platform API to handle cluster-specific logic for the Closure Control cluster on a single endpoint.
 */

#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <app/AttributeAccessInterface.h>
#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/clusters/closure-control-server/closure-control-cluster-delegate.h>
#include <app/clusters/closure-control-server/closure-control-cluster-matter-context.h>
#include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * @brief Closure Control optional attribute enum class
 */
enum class OptionalAttributeEnum : uint32_t
{
    kCountdownTime = 0x1
};
// As per the spec, the maximum allowed CurrentErrorList size is 10.
constexpr int kCurrentErrorListMaxSize = 10;

/**
 * @brief Structure is used to configure and validate the Cluster configuration.
 *        Validates if the feature map, attributes and commands configuration is valid.
 */
struct ClusterConformance
{
public:
    BitFlags<Feature> & FeatureMap() { return mFeatureMap; }
    const BitFlags<Feature> & FeatureMap() const { return mFeatureMap; }

    BitFlags<OptionalAttributeEnum> & OptionalAttributes() { return mOptionalAttributes; }
    const BitFlags<OptionalAttributeEnum> & OptionalAttributes() const { return mOptionalAttributes; }

    inline bool HasFeature(Feature aFeature) const { return mFeatureMap.Has(aFeature); }

    /**
     * @brief Function determines if Cluster conformance is valid
     *
     *        The function executes these checks in order to validate the conformance
     *        1. Check if either Positioning or MotionLatching is supported. If neither are enabled, returns false.
     *        2. If Speed is enabled, checks that Positioning is enabled and Instantaneous is disabled. Returns false otherwise.
     *        3. If Ventilation, pedestrian or calibration is enabled, Positioning must be enabled. Return false otherwise.
     *
     * @return true, the cluster confirmance is valid
     *         false, otherwise
     */
    bool Valid() const
    {
        // Positioning or Matching must be enabled
        VerifyOrReturnValue(HasFeature(Feature::kPositioning) || HasFeature(Feature::kMotionLatching), false,
                            ChipLogError(AppServer, "Validation failed: Neither Positioning nor MotionLatching is enabled."));

        // If Speed is enabled, Positioning shall be enabled and Instantaneous shall be disabled.
        if (HasFeature(Feature::kSpeed))
        {
            VerifyOrReturnValue(
                HasFeature(Feature::kPositioning) && !HasFeature(Feature::kInstantaneous), false,
                ChipLogError(AppServer, "Validation failed: Speed requires Positioning enabled and Instantaneous disabled."));
        }

        if (HasFeature(Feature::kVentilation) || HasFeature(Feature::kPedestrian) || HasFeature(Feature::kCalibration))
        {
            VerifyOrReturnValue(
                HasFeature(Feature::kPositioning), false,
                ChipLogError(AppServer,
                             "Validation failed: Ventilation, Pedestrian, or Calibration requires Positioning enabled."));
        }

        return true;
    }

private:
    BitFlags<Feature> mFeatureMap;
    BitFlags<OptionalAttributeEnum> mOptionalAttributes;
};

/**
 * @brief Struct to store the current cluster state
 */
struct ClusterState
{
    ClusterState()
    {
        // Configure CountdownTime Quiet Reporting strategies
        // - When it changes from 0 to any other value and vice versa
        // - When it increases
        // - When it changes from null to any other value and vice versa (default support)
        mCountdownTime.policy()
            .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement)
            .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
    };

    QuieterReportingAttribute<ElapsedS> mCountdownTime{ DataModel::NullNullable };
    MainStateEnum mMainState                                             = MainStateEnum::kUnknownEnumValue;
    DataModel::Nullable<GenericOverallCurrentState> mOverallCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericOverallTargetState> mOverallTargetState   = DataModel::NullNullable;
    BitFlags<LatchControlModesBitmap> mLatchControlModes;
    ClosureErrorEnum mCurrentErrorList[kCurrentErrorListMaxSize] = {};

    // The current error count is used to track the number of errors in the CurrentErrorList.
    size_t mCurrentErrorCount = 0;
};

/**
 * @brief Struct to store the cluster initialization parameters
 */
struct ClusterInitParameters
{
    MainStateEnum mMainState                                             = MainStateEnum::kStopped;
    DataModel::Nullable<GenericOverallCurrentState> mOverallCurrentState = DataModel::NullNullable;
};

/**
 * @brief Class containing the cluster business logic
 *
 */
class ClusterLogic
{
public:
    // Instantiates a ClusterLogic class. The caller maintains ownership of the driver and the context, but provides them for use by
    // the ClusterLogic class.
    ClusterLogic(DelegateBase & delegate, MatterContext & matterContext) : mDelegate(delegate), mMatterContext(matterContext) {}

    ~ClusterLogic() = default;

    const ClusterConformance & GetConformance() const { return mConformance; }
    const ClusterState & GetState() const { return mState; }

    /**
     * @brief Initializes the cluster logic
     *        Validates that the provided conformance is spec compliant.
     *        Set the initPrasams in their respective attributes.
     *
     * @param[in] conformance
     * @param[in] initParams
     *
     * @return CHIP_ERROR_INCORRECT_STATE if the class has already been initialized.
     *         Set fucntion Specific errors, if set of initParams failed.
     *         CHIP_NO_ERROR on successful Initialization.
     */
    CHIP_ERROR Init(const ClusterConformance & conformance, const ClusterInitParameters & initParams);

    // All Get functions
    // Return CHIP_ERROR_INCORRECT_STATE if the class has not been initialized.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported by the conformance.
    // Otherwise return CHIP_NO_ERROR and set the input parameter value to the current cluster state value

    CHIP_ERROR GetCountdownTime(DataModel::Nullable<ElapsedS> & countdownTime);
    CHIP_ERROR GetMainState(MainStateEnum & mainState);
    CHIP_ERROR GetOverallCurrentState(DataModel::Nullable<GenericOverallCurrentState> & overallCurrentState);
    CHIP_ERROR GetOverallTargetState(DataModel::Nullable<GenericOverallTargetState> & overallTarget);
    CHIP_ERROR GetLatchControlModes(BitFlags<LatchControlModesBitmap> & latchControlModes);
    CHIP_ERROR GetFeatureMap(BitFlags<Feature> & featureMap);
    CHIP_ERROR GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision);

    /**
     * @brief Gets the current error list.
     *        This method is used to retrieve the current error list.
     *        The outputSpan must initially be of size kCurrentErrorListMaxSize and will be resized to the correct size for the
     * list.
     * @param[out] outputSpan The span to fill with the current error list.
     *
     * @return CHIP_NO_ERROR if the retrieval was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_BUFFER_TOO_SMALL if the outputSpan size is not equal to kCurrentErrorListMaxSize.
     */
    CHIP_ERROR GetCurrentErrorList(Span<ClosureErrorEnum> & outputSpan);

    /**
     * @brief Reads the CurrentErrorList attribute.
     *        This method is used to read the CurrentErrorList attribute and encode it using the provided encoder.
     *
     * @param[in] encoder The encoder to use for encoding the CurrentErrorList attribute.
     *
     * @return CHIP_NO_ERROR if the read was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     */
    CHIP_ERROR ReadCurrentErrorListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder);

    /**
     * @brief Set SetOverallCurrentState.
     *
     * @param[in] overallCurrentState SetOverallCurrentState Position, Latch and Speed.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
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
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
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
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if new MainState is not supported.
     *         CHIP_ERROR_INCORRECT_STATE if the transition to new MainState is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetMainState(MainStateEnum mainState);

    /**
     * @brief Sets the latch control modes for the closure control cluster.
     *        This method updates the latch control modes using the provided bit flags.
     *
     * @param[in] latchControlModes  Reference to a BitFlags object representing the desired latch control modes.
     *
     * @return CHIP_ERROR Returns CHIP_NO_ERROR on success, or an appropriate error code on failure.
     */
    CHIP_ERROR SetLatchControlModes(const BitFlags<LatchControlModesBitmap> & latchControlModes);

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
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
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
     *  @return Exits if the cluster is not initialized.
     *          Success if the Stop command not supported from present Mainstate.
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
     *  @return Exits if the cluster is not initialized.
     *          ConstraintError if the input values are out is out of range.
     *          InvalidInState if the MoveTo command not supported from present Mainstate.
     *          Success on succesful handling.
     */
    Protocols::InteractionModel::Status HandleMoveTo(Optional<TargetPositionEnum> position, Optional<bool> latch,
                                                     Optional<Globals::ThreeLevelAutoEnum> speed);

    /**
     *  @brief Calls delegate HandleCalibrateCommand function after validating the parameters and conformance.
     *
     *  @return Exits if the cluster is not initialized.
     *          ConstraintError if the input values are out is out of range.
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
    bool mIsInitialized = false;
    DelegateBase & mDelegate;
    ClusterConformance mConformance;
    ClusterState mState;
    MatterContext & mMatterContext;

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
     * @brief Function validates if the requested mainState is a valid transition from the current state.
     *        TODO: Add functionnal description of the state machine
     *
     * @param mainState requested main state to be applied
     *
     * @return true, transition from current to requested is valid
     *         false, otherwise
     */
    bool IsValidMainStateTransition(MainStateEnum mainState) const;

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
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
