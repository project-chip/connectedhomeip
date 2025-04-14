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
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 * @file Cross-platform API to handle cluster-specific logic for the valve configuration and control cluster on a single endpoint.
 */

#pragma once

#include "closure-dimension-cluster-objects.h"
#include "closure-dimension-delegate.h"
#include "closure-dimension-matter-context.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

struct ClusterConformance
{
    inline bool HasFeature(Feature feature) const { return featureMap & to_underlying(feature); }
    uint32_t featureMap;
    bool supportsOverflow = false;
    bool Valid() const
    {
        bool supportsRotation       = HasFeature(Feature::kRotation);
        bool supportsMotionLatching = HasFeature(Feature::kMotionLatching);

        // Overflow attribute can only be supported if device supports Rotation or MotionLatching features
        if (supportsOverflow && !(supportsRotation || supportsMotionLatching))
        {
            ChipLogError(Zcl,
                         "Invalid closure dimension cluster conformance - Overflow is not supported without Rotation or "
                         "MotionLatching features");
            return false;
        }

        // if device supports Rotation feature , Overflow attribute should be supported as per attribute conformance
        if (supportsRotation && !supportsOverflow)
        {
            ChipLogError(Zcl,
                         "Invalid closure dimension cluster conformance - Overflow is mandatory attribute for Rotation feature");
            return false;
        }

        return true;
    }
};

struct ClusterState
{
    GenericCurrentStateStruct currentState{};
    GenericTargetStruct target{};
    Percent100ths resolution                                      = 1;
    Percent100ths stepValue                                       = 1;
    ClosureUnitEnum unit                                          = ClosureUnitEnum::kUnknownEnumValue;
    DataModel::Nullable<Structs::UnitRangeStruct::Type> unitRange = DataModel::Nullable<Structs::UnitRangeStruct::Type>();
    Structs::RangePercent100thsStruct::Type limitRange{};
    TranslationDirectionEnum translationDirection = TranslationDirectionEnum::kUnknownEnumValue;
    RotationAxisEnum rotationAxis                 = RotationAxisEnum::kUnknownEnumValue;
    OverflowEnum overflow                         = OverflowEnum::kUnknownEnumValue;
    ModulationTypeEnum modulationType             = ModulationTypeEnum::kUnknownEnumValue;
};

class ClusterLogic
{
public:
    /**
     *  @brief Instantiates a ClusterLogic class. The caller maintains ownership of the driver and the context,
     *           but provides them for use by the ClusterLogic class.
     */
    ClusterLogic(DelegateBase & clusterDriver, MatterContext & matterContext) :
        mClusterDriver(clusterDriver), mMatterContext(matterContext)
    {}

    const ClusterState & GetState() { return mState; }

    const ClusterConformance & GetConformance() { return mConformance; }

    /**
     *  @brief Validates the conformance and performs initialization
     *  @param [in] conformance cluster conformance
     *  @return Returns CHIP_ERROR_INCORRECT_STATE if the cluster has already been initialized,
     *          Returns CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR if the conformance is incorrect.
     *          Returns CHIP_NO_ERROR on succesful initialization.
     */
    CHIP_ERROR Init(const ClusterConformance & conformance);

    /**
     * @brief Set Current State.
     * @param[in] currentState Current State Position, Latching and/or Speed.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetCurrentState(const GenericCurrentStateStruct & currentState);

    /**
     * @brief Set Target.
     * @param[in] target Target Position, Latching and/or Speed.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetTarget(const GenericTargetStruct & target);

    /**
     * @brief Set Resolution.
     * @param[in] resolution Minimal acceptable change of Position fields of attributes.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetResolution(const Percent100ths resolution);

    /**
     * @brief Set StepValue.
     * @param[in] stepValue One step value for Step command
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetStepValue(const Percent100ths stepValue);

    /**
     * @brief Set Unit.
     * @param[in] unit Unit related to the Positioning.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetUnit(const ClosureUnitEnum unit);

    /**
     * @brief Set UnitRange.
     * @param[in] unitRange Minimum and Maximum values expressed by positioning following the unit.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetUnitRange(const DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange);

    /**
     * @brief Set LimitRange.
     * @param[in] limitRange Range of possible values for the position field in Current attribute.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetLimitRange(const Structs::RangePercent100thsStruct::Type & limitRange);

    /**
     * @brief Set TranslationDirection.
     * @param[in] translationDirection Direction of the translation.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported
     */
    CHIP_ERROR SetTranslationDirection(const TranslationDirectionEnum translationDirection);

    /**
     * @brief Set RotationAxis.
     * @param[in] rotationAxis Axis of the rotation.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported
     */
    CHIP_ERROR SetRotationAxis(const RotationAxisEnum rotationAxis);

    /**
     * @brief Set Overflow.
     * @param[in] overflow Overflow related to Rotation.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetOverflow(const OverflowEnum overflow);

    /**
     * @brief Set ModulationType.
     * @param[in] modulationType Modulation type.
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetModulationType(const ModulationTypeEnum modulationType);

    // All Get functions:
    // Return CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported.
    // Otherwise return CHIP_NO_ERROR and set the input parameter value to the current cluster state value
    CHIP_ERROR GetCurrentState(GenericCurrentStateStruct & currentState);
    CHIP_ERROR GetTarget(GenericTargetStruct & target);
    CHIP_ERROR GetResolution(Percent100ths & resolution);
    CHIP_ERROR GetStepValue(Percent100ths & stepValue);
    CHIP_ERROR GetUnit(ClosureUnitEnum & unit);
    CHIP_ERROR GetUnitRange(DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange);
    CHIP_ERROR GetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange);
    CHIP_ERROR GetTranslationDirection(TranslationDirectionEnum & translationDirection);
    CHIP_ERROR GetRotationAxis(RotationAxisEnum & rotationAxis);
    CHIP_ERROR GetOverflow(OverflowEnum & overflow);
    CHIP_ERROR GetModulationType(ModulationTypeEnum & modulationType);
    CHIP_ERROR GetFeatureMap(Attributes::FeatureMap::TypeInfo::Type & featureMap);
    CHIP_ERROR GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision);

    /**
     *  @brief Calls delegate HandleSetTarget function after validating the parameters and conformance.
     *  @param [in] position target position
     *  @param [in] latch Target latch
     *  @param [in] speed Target speed
     *  @return Returns ConstraintError if the input values are out is out of range
     *          Returns Success on succesful handling.
     */
    Protocols::InteractionModel::Status HandleSetTargetCommand(Optional<Percent100ths> position, Optional<TargetLatchEnum> latch,
                                                               Optional<Globals::ThreeLevelAutoEnum> speed);

    /**
     *  @brief Calls delegate HandleStep function after validating the parameters and confromance.
     *  @param [in] direction step direction
     *  @param [in] numberOfSteps Number of steps
     *  @param [in] speed step speed
     *  @return Returns ConstraintError if the input values are out is out of range
     *          Returns Failure if feature conformance is not valid
     *          Returns Success on succesful handling.
     */
    Protocols::InteractionModel::Status HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps,
                                                          Optional<Globals::ThreeLevelAutoEnum> speed);

private:
    // This cluster implements version 1 of the Closure Dimension cluster. Do not change this revision without updating
    // the cluster to implement the newest features.
    static constexpr Attributes::ClusterRevision::TypeInfo::Type kClusterRevision = 1u;

    bool mInitialized;
    ClusterState mState;
    ClusterConformance mConformance;
    DelegateBase & mClusterDriver;
    MatterContext & mMatterContext;
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
