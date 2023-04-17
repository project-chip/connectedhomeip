/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <app/util/af.h>
#include <app/clusters/mode-select-server/mode-select-server.h>

// using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeSelect;


namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    // ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    ReturnErrorOnFailure(msDelegate->Init());
    return CHIP_NO_ERROR;
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    // todo
    // if !RequestT::IsValidClusterId(handlerContext.mRequestPath.mClusterId)
    // {
    //     handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::UnsupportedCluster);
    //     return;        
    // }

    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

        //
        // If the command matches what the caller is looking for, let's mark this as being handled
        // even if errors happen after this. This ensures that we don't execute any fall-back strategies
        // to handle this command since at this point, the caller is taking responsibility for handling
        // the command in its entirety, warts and all.
        //
        handlerContext.SetCommandHandled();

        if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                        Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        func(handlerContext, requestPayload);
    }
}



EmberAfStatus Instance::SetCurrentMode(uint8_t newMode)
{
    switch (clusterId)
    {
    case ModeSelect::Id:
        return ModeSelect::Attributes::CurrentMode::Set(endpointId, newMode);
    default:
        return EMBER_ZCL_STATUS_CONSTRAINT_ERROR; // todo is this the correct error? Is there a "not implemented error"?
    }
}

void Instance::ModeSelectHandleChangeToMode(HandlerContext & ctx, const Commands::ChangeToMode::DecodableType & commandData) 
{
    uint8_t newMode = commandData.newMode;

    Status checkSupportedModeStatus = msDelegate->IsSupportedMode(newMode);

    if (Status::Success != checkSupportedModeStatus)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Failed to find the option with mode %u", newMode);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, checkSupportedModeStatus);
        return;
    }

    SetCurrentMode(newMode);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: ChangeToMode successful");
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case ModeSelect::Commands::ChangeToMode::Id:
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Entering handling ChangeToMode");

        HandleCommand<Commands::ChangeToMode::DecodableType>(handlerContext, [this](HandlerContext & ctx, const auto & commandData) { ModeSelectHandleChangeToMode(ctx, commandData); });
    }
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    return CHIP_NO_ERROR;
}

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterModeSelectPluginServerInitCallback()
{
    // Nothing to do, the server init routine will be done in Instance::Init()
}
