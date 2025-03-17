/*
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

#include "closure-dimension-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureDimension;
using namespace chip::app::Clusters::ClosureDimension::Attributes;
using namespace chip::app::Clusters::ClosureDimension::Commands;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(Feature aFeatures) const
{
    return mFeatures.Has(aFeatures);
}

CHIP_ERROR Instance::SetCurrent(GenericCurrentStruct & aCurrent)
{
    // If the Current has changed, trigger the attribute change callback
    if (!(mCurrent == aCurrent))
    {
        mCurrent = aCurrent;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::Current::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetTarget(GenericTargetStruct & aTarget)
{
    // If the Target has changed, trigger the attribute change callback
    if (!(mTarget == aTarget))
    {
        mTarget = aTarget;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::Target::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetResolution(const Percent100ths aResolution)
{
    // If the Resolution has changed, trigger the attribute change callback
    if (mResolution != aResolution)
    {
        mResolution = aResolution;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::Resolution::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetStepValue(const Percent100ths aStepValue)
{
    // If the StepValue has changed, trigger the attribute change callback
    if (mStepValue != aStepValue)
    {
        mStepValue = aStepValue;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::StepValue::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetUnit(const ClosureUnitEnum aUnit)
{
    // If the Unit has changed, trigger the attribute change callback
    if (mUnit != aUnit)
    {
        mUnit = aUnit;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::Unit::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetUnitRange(Structs::UnitRangeStruct::Type & aUnitRange)
{
    // If the UnitRange has changed, trigger the attribute change callback
    if ((mUnitRange.min != aUnitRange.min) || (mUnitRange.max != aUnitRange.max))
    {
        mUnitRange = aUnitRange;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::UnitRange::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetLimitRange(Structs::RangePercent100thsStruct::Type & aLimitRange)
{
    // If the LimitRange has changed, trigger the attribute change callback
    if ((mLimitRange.min != aLimitRange.min) || (mLimitRange.max != aLimitRange.max))
    {
        mLimitRange = aLimitRange;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::LimitRange::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetTranslationDirection(const TranslationDirectionEnum aTranslationDirection)
{
    // If the TranslationDIrection has changed, trigger the attribute change callback
    if (mTranslationDirection != aTranslationDirection)
    {
        mTranslationDirection = aTranslationDirection;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::TranslationDirection::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetRotationAxis(const RotationAxisEnum aRotationAxis)
{
    // If the RotationAxis has changed, trigger the attribute change callback
    if (mRotationAxis != aRotationAxis)
    {
        mRotationAxis = aRotationAxis;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::RotationAxis::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetOverflow(const OverflowEnum aOverflow)
{
    // If the Overflow has changed, trigger the attribute change callback
    if (mOverflow != aOverflow)
    {
        mOverflow = aOverflow;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::Overflow::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::SetModulationType(const ModulationTypeEnum aModulationType)
{
    // If the ModulationType has changed, trigger the attribute change callback
    if (mModulationType != aModulationType)
    {
        mModulationType = aModulationType;
        MatterReportingAttributeChangeCallback(mDelegate.GetEndpointId(), ClosureDimension::Id, Attributes::ModulationType::Id);
    }
    return CHIP_NO_ERROR;
}

const GenericCurrentStruct & Instance::GetCurrent() const
{
    return mCurrent;
}

const GenericTargetStruct & Instance::GetTarget() const
{
    return mTarget;
}

Percent100ths Instance::GetResolution() const
{
    return mResolution;
}

Percent100ths Instance::GetStepValue() const
{
    return mStepValue;
}

ClosureUnitEnum Instance::GetUnit() const
{
    return mUnit;
}

const Structs::UnitRangeStruct::Type & Instance::GetUnitRange() const
{
    return mUnitRange;
}

const Structs::RangePercent100thsStruct::Type & Instance::GetLimitRange() const
{
    return mLimitRange;
}

TranslationDirectionEnum Instance::GetTranslationDirection() const
{
    return mTranslationDirection;
}

RotationAxisEnum Instance::GetRotationAxis() const
{
    return mRotationAxis;
}

OverflowEnum Instance::GetOverflow() const
{
    return mOverflow;
}

ModulationTypeEnum Instance::GetModulationType() const
{
    return mModulationType;
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == ClosureDimension::Id);

    switch (aPath.mAttributeId)
    {
    case Current::Id:
        return aEncoder.Encode(GetCurrent());

    case Target::Id:
        return aEncoder.Encode(GetTarget());

    case Resolution::Id:
        if (HasFeature(Feature::kPositioning))
        {
            return aEncoder.Encode(GetResolution());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    case StepValue::Id:
        if (HasFeature(Feature::kPositioning))
        {
            return aEncoder.Encode(GetStepValue());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    case Unit::Id:
        if (HasFeature(Feature::kUnit))
        {
            return aEncoder.Encode(GetUnit());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    
    case UnitRange::Id:
        if (HasFeature(Feature::kUnit))
        {
            return aEncoder.Encode(GetUnitRange());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    
    case LimitRange::Id:
        if (HasFeature(Feature::kLimitation))
        {
            return aEncoder.Encode(GetLimitRange());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        
    case TranslationDirection::Id:
        if (HasFeature(Feature::kTranslation))
        {
            return aEncoder.Encode(GetTranslationDirection());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        
    case RotationAxis::Id:
        if (HasFeature(Feature::kRotation))
        {
            return aEncoder.Encode(GetRotationAxis());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        
    case Overflow::Id:
        if (HasFeature(Feature::kRotation))
        {
            return aEncoder.Encode(GetOverflow());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        
    case ModulationType::Id:
        if (HasFeature(Feature::kModulation))
        {
            return aEncoder.Encode(GetModulationType());
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeatures);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == ClosureDimension::Id);

    switch (aPath.mAttributeId)
    {
    case Current::Id:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        
    case Target::Id:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);

    case Resolution::Id:
        if (!(HasFeature(Feature::kPositioning)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);

    case StepValue::Id:
        if (!(HasFeature(Feature::kPositioning)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);

    case Unit::Id:
        if (!(HasFeature(Feature::kUnit)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    
    case UnitRange::Id:
        if (!(HasFeature(Feature::kUnit)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    
    case LimitRange::Id:
        if (!(HasFeature(Feature::kLimitation)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        
    case TranslationDirection::Id:
        if (!(HasFeature(Feature::kTranslation)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        
    case RotationAxis::Id:
        if (!(HasFeature(Feature::kRotation)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        
    case Overflow::Id:
        if (!(HasFeature(Feature::kRotation)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
        
    case ModulationType::Id:
        if (!(HasFeature(Feature::kModulation)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);

    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

// CommandHandlerInterface
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case SetTarget::Id:
        HandleCommand<SetTarget::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleSetTarget(ctx, commandData); });
        break;
    case Step::Id:
        if (HasFeature(Feature::kPositioning))
        {
            HandleCommand<Step::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleStep(ctx, commandData); });
        }
        else
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        break;
    }
}

void Instance::HandleSetTarget(HandlerContext & ctx, const Commands::SetTarget::DecodableType & commandData)
{
    Status status = mDelegate.SetTarget(commandData.position,commandData.latch,commandData.speed);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleStep(HandlerContext & ctx, const Commands::Step::DecodableType & commandData)
{
    Status status = mDelegate.Step(commandData.direction, commandData.numberOfSteps, commandData.speed);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterClosureDimensionPluginServerInitCallback() {}
