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

#include "closure-control-server.h"

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
using namespace chip::app::Clusters::ClosureControl;

using chip::Protocols::InteractionModel::Status;

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

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

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

CHIP_ERROR Interface::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::CountdownTime::Id: {
        typedef Attributes::CountdownTime::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR {
            // TODO: Use the correct getter
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        });
    }

    case Attributes::CurrentErrorList::Id: {

        typedef Attributes::CurrentErrorList::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR {
            // TODO: Use the correct getter
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        });
    }

    case Attributes::OverallState::Id: {
        typedef Attributes::OverallState::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR {
            // TODO: Use the correct getter
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        });
    }

    case Attributes::OverallTarget::Id: {
        typedef Attributes::OverallTarget::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR {
            // TODO: Use the correct getter
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        });
    }

    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

void Interface::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::Stop::Id:
        HandleCommand<Commands::SetTarget::DecodableType>(
            handlerContext, [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                // TODO: Call cluster logic
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
            });
        return;

    case Commands::MoveTo::Id:
        HandleCommand<Commands::Step::DecodableType>(
            handlerContext, [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                // TODO: Call cluster logic
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
            });
        return;
    case Commands::Calibrate::Id:
        HandleCommand<Commands::Step::DecodableType>(
            handlerContext, [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                // TODO: Call cluster logic
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
            });
        return;

    default:
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        return;
    }
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterClosureControlPluginServerInitCallback() {}
void MatterClosureControlPluginServerShutdownCallback() {}
