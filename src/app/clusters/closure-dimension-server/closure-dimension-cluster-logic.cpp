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
    
using namespace Protocols::InteractionModel;
 
CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance)
{
   if (!conformance.Valid())
   {
       return CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR;
   }
   
   mConformance = conformance;
   mInitialized = true;
   return CHIP_NO_ERROR;
}

 CHIP_ERROR ClusterLogic::SetCurrent(GenericCurrentStruct & current)
 {
    //TODO : Q reporting for this attribute
    if(mConformance.HasFeature(Feature::kPositioning)) {
        if(current.position.HasValue()) {
            VerifyOrReturnError(current.position.Value() <= 10000, CHIP_ERROR_INVALID_ARGUMENT);
        }
    } else {
        current.position.ClearValue();
    }
    if(mConformance.HasFeature(Feature::kMotionLatching)) {
        VerifyOrReturnError(EnsureKnownEnumValue(current.latching.Value()) != LatchingEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
    } else {
        current.latching.ClearValue();
    }
    if(mConformance.HasFeature(Feature::kSpeed)) {
        VerifyOrReturnError(EnsureKnownEnumValue(current.speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
        current.speed.ClearValue();
    }
      
    //TODO: Current.Position value SHALL follow the scaling from "Resolution Attribute".
    
    VerifyOrReturnError(current == mState.current,CHIP_NO_ERROR);
    
    mState.current = current;
    mMatterContext.MarkDirty(Attributes::Current::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetTarget(GenericTargetStruct & target)
 {
    if(mConformance.HasFeature(Feature::kPositioning)) {
        if(target.position.HasValue()) {
            VerifyOrReturnError(target.position.Value() <= 10000, CHIP_ERROR_INVALID_ARGUMENT);
        }
    } else {
        target.position.ClearValue();
    }
    if(mConformance.HasFeature(Feature::kMotionLatching)) {
        VerifyOrReturnError(EnsureKnownEnumValue(target.latch.Value()) != TargetLatchEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
    } else {
        target.latch.ClearValue();
    }
    if(mConformance.HasFeature(Feature::kSpeed)) {
        VerifyOrReturnError(EnsureKnownEnumValue(target.speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
        target.speed.ClearValue();
    }
    
    //TODO: Target.Position value SHALL follow the scaling from "Resolution Attribute".
    VerifyOrReturnError(target == mState.target,CHIP_NO_ERROR);
    
    mState.target = target;
    mMatterContext.MarkDirty(Attributes::Target::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetResolution(const Percent100ths resolution)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(resolution <= 10000,CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(resolution == mState.resolution,CHIP_NO_ERROR);
    
    mState.resolution = resolution;
    mMatterContext.MarkDirty(Attributes::Resolution::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetStepValue(const Percent100ths stepValue)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(stepValue <= 10000,CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(stepValue == mState.stepValue,CHIP_NO_ERROR);
    
    //TODO: The value of this attribute SHALL be equal to an integer multiple of the Resolution attribute.
    //TODO: The value of this attribute should be large enough to cause a visible change on the closure when the Step command is received.
    mState.stepValue = stepValue;
    mMatterContext.MarkDirty(Attributes::StepValue::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetUnit(const ClosureUnitEnum unit)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(unit == mState.unit,CHIP_NO_ERROR);

    mState.unit = unit;
    mMatterContext.MarkDirty(Attributes::Unit::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetUnitRange(Structs::UnitRangeStruct::Type & unitRange)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(((unitRange.min  == mState.unitRange.min) && (unitRange.max == mState.unitRange.max)) ,CHIP_NO_ERROR);
 
    mState.unitRange.min = unitRange.min;
    mState.unitRange.max = unitRange.max;
    mMatterContext.MarkDirty(Attributes::UnitRange::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kLimitation),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(((limitRange.min == mState.limitRange.min) && (limitRange.max == mState.limitRange.max)),CHIP_NO_ERROR);
    
    mState.limitRange.min = limitRange.min;
    mState.limitRange.max = limitRange.max;
    mMatterContext.MarkDirty(Attributes::LimitRange::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetTranslationDirection(const TranslationDirectionEnum translationDirection)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kTranslation),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(translationDirection == mState.translationDirection,CHIP_NO_ERROR);
    
    //TODO:  This attribute is not supposed to change once the installation is finalized.
     
    mState.translationDirection = translationDirection;
    mMatterContext.MarkDirty(Attributes::TranslationDirection::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetRotationAxis(const RotationAxisEnum rotationAxis)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kRotation),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(rotationAxis == mState.rotationAxis,CHIP_NO_ERROR);
    
    //TODO:  This attribute is not supposed to change once the installation is finalized.

    mState.rotationAxis = rotationAxis;
    mMatterContext.MarkDirty(Attributes::RotationAxis::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetOverflow(const OverflowEnum overflow)
 {
    VerifyOrReturnError((mConformance.HasFeature(Feature::kRotation) || mConformance.HasFeature(Feature::kMotionLatching)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     VerifyOrReturnError(mConformance.supportsOverflow, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(overflow == mState.overflow,CHIP_NO_ERROR);

    //TODO:  If the axis is centered, one part goes Outside and the other part goes Inside. In this case, this attribute SHALL use Top/Bottom/Left/Right Inside or Top/Bottom/Left/Right Outside enumerated value.
    
    mState.overflow = overflow;
    mMatterContext.MarkDirty(Attributes::Overflow::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::SetModulationType(const ModulationTypeEnum modulationType)
 {
    VerifyOrReturnError(mConformance.HasFeature(Feature::kModulation),CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(modulationType == mState.modulationType,CHIP_NO_ERROR);
    //TODO:  This attribute is not supposed to change once the installation is finalized.
    
    mState.modulationType = modulationType;
    mMatterContext.MarkDirty(Attributes::ModulationType::Id);
    return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetCurrent(GenericCurrentStruct & current)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     current = mState.current;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetTarget(GenericTargetStruct & target)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     target = mState.target;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetResolution(Percent100ths & resolution)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     resolution = mState.resolution;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetStepValue(Percent100ths & stepValue)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     stepValue = mState.stepValue;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetUnit(ClosureUnitEnum & unit)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     unit = mState.unit;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetUnitRange(DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kUnit), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     unitRange = mState.unitRange;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kLimitation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     limitRange = mState.limitRange;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetTranslationDirection(TranslationDirectionEnum & translationDirection)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kTranslation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     translationDirection = mState.translationDirection;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetRotationAxis(RotationAxisEnum & rotationAxis)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kRotation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     rotationAxis = mState.rotationAxis;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetOverflow(OverflowEnum & overflow)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError((mConformance.HasFeature(Feature::kRotation) || mConformance.HasFeature(Feature::kMotionLatching)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     VerifyOrReturnError(mConformance.supportsOverflow, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     overflow = mState.overflow;
     return CHIP_NO_ERROR;
 }
 
 CHIP_ERROR ClusterLogic::GetModulationType(ModulationTypeEnum & modulationType)
 {
     VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kModulation), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
     modulationType = mState.modulationType;
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
 
Status ClusterLogic::HandleSetTargetCommand(chip::Optional<chip::Percent100ths> position, chip::Optional<TargetLatchEnum> latch, chip::Optional<Globals::ThreeLevelAutoEnum> speed)
 {
     VerifyOrReturnError(mInitialized, Status::Failure);
     //TODO: If this command is sent while the device is in a non-compatible internal-state, a status code of INVALID_IN_STATE SHALL be returned.
     VerifyOrReturnError((position.Value() <= 10000), Status::ConstraintError);
     VerifyOrReturnError(EnsureKnownEnumValue(latch.Value()) != TargetLatchEnum::kUnknownEnumValue, Status::ConstraintError);
     VerifyOrReturnError(EnsureKnownEnumValue(speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
     
     
    if(position.HasValue()) {
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning),Status::Success);
        
        //TODO: not an integer multiple of the Resolution attribute, then Target.Position is updated to the closest integer multiple of the Resolution attribute according to the server, and a status code of SUCCESS is returned. 
        //TODO: supported then the device (depending of its current state) start or change the course of its motion procedure and update Target.Position.
    }
    
    if(latch.HasValue()) {
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching),Status::Success);
        
        //TODO: If the device supports the Latching(LT) feature, the device dimension SHALL either fulfill the latch order and update Target.Latch or, if manual intervention is required to latch, respond with INVALID_ACTION and remain in its current state.
    }
    
    if(speed.HasValue()) {
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed),Status::Success);
        
        //TODO: If Speed(SP) feature is enabled then Target.Speed is updated with the Speed Field.
        //TODO: In case a SetTarget command is sent with only the Speed field then if: It SHOULD affect the current dimension motion speed of the device (this change speed on the fly).
        
    }
     return Status::Success;;
 }
 
Status ClusterLogic::HandleStepCommand(StepDirectionEnum direction, uint16_t numberOfSteps, chip::Optional<Globals::ThreeLevelAutoEnum> speed)
 {
     VerifyOrReturnError(mInitialized, Status::Failure);
     VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning),Status::Failure);
     //TODO: If any of the mandatory fields are missing then a status code of INVALID_COMMAND SHALL be returned and the Target attribute value SHALL remain unchanged.
     VerifyOrReturnError(EnsureKnownEnumValue(direction) != StepDirectionEnum::kUnknownEnumValue, Status::ConstraintError);
     VerifyOrReturnError(EnsureKnownEnumValue(speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
     //TODO: If the server is in a state where it cannot support the command, the server SHALL respond with an INVALID_IN_STATE response and the Target attribute value SHALL remain unchanged.
     VerifyOrReturnError(numberOfSteps == 0,Status::Success);
     
     //TODO: , this SHALL update the Target.Position attribute value e.g. by sending multiple commands with short step by step or a single command with multiple steps.
     
     GenericTargetStruct step_target;
     GetTarget(step_target);
     
     //TODO: If the Direction field is set to Decrease, the Position field of the Target attribute SHALL be set to: Position = Position - NumberOfSteps * StepValue and the value SHALL be clamped to 0.00% if the LM feature is not supported or LimitRange.Min if the LM feature is supported.
     //TODO: If the Direction field is set to Increase, the Position field of the Target attribute SHALL be set to: Position = Position + NumberOfSteps * StepValue and the value SHALL be clamped to 100.00% if the LM feature is not supported or LimitRange.Max if the LM feature is supported.
     
     if(speed.HasValue()) {
        step_target.speed = speed;
     }
     
     //TODO: CHIP_ERROR err = mClusterDriver.HandleStep(direction,numberOfSteps,speed);
     SetTarget(step_target);
     
     return Status::Success;
 }
 } // namespace ClosureDimension
 } // namespace Clusters
 } // namespace app
 } // namespace chip
 