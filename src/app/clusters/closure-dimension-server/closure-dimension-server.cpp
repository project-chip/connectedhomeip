/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *
 */

#include "closure-dimension-server.h"
#include "closure-dimension-cluster-logic.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

using namespace Protocols::InteractionModel;

namespace {

template <typename T, typename F>
CHIP_ERROR EncodeRead(AttributeValueEncoder & aEncoder, const F & getter)
{
    T ret;
    CHIP_ERROR err = getter(ret);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, err);
    return aEncoder.Encode(ret);
}

} // namespace

CHIP_ERROR Interface::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::CurrentState::Id: {
        typedef DataModel::Nullable<GenericDimensionStateStruct> T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetCurrentState(ret); });
    }
    case Attributes::TargetState::Id: {
        typedef DataModel::Nullable<GenericDimensionStateStruct> T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetTargetState(ret); });
    }
    case Attributes::Resolution::Id: {
        typedef Attributes::Resolution::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetResolution(ret); });
    }
    case Attributes::StepValue::Id: {
        typedef Attributes::StepValue::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetStepValue(ret); });
    }
    case Attributes::Unit::Id: {
        typedef Attributes::Unit::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetUnit(ret); });
    }
    case Attributes::UnitRange::Id: {
        typedef Attributes::UnitRange::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetUnitRange(ret); });
    }
    case Attributes::LimitRange::Id: {
        typedef Attributes::LimitRange::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetLimitRange(ret); });
    }
    case Attributes::TranslationDirection::Id: {
        typedef Attributes::TranslationDirection::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder,
                             [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetTranslationDirection(ret); });
    }
    case Attributes::RotationAxis::Id: {
        typedef Attributes::RotationAxis::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetRotationAxis(ret); });
    }
    case Attributes::Overflow::Id: {
        typedef Attributes::Overflow::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetOverflow(ret); });
    }
    case Attributes::ModulationType::Id: {
        typedef Attributes::ModulationType::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetModulationType(ret); });
    }
    case Attributes::LatchControlModes::Id: {
        typedef BitFlags<LatchControlModesBitmap> T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetLatchControlModes(ret); });
    }
    case Attributes::FeatureMap::Id: {
        typedef BitFlags<Feature> T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetFeatureMap(ret); });
    }
    case Attributes::ClusterRevision::Id: {
        typedef Attributes::ClusterRevision::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetClusterRevision(ret); });
    }
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

void Interface::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SetTarget::Id:
        HandleCommand<Commands::SetTarget::DecodableType>(
            handlerContext, [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                Status status = logic.HandleSetTargetCommand(commandData.position, commandData.latch, commandData.speed);
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
            });
        return;
    case Commands::Step::Id:
        HandleCommand<Commands::Step::DecodableType>(
            handlerContext, [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                Status status = logic.HandleStepCommand(commandData.direction, commandData.numberOfSteps, commandData.speed);
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
            });
        return;
    default:
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        return;
    }
}

CHIP_ERROR Interface::Init()
{
    VerifyOrDieWithMsg(AttributeAccessInterfaceRegistry::Instance().Register(this), NotSpecified,
                       "Failed to register attribute access");
    VerifyOrDieWithMsg(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this) == CHIP_NO_ERROR, NotSpecified,
                       "Failed to register command handler");

    return CHIP_NO_ERROR;
}

CHIP_ERROR Interface::Shutdown()
{
    VerifyOrDieWithMsg(CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this) == CHIP_NO_ERROR, NotSpecified,
                       "Failed to unregister command handler");
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);

    return CHIP_NO_ERROR;
}

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterClosureDimensionPluginServerInitCallback() {}
