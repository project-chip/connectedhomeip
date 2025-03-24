/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

 #include "closure-dimension-cluster-logic.h"

 #include <chrono>
 
 #include <app-common/zap-generated/ids/Attributes.h>
 #include <lib/core/CHIPError.h>
 #include <platform/CHIPDeviceLayer.h>
 #include <system/SystemClock.h>
 
 namespace chip {
 namespace app {
 namespace Clusters {
 namespace ClosureDimension {
 
//  void ClusterStateAttributes::Init(ClusterInitParameters state)
//  {
//  }
 
 CHIP_ERROR ClusterStateAttributes::SetCurrent(GenericCurrentStruct & current)
 {
     bool dirty          = !(current == mState.current);
     mState.current = current;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::Current::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetTarget(GenericTargetStruct & target)
 {
     bool dirty = !(target == mState.target);
     mState.target = target;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::Target::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetResolution(const Percent100ths resolution)
 {
     bool dirty           = resolution != mState.resolution;
     mState.resolution = resolution;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::Resolution::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetStepValue(const Percent100ths stepValue)
 {
     bool dirty          = stepValue != mState.stepValue;
     mState.stepValue = stepValue;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::StepValue::Id);
     }
     return CHIP_NO_ERROR;
 }
 CHIP_ERROR ClusterStateAttributes::SetUnit(const ClosureUnitEnum unit)
 {
     bool dirty         = unit != mState.unit;
     mState.unit = unit;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::Unit::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetUnitRange(Structs::UnitRangeStruct::Type & unitRange)
 {
     bool dirty          = ((unitRange.min != mState.unitRange.min) || (unitRange.max != mState.unitRange.max));
     mState.unitRange.min = unitRange.min;
     mState.unitRange.max = unitRange.max;
     
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::UnitRange::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange)
 {
     bool dirty         = ((limitRange.min != mState.limitRange.min) || (limitRange.max != mState.limitRange.max));
     mState.limitRange.min = limitRange.min;
     mState.limitRange.max = limitRange.max;
     
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::LimitRange::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetTranslationDirection(const TranslationDirectionEnum translationDirection)
 {
     bool dirty = translationDirection != mState.translationDirection;
     mState.translationDirection = translationDirection;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::TranslationDirection::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetRotationAxis(const RotationAxisEnum rotationAxis)
 {
     bool dirty        = rotationAxis != mState.rotationAxis;
     mState.rotationAxis = rotationAxis;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::RotationAxis::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetOverflow(const OverflowEnum overflow)
 {
     bool dirty       = overflow != mState.overflow;
     mState.overflow = overflow;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::Overflow::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterStateAttributes::SetModulationType(const ModulationTypeEnum modulationType)
 {
     bool dirty       = modulationType != mState.modulationType;
     mState.modulationType = modulationType;
     if (dirty)
     {
         mMatterContext.MarkDirty(Attributes::ModulationType::Id);
     }
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance)
 {
    if (!conformance.Valid())
    {
        return CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR;
    }
    mConformance = conformance;
    //mState.Init(initialState);
    mInitialized = true;
    return CHIP_NO_ERROR;
 }
 
 // All Get functions:
 // Return CHIP_ERROR_INVALID_STATE if the class has not been initialized.
 // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE if the attribute is not supported by the conformance.
 // Return CHIP_NO_ERROR and set the parameter value otherwise
 CHIP_ERROR ClusterLogic::GetCurrent(GenericCurrentStruct & current)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     current = mState.GetState().current;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetTarget(GenericTargetStruct & target)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     target = mState.GetState().target;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetResolution(Percent100ths & resolution)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     resolution = mState.GetState().resolution;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetStepValue(Percent100ths & stepValue)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     stepValue = mState.GetState().stepValue;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetUnit(ClosureUnitEnum & unit)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     unit = mState.GetState().unit;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetUnitRange(DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     unitRange = mState.GetState().unitRange;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kLimitation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     limitRange = mState.GetState().limitRange;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetTranslationDirection(TranslationDirectionEnum & translationDirection)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kTranslation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     translationDirection = mState.GetState().translationDirection;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetRotationAxis(RotationAxisEnum & rotationAxis)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kRotation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     rotationAxis = mState.GetState().rotationAxis;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetOverflow(OverflowEnum & overflow)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError((mConformance.HasFeature(Feature::kRotation) || mConformance.HasFeature(Feature::kMotionLatching)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     VerifyOrReturnError(mConformance.supportsOverflow, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     overflow = mState.GetState().overflow;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetModulationType(ModulationTypeEnum & modulationType)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kModulation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     modulationType = mState.GetState().modulationType;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetFeatureMap(Attributes::FeatureMap::TypeInfo::Type & featureMap)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     featureMap = mConformance.featureMap;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     clusterRevision = kClusterRevision;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::HandleSetTarget(std::optional<chip::Percent100ths> position, std::optional<TargetLatchEnum> latch, std::optional<Globals::ThreeLevelAutoEnum> speed)
 {
    // Implement the command and call delegate
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::HandleSetTargetCommand(std::optional<chip::Percent100ths> position, std::optional<TargetLatchEnum> latch, std::optional<Globals::ThreeLevelAutoEnum> speed)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    //validate the parameters
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps, std::optional<Globals::ThreeLevelAutoEnum> speed)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     //validate the parameters
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::HandleStep(StepDirectionEnum direction, uint16_t numberOfSteps, std::optional<Globals::ThreeLevelAutoEnum> speed)
 {
     CHIP_ERROR err = CHIP_NO_ERROR;
     // Implement the command and call delegate
     return err;
 }
 } // namespace ClosureDimension
 } // namespace Clusters
 } // namespace app
 } // namespace chip
 