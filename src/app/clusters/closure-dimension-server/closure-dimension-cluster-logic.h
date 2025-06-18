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

/**
 * @brief Closure dimension feature optional attribute enum class
 */
enum class OptionalAttributeEnum : uint32_t
{
    kOverflow = 0x1
};

/**
 * @brief Structure is used to configure and validate the Cluster configuration.
 *        Validates if the feature map, attributes and commands configuration is valid.
 */
struct ClusterConformance
{
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
     *        2. If Unit, Limitation or speed is enabled, Positioning must be enabled. Return false otherwise.
     *        3. If Translation, Rotation or Modulation is enabled, Positioning must be enabled. Return false otherwise.
     *        4. Only one of Translation, Rotation or Modulation must be enabled. Return false otherwise.
     *        5. If the Overflow attribute is supported, at least one of Rotation or MotionLatching feature must be supported.
     *            Return false otherwise.
     *        6. If Rotation feature is enabled, then the Overflow attribute must be supported. Return false otherwise.
     *
     * @return true, the cluster confirmance is valid
     *         false, otherwise
     */
    bool Valid() const
    {
        // Positioning or Matching must be enabled
        VerifyOrReturnValue(HasFeature(Feature::kPositioning) || HasFeature(Feature::kMotionLatching), false,
                            ChipLogError(AppServer, "Validation failed: Neither Positioning nor MotionLatching is enabled."));

        // If Unit, Limitation or speed is enabled, Positioning must be enabled
        if (HasFeature(Feature::kUnit) || HasFeature(Feature::kLimitation) || HasFeature(Feature::kSpeed))
        {
            VerifyOrReturnValue(
                HasFeature(Feature::kPositioning), false,
                ChipLogError(AppServer, "Validation failed: Unit , Limitation, and speed requires the Positioning feature."));
        }

        // If Translation, Rotation or Modulation is enabled, Positioning must be enabled.
        if (HasFeature(Feature::kTranslation) || HasFeature(Feature::kRotation) || HasFeature(Feature::kModulation))
        {
            VerifyOrReturnValue(
                HasFeature(Feature::kPositioning), false,
                ChipLogError(NotSpecified, "Validation failed: Translation, Rotation or Modulation requires Positioning enabled."));
        }

        // Only one of Translation, Rotation or Modulation features must be enabled. Return false otherwise.
        if ((HasFeature(Feature::kTranslation) && HasFeature(Feature::kRotation)) ||
            (HasFeature(Feature::kRotation) && HasFeature(Feature::kModulation)) ||
            (HasFeature(Feature::kModulation) && HasFeature(Feature::kTranslation)))
        {
            ChipLogError(AppServer, "Validation failed: Only one of Translation, Rotation or Modulation feature can be enabled.");
            return false;
        }

        // If the Overflow Attribute is supported, at least one of Rotation or MotionLatching must be supported.
        if (mOptionalAttributes.Has(OptionalAttributeEnum::kOverflow))
        {
            VerifyOrReturnValue(
                HasFeature(Feature::kRotation) || HasFeature(Feature::kMotionLatching), false,
                ChipLogError(NotSpecified,
                             "Validation failed: If the Overflow attribute is supported, at least one of Rotation or "
                             "MotionLatching must be supported."));
        }

        // If Rotation feature is supported, the Overflow attribute must be supported.
        if (HasFeature(Feature::kRotation))
        {
            VerifyOrReturnValue(
                mOptionalAttributes.Has(OptionalAttributeEnum::kOverflow), false,
                ChipLogError(
                    NotSpecified,
                    "Validation failed: If the Rotation feature is supported, then Overflow Attribute must be supported."));
        }

        return true;
    }

private:
    BitFlags<Feature> mFeatureMap;
    BitFlags<OptionalAttributeEnum> mOptionalAttributes;
};

/**
 * @brief Struct to store the cluster Initilization parameters
 */
struct ClusterInitParameters
{
    TranslationDirectionEnum translationDirection = TranslationDirectionEnum::kUnknownEnumValue;
    RotationAxisEnum rotationAxis                 = RotationAxisEnum::kUnknownEnumValue;
    ModulationTypeEnum modulationType             = ModulationTypeEnum::kUnknownEnumValue;
};

/**
 * @brief Struct to store the current cluster state
 */
struct ClusterState
{
    DataModel::Nullable<GenericCurrentStateStruct> currentState{ DataModel::NullNullable };
    DataModel::Nullable<GenericTargetStruct> target{ DataModel::NullNullable };
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
    ClusterLogic(DelegateBase & delegate, MatterContext & matterContext) : mDelegate(delegate), mMatterContext(matterContext) {}

    const ClusterState & GetState() { return mState; }
    const ClusterConformance & GetConformance() { return mConformance; }

    /**
     *  @brief Validates the conformance and performs initialisation and sets up the ClusterInitParameters into Attributes.
     *
     *  @param [in] conformance cluster conformance
     *  @param [in] clusterInitParameters cluster Init Parameters
     *
     *  @return CHIP_ERROR_INCORRECT_STATE if the cluster has already been initialized,
     *          CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR if the conformance is incorrect.
     *          Set function errors if setting the attributes with the provided ClusterInitParameters fails.
     *          CHIP_NO_ERROR on succesful initialisation.
     */
    CHIP_ERROR Init(const ClusterConformance & conformance, const ClusterInitParameters & clusterInitParameters);

    /**
     * @brief Set Current State.
     *
     * @param[in] currentState Current State Position, Latch and Speed.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetCurrentState(const DataModel::Nullable<GenericCurrentStateStruct> & currentState);

    /**
     * @brief Set Target.
     *
     * @param[in] target Target Position, Latch and Speed.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     */
    CHIP_ERROR SetTarget(const DataModel::Nullable<GenericTargetStruct> & target);

    /**
     * @brief Set Resolution.
     *
     * @param[in] resolution Minimal acceptable change of Position fields of attributes.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetResolution(const Percent100ths resolution);

    /**
     * @brief Set StepValue.
     *
     * @param[in] stepValue One step value for Step command
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetStepValue(const Percent100ths stepValue);

    /**
     * @brief Set Unit.
     *
     * @param[in] unit Unit related to the Positioning.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetUnit(const ClosureUnitEnum unit);

    /**
     * @brief Set UnitRange.
     *
     * @param[in] unitRange Minimum and Maximum values expressed by positioning following the unit.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetUnitRange(const DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange);

    /**
     * @brief Set LimitRange.
     *
     * @param[in] limitRange Range of possible values for the position field in Current attribute.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetLimitRange(const Structs::RangePercent100thsStruct::Type & limitRange);

    /**
     * @brief Set Overflow.
     *
     * @param[in] overflow Overflow related to Rotation.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     */
    CHIP_ERROR SetOverflow(const OverflowEnum overflow);

    // All Get functions:
    // Return CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported.
    // Otherwise return CHIP_NO_ERROR and set the input parameter value to the current cluster state value
    CHIP_ERROR GetCurrentState(DataModel::Nullable<GenericCurrentStateStruct> & currentState);
    CHIP_ERROR GetTarget(DataModel::Nullable<GenericTargetStruct> & target);
    CHIP_ERROR GetResolution(Percent100ths & resolution);
    CHIP_ERROR GetStepValue(Percent100ths & stepValue);
    CHIP_ERROR GetUnit(ClosureUnitEnum & unit);
    CHIP_ERROR GetUnitRange(DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange);
    CHIP_ERROR GetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange);
    CHIP_ERROR GetTranslationDirection(TranslationDirectionEnum & translationDirection);
    CHIP_ERROR GetRotationAxis(RotationAxisEnum & rotationAxis);
    CHIP_ERROR GetOverflow(OverflowEnum & overflow);
    CHIP_ERROR GetModulationType(ModulationTypeEnum & modulationType);
    CHIP_ERROR GetFeatureMap(BitFlags<Feature> & featureMap);
    CHIP_ERROR GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision);

    /**
     *  @brief Calls delegate HandleSetTarget function after validating the parameters and conformance.
     *
     *  @param [in] position target position
     *  @param [in] latch Target latch
     *  @param [in] speed Target speed
     *
     *  @return Exits if the cluster is not initialized.
     *          InvalidCommand if none of the input parameters are present.
     *          ConstraintError if the input values are out is out of range.
     *          InvalidInState if the current position of closure is not known.
     *          Success on succesful handling.
     */
    Protocols::InteractionModel::Status HandleSetTargetCommand(Optional<Percent100ths> position, Optional<bool> latch,
                                                               Optional<Globals::ThreeLevelAutoEnum> speed);

    /**
     *  @brief Calls delegate HandleStep function after validating the parameters and conformance.
     *
     *  @param [in] direction step direction
     *  @param [in] numberOfSteps Number of steps
     *  @param [in] speed step speed
     *
     *  @return Exits if the cluster is not initialized.
     *          UnsupportedCommand if Positioning feature is not supported.
     *          ConstraintError if the input values are out is out of range.
     *          InvalidInState if the current position of closure is not known.
     *          Success on successful handling.
     */
    Protocols::InteractionModel::Status HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps,
                                                          Optional<Globals::ThreeLevelAutoEnum> speed);

private:
    /**
     * @brief Set TranslationDirection.
     *             This attribute is not supposed to change once the installation is finalized.
     *             SetTranslationDirection should only be called from Init()
     *
     * @param[in] translationDirection Direction of the translation.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported
     *
     */
    CHIP_ERROR SetTranslationDirection(const TranslationDirectionEnum translationDirection);

    /**
     * @brief Set RotationAxis.
     *              This attribute is not supposed to change once the installation is finalized.
     *              so SetRotationAxis should only be called from Init().
     *
     * @param[in] rotationAxis Axis of the rotation.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported
     *
     */
    CHIP_ERROR SetRotationAxis(const RotationAxisEnum rotationAxis);

    /**
     * @brief Set ModulationType.
     *              This attribute is not supposed to change once the installation is finalized.
     *              so SetModulationType should only be called from Init().
     *
     * @param[in] modulationType Modulation type.
     *
     * @return CHIP_NO_ERROR if set was successful.
     *         CHIP_ERROR_INVALID_ARGUMENT if argument are not valid
     *         CHIP_ERROR_INCORRECT_STATE if the cluster has not been initialized
     *         CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if feature is not supported.
     *
     */
    CHIP_ERROR SetModulationType(const ModulationTypeEnum modulationType);

    bool mInitialized = false;
    ClusterState mState;
    ClusterConformance mConformance;
    DelegateBase & mDelegate;
    MatterContext & mMatterContext;
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
