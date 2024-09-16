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

#include "valve-configuration-and-control-server-disco.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster-logic.h>
#include <app/data-model/Encode.h>
#include <app/util/config.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

using namespace Attributes;
using namespace Commands;
using namespace Protocols::InteractionModel;
namespace {

template <typename T, typename F>
CHIP_ERROR EncodeRead(AttributeValueEncoder & aEncoder, const F & getter)
{
    T ret;
    CHIP_ERROR err = getter(ret);
    if (err == CHIP_NO_ERROR)
    {
        err = aEncoder.Encode(ret);
    }

    // TODO: Should the logic return these directly? I didn't want to mix the IM layer into there, but this is annoying.
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
    if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        // what actually gets returned here? This is really an internal error, so failure seems perhaps correct.
        return CHIP_IM_GLOBAL_STATUS(Failure);
    }
    return err;
}

} // namespace

CHIP_ERROR Interface::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case OpenDuration::Id: {
        typedef OpenDuration::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetOpenDuration(ret); });
    }
    case DefaultOpenDuration::Id: {
        typedef DefaultOpenDuration::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder,
                             [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetDefaultOpenDuration(ret); });
    }
    case AutoCloseTime::Id: {
        typedef AutoCloseTime::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetAutoCloseTime(ret); });
    }
    case RemainingDuration::Id: {
        typedef RemainingDuration::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetRemainingDuration(ret); });
    }
    case CurrentState::Id: {
        typedef CurrentState::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetCurrentState(ret); });
    }
    case TargetState::Id: {
        typedef TargetState::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetTargetState(ret); });
    }
    case CurrentLevel::Id: {
        typedef CurrentLevel::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetCurrentLevel(ret); });
    }
    case TargetLevel::Id: {
        typedef TargetLevel::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetTargetLevel(ret); });
    }
    case DefaultOpenLevel::Id: {
        typedef DefaultOpenLevel::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetDefaultOpenLevel(ret); });
    }
    case ValveFault::Id: {
        typedef ValveFault::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetValveFault(ret); });
    }
    case LevelStep::Id: {
        typedef LevelStep::TypeInfo::Type T;
        return EncodeRead<T>(aEncoder, [&logic = mClusterLogic](T & ret) -> CHIP_ERROR { return logic.GetLevelStep(ret); });
    }
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

CHIP_ERROR Interface::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case DefaultOpenDuration::Id: {
        DefaultOpenDuration::TypeInfo::Type val;
        ReturnErrorOnFailure(aDecoder.Decode(val));
        return mClusterLogic.SetDefaultOpenDuration(val);
    }
    case DefaultOpenLevel::Id: {
        DefaultOpenLevel::TypeInfo::Type val;
        ReturnErrorOnFailure(aDecoder.Decode(val));
        return mClusterLogic.SetDefaultOpenLevel(val);
    }
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedWrite);
    }
}

// CommandHandlerInterface
void Interface::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Open::Id:
        HandleCommand<Open::DecodableType>(
            handlerContext, [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                // TODO: I used optional in the lower layers because I think we want to move to std::optional in general
                // So here, I need to change over. But I can also change the Logic cluster to use Optional
                CHIP_ERROR err =
                    logic.HandleOpenCommand(commandData.openDuration.std_optional(), commandData.targetLevel.std_optional());
                Status status = Status::Success;
                if (err == CHIP_ERROR_INVALID_ARGUMENT)
                {
                    status = Status::ConstraintError;
                }
                if (err != CHIP_NO_ERROR)
                {
                    status = Status::Failure;
                }
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
            });
        return;
    case Close::Id:
        HandleCommand<Close::DecodableType>(handlerContext,
                                            [&logic = mClusterLogic](HandlerContext & ctx, const auto & commandData) {
                                                CHIP_ERROR err = logic.HandleCloseCommand();
                                                Status status  = Status::Success;
                                                if (err == CHIP_ERROR_INVALID_ARGUMENT)
                                                {
                                                    status = Status::ConstraintError;
                                                }
                                                if (err != CHIP_NO_ERROR)
                                                {
                                                    status = Status::Failure;
                                                }
                                                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
                                            });
        return;
    }
}

CHIP_ERROR Interface::Init()
{
    AttributeAccessInterfaceRegistry::Instance().Register(this);
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this);
    return CHIP_NO_ERROR;
}

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
