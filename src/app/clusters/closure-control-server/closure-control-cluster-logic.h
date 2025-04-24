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
#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/clusters/closure-control-server/closure-control-cluster-delegate.h>
#include <app/clusters/closure-control-server/closure-control-cluster-matter-context.h>
#include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/logging/CHIPLogging.h>
#include <app/AttributeAccessInterface.h>

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
        // - Otherwise strategies manages within the set function
        mCountdownTime.policy()
            .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement)
            .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
    };

    QuieterReportingAttribute<ElapsedS> mCountdownTime{ DataModel::NullNullable };
    MainStateEnum mMainState                                 = MainStateEnum::kUnknownEnumValue;
    DataModel::Nullable<GenericOverallState> mOverallState   = DataModel::NullNullable;
    DataModel::Nullable<GenericOverallTarget> mOverallTarget = DataModel::NullNullable;

    // CurrentErrorList attribute is not stored here. When it is necessary it will be requested from the delegate to get the current
    // active errors.
};

/**
 * @brief Struct to store the cluster initialization parameters
 */
struct ClusterInitParameters
{
    MainStateEnum mMainState                               = MainStateEnum::kStopped;
    DataModel::Nullable<GenericOverallState> mOverallState = DataModel::NullNullable;
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
    ClusterLogic(DelegateBase & delegate, MatterContext & matterContext) : mDelegate(delegate), mMatterContext(matterContext)
    {}

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
    CHIP_ERROR GetOverallState(DataModel::Nullable<GenericOverallState> & overallState);
    CHIP_ERROR GetOverallTarget(DataModel::Nullable<GenericOverallTarget> & overallTarget);
    CHIP_ERROR GetCurrentErrorList(const AttributeValueEncoder::ListEncodeHelper & aEncoder);

    /**
     * @brief Set OverallTarget.
     * 
     * @param[in] overallTarget OverallTarget Position, Latch and Speed.
     * 
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState);
    
    /**
     * @brief Function to change the MainState.
     *
     *        See SetCountdownTime function comment below
     */
    CHIP_ERROR SetMainState(MainStateEnum mainState);
    
    /**
     * @brief Public API to trigger countdown time update from the delegate (application layer).
     *        Function calls the SetCountdownTime function with the fromDelegate parameter set to true.
     *
     *        See SetCountdownTime function comment below
     */
    inline CHIP_ERROR SetCountdownTimeFromDelegate(const DataModel::Nullable<ElapsedS> & countdownTime)
    {
        return SetCountdownTime(countdownTime, true);
    }
    
    /**
     *  @brief Calls delegate HandleStopCommand function after validating MainState, parameters and conformance.
     * 
     *  @return Exits if the cluster is not initialized.
     *          Success if the Stop command not supported from present Mainstate.
     *          UnsupportedCommand if Instantaneous feature is supported.
     *          Success on succesful handling or Error Otherwise
     */
    chip::Protocols::InteractionModel::Status HandleStop();
    
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
    chip::Protocols::InteractionModel::Status HandleMoveTo(Optional<TargetPositionEnum> position, Optional<bool> latch,
                                                           Optional<Globals::ThreeLevelAutoEnum> speed);
    
    /**
     *  @brief Calls delegate HandleCalibrateCommand function after validating the parameters and conformance.
     * 
     *  @return Exits if the cluster is not initialized.
     *          ConstraintError if the input values are out is out of range.
     *          InvalidInState if the Calibrate command not supported from present Mainstate.
     *          Success on succesful handling.
     */
    chip::Protocols::InteractionModel::Status HandleCalibrate();
    
    /**
     * @brief Post event when a reportable error condition is detected
     * 
     * @param [in] errorState current error list
     * 
     * @return CHIP_NO_ERROR if event posted succesfully
     *         Return error recieved from LogEvent.
     */
    CHIP_ERROR PostOperationalErrorEvent(const DataModel::List<const ClosureErrorEnum> & errorState);

    /**
     * @brief Post event, if supported, when the overall operation ends, either successfully or otherwise
     * 
     * @return CHIP_NO_ERROR if event posted succesfully
     *         CHIP_NO_ERROR if positioning feature is not supported.
     *         Return error recieved from LogEvent.
     */
    CHIP_ERROR PostMovementCompletedEvent();

    /**
     * @brief Post event, if supported,when the MainStateEnum attribute changes state to and from disengaged
     * 
     * @param[in] EngageValue will indicate if the actuator is Engaged or Disengaged
     * 
     * @return CHIP_NO_ERROR if event posted succesfully
     *         CHIP_NO_ERROR if manuallyOperable feature is not supported.
     *         Return error recieved from LogEvent.
     */
    CHIP_ERROR PostEngageStateChangedEvent(const bool engageValue);

    /**
     * @brief Post event, if supported, when the SecureState field in the OverallState attribute changes.
     * 
     * @param[in] secureValue will indicate whether a closure is securing a space against possible unauthorized entry.
     * @return CHIP_NO_ERROR if event posted succesfully
     *         CHIP_NO_ERROR if feature conformance is not supported
     *         Return error recieved from LogEvent.
     */
    CHIP_ERROR PostSecureStateChangedEvent(const bool secureValue);

private:
    /**
     * @brief Function validates if the requested mainState is supported by the device.
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
    bool IsSupportedMainState(MainStateEnum mainState);

    /**
     * @brief Function validates if the requested mainState is a valid transition from the current state.
     *        TODO: Add functionnal description of the state machine
     *
     * @param mainState requested main state to be applied
     *
     * @return true, transition from current to requested is valid
     *         false, otherwise
     */
    bool IsValidMainStateTransition(MainStateEnum mainState);

    /**
     * @brief Function validates if the requested overallState positioning is supported by the device.
     *        Function validates against the FeatureMap conformance to validate support.
     *
     *        - FullyClosed, FullyOpened, PartiallyOpened and OpenedAtSignature always return true since they are mandatory.
     *        - OpenedForPedestrian returns true if the Pedestrian feature is supported, false otherwise.
     *        - OpenedForVentilation returns true if the Ventilation feature is supported, false otherwise.
     *
     * @param positioning requested Positioning to validate
     *
     * @return true if the requested Positioning is supported
     *        false, otherwise
     */
    bool IsSupportedOverallStatePositioning(PositioningEnum positioning);
    
    /**
     * @brief Function validates if the requested OverallTarget positioning is supported by the device.
     *        Function validates agaisnt the FeatureMap conformance to validate support.
     *
     *        - CloseInFull, OpenInFull and Signature always return true since they are mandatory.
     *        - Pedestrian returns true if the Pedestrian feature is supported, false otherwise.
     *        - Ventilation returns true if the Ventilation feature is supported, false otherwise.
     *
     * @param positioning requested Positioning to validate
     *
     * @return true if the requested Positioning is supported
     *        false, otherwise
     */
    bool IsSupportedOverallTargetPositioning(TargetPositionEnum positioning);

    /**
     * @brief Update the stored countdown time
     *        When the attribute should be marked dirty changes based on wether the change is from the delegate or the internal
     *        cluster processing.
     *
     *        If countdown time is changed due to the internal cluster logic, the attribute will be marked dirty,
     *        otherwise, configured quiet reporting rules apply
     *
     *        See Set function comment above for common behavior
     *
     * @param fromDelegate true if the coutdown time is being configured by the delegate, false otherwise
     */
    CHIP_ERROR SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime, bool fromDelegate);
    
        
    /**
     * @brief API to trigger countdown time update from the cluster logic.
     *        Function calls the SetCountdownTime function with the fromDelegate parameter set to false.
     *
     *        See SetCountdownTime function comment above
     */
    inline CHIP_ERROR SetCountdownTimeFromCluster(const DataModel::Nullable<ElapsedS> & countdownTime)
    {
        return SetCountdownTime(countdownTime, false);
    }
    
    /**
     * @brief Set OverallTarget.
     * 
     * @param[in] overallTarget OverallTarget Position, Latch and Speed.
     * 
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetOverallTarget(const DataModel::Nullable<GenericOverallTarget> & overallTarget);

    bool mIsInitialized = false;
    DelegateBase & mDelegate;
    ClusterConformance mConformance;
    ClusterState mState;
    MatterContext & mMatterContext;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
