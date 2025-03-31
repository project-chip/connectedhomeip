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

 #include "closure-dimension-delegate.h"
 #include "closure-dimension-matter-context.h"
 #include "closure-dimension-cluster-objects.h"
 #include <app-common/zap-generated/cluster-objects.h>
 #include <app/cluster-building-blocks/QuieterReporting.h>
 #include <app/data-model/Nullable.h>
 #include <lib/core/CHIPError.h>
 #include <system/SystemLayer.h>
 
 namespace chip {
 namespace app {
 namespace Clusters {
 namespace ClosureDimension {
 
 struct ClusterConformance
 {
    inline bool HasFeature(Feature feature) const { return featureMap & to_underlying(feature); }
    uint32_t featureMap;
    bool supportsOverflow;
    bool Valid() const
    {
        bool supportsRotation = HasFeature(Feature::kRotation);
        bool supportsMotionLatching = HasFeature(Feature::kMotionLatching);
        if (supportsOverflow && !(supportsRotation || supportsMotionLatching))
        {
            ChipLogError(Zcl,
                         "Invalid Valve configuration and control conformance - Overflow is not supported without Rotation or MotionLatching features");
            return false;
        }
        return true;
    }
 };
 
 struct ClusterState
 {
    GenericCurrentStruct current;
    GenericTargetStruct target;
    Percent100ths resolution = 0;
    Percent100ths stepValue = 0;
    ClosureUnitEnum unit = ClosureUnitEnum::kMillimeter;
    Structs::UnitRangeStruct::Type unitRange;
    Structs::RangePercent100thsStruct::Type limitRange;
    TranslationDirectionEnum translationDirection = TranslationDirectionEnum::kBackward;
    RotationAxisEnum rotationAxis = RotationAxisEnum::kBottom;
    OverflowEnum overflow = OverflowEnum::kBottomInside;
    ModulationTypeEnum modulationType = ModulationTypeEnum::kOpacity;
 };
 
 class ClusterLogic
 {
 public:
     // Instantiates a ClusterLogic class. The caller maintains ownership of the driver and the context, but provides them for use by
     // the ClusterLogic class.
     ClusterLogic(DelegateBase & clusterDriver, MatterContext & matterContext) :
         mClusterDriver(clusterDriver), mMatterContext(matterContext)
     {
         (void) mClusterDriver;
     }
     
     const ClusterState & GetState() { return mState; }
     
     const ClusterConformance & GetConformance() { return mConformance; }
     
    
 
     // Validates the conformance and performs initialization.
     // Returns CHIP_ERROR_INCORRECT_STATE if the cluster has already been initialized.
     // Returns CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR if the conformance is incorrect.
    CHIP_ERROR Init(const ClusterConformance & conformance);
    
    /**
     * @brief Set Current.
     * @param[in] current Current Position, Latching and/or Speed.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetCurrent(GenericCurrentStruct & current);
    
    /**
     * @brief Set Target.
     * @param[in] target Target Position, Latching and/or Speed.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetTarget(GenericTargetStruct & target);
    
    /**
     * @brief Set Resolution.
     * @param[in] resolution Minimal acceptable change of Position fields of attributes.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetResolution(const Percent100ths resolution);
    
    /**
     * @brief Set StepValue.
     * @param[in] stepValue One step value for Step command
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetStepValue(const Percent100ths stepValue);
    
    /**
     * @brief Set Unit.
     * @param[in] unit Unit related to the Positioning.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetUnit(const ClosureUnitEnum unit);
    
    /**
     * @brief Set UnitRange.
     * @param[in] unitRange Minimum and Maximum values expressed by positioning following the unit.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetUnitRange(Structs::UnitRangeStruct::Type & unitRange);
    
    /**
     * @brief Set LimitRange.
     * @param[in] limitRange Range of possible values for the position field in Current attribute.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange);
    
    /**
     * @brief Set TranslationDirection.
     * @param[in] translationDirection Direction of the translation.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetTranslationDirection(const TranslationDirectionEnum translationDirection);
    
    /**
     * @brief Set RotationAxis.
     * @param[in] rotationAxis Axis of the rotation.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetRotationAxis(const RotationAxisEnum rotationAxis);
    
    /**
     * @brief Set Overflow.
     * @param[in] overflow Overflow related to Rotation.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetOverflow(const OverflowEnum overflow);
    
    /**
     * @brief Set ModulationType.
     * @param[in] modulationType Modulation type.
     * @return CHIP_NO_ERROR if set was successful.
     */
    CHIP_ERROR SetModulationType(const ModulationTypeEnum modulationType);
 
    // All Get functions:
     // Return CHIP_ERROR_INCORRECT_STATE if the class has not been initialized.
     // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported by the conformance.
     // Otherwise return CHIP_NO_ERROR and set the input parameter value to the current cluster state value
     CHIP_ERROR GetCurrent(GenericCurrentStruct & current);
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
 
     // Returns CHIP_ERROR_INCORRECT_STATE if the class has not been initialized.
     // Returns CHIP_ERROR_INVALID_ARGUMENT if the input values are out is out of range or the targetLevel is supplied when LVL is
     // not supported.
     // Calls delegate HandleOpen function after validating the parameters
     chip::Protocols::InteractionModel::Status HandleSetTargetCommand(chip::Optional<chip::Percent100ths> position, chip::Optional<TargetLatchEnum> latch, chip::Optional<Globals::ThreeLevelAutoEnum> speed);
 
     // Returns CHIP_ERROR_INCORRECT_STATE if the class has not been initialized.
     // Calls delegate HandleClose function after validating the parameters and stops any open duration timers.
     chip::Protocols::InteractionModel::Status HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps, chip::Optional<Globals::ThreeLevelAutoEnum> speed);
 
 private:
    // This cluster implements version 1 of the valve cluster. Do not change this revision without updating
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
 