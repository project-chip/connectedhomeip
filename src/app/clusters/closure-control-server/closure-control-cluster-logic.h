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
    {
        // TODO remove this
        (void) mDelegate;
    }

    ~ClusterLogic() = default;

    const ClusterConformance & GetConformance() const { return mConformance; }
    const ClusterState & GetState() const { return mState; }

    /**
     * @brief Initializes the cluster logic
     *        Validates that the provided conformance is spec compliant
     *
     * @param[in] conformance
     * @return CHIP_ERROR
     */
    CHIP_ERROR Init(const ClusterConformance & conformance);

    // All Get functions
    // Return CHIP_ERROR_INCORRECT_STATE if the class has not been initialized.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported by the conformance.
    // Otherwise return CHIP_NO_ERROR and set the input parameter value to the current cluster state value

    CHIP_ERROR GetCountdownTime(DataModel::Nullable<ElapsedS> & countdownTime);
    CHIP_ERROR GetMainState(MainStateEnum & mainState);
    CHIP_ERROR GetOverallState(DataModel::Nullable<GenericOverallState> & overallState);
    CHIP_ERROR GetOverallTarget(DataModel::Nullable<GenericOverallTarget> & overallTarget);

    // TODO: Add ErrorList guetter with its implementation

    // All Set functions
    // Return CHIP_ERROR_INCORRECT_STATE if the class has not been initialized.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported by the conformance.
    // Return CHIP_ERROR_INVALID_ARGUMENT if the input value is out of range.
    // Returns CHIP_ERROR_PERSISTED_STORAGE_FAILED if the value could not not be stored in persistent storage.
    // Otherwise return CHIP_NO_ERROR and set the parameter value in the cluster state
    // Set functions are supplied for any values that can be set either internally by the device or externally
    // through a direct attribute write. Changes to attributes that happen as a side effect of cluster commands
    // are handled by the cluster command handlers.

    CHIP_ERROR SetMainState(MainStateEnum mainState);
    CHIP_ERROR SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState);

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
     * @brief Function validates if the requested positioning is supported by the device.
     *        Function validates agaisnt the FeatureMap conformance to validate support.
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
    bool IsSupportedPositioning(PositioningEnum positioning);

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
     * @brief Set the Overall Target object
     *
     * @param overallTarget
     * @return CHIP_ERROR
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
