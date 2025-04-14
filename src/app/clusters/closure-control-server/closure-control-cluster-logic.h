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
     * @brief Functions determines if Cluster confrmance is valid
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

        // If Speed is enabled, Positioning must be enabled and Instantaneous is disabled.
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
    QuieterReportingAttribute<ElapsedS> mCountDownTime{ DataModel::NullNullable };
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
 * @brief Attribute Class Set interface
 *        Class is responsible for setting the attributes of the cluster and marking them as dirty if necessary.
 */
class ClusterStateAttributes
{
public:
    explicit ClusterStateAttributes(MatterContext & matterContext) : mMatterContext(matterContext) { (void) mMatterContext; }
    ~ClusterStateAttributes() = default;

    CHIP_ERROR SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime);
    CHIP_ERROR SetMainState(MainStateEnum mainState);
    CHIP_ERROR SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState);
    CHIP_ERROR SetTargetState(const DataModel::Nullable<GenericOverallTarget> & targetState);

    CHIP_ERROR GetCountdownTime(DataModel::Nullable<ElapsedS> & countdownTime);
    CHIP_ERROR GetMainState(MainStateEnum & mainState);
    CHIP_ERROR GetOverallState(DataModel::Nullable<GenericOverallState> & overallState);
    CHIP_ERROR GetOverallTarget(DataModel::Nullable<GenericOverallTarget> & overallTarget);

private:
    MatterContext & mMatterContext;
    ClusterState mState;
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
    ClusterLogic(DelegateBase & delegate, MatterContext & matterContext) : mDelegate(delegate), mState(matterContext)
    {
        // TODO remove this
        (void) mDelegate;
    }

    ~ClusterLogic() = default;

    const ClusterConformance & GetConformance() const { return mConformance; }
    const ClusterStateAttributes & GetState() const { return mState; }

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

    CHIP_ERROR SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime);
    CHIP_ERROR SetMainState(MainStateEnum mainState);
    CHIP_ERROR SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState);

private:
    bool mIsInitialized = false;

    DelegateBase & mDelegate;

    ClusterConformance mConformance;
    ClusterStateAttributes mState;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
