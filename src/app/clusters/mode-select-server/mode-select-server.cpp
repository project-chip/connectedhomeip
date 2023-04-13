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

#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
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
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Init yes yes!!");
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    // ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    ReturnErrorOnFailure(msDelegate->Init());
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & ctxt)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: InvokeCommand yes yes!!");
    ctxt.mCommandHandled = true;
//    switch (ctxt.mRequestPath.mCommandId)
//    {
//    case Commands::ModeSelect::ChangeToMode:
//        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Entering handling ChangeToMode");
//        EndpointId endpointId = ctxt.mEndpointId;
//        uint8_t newMode       = ctxt.newMode;
//        // Check that the newMode matches one of the supported options
//        const ModeSelect::Structs::ModeOptionStruct::Type * modeOptionPtr;
//        Status checkSupportedModeStatus = this::msDelegate->getModeOptionByMode(endpointId, newMode, &modeOptionPtr);
//        if (Status::Success != checkSupportedModeStatus)
//        {
//            emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: Failed to find the option with mode %u", newMode);
//            // ctcx.mCommandHandler->AddStatus(ctcx.mPath, checkSupportedModeStatus);
//            return;
//        }
//        ModeSelect::Attributes::CurrentMode::Set(endpointId, newMode);
//
//        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "ModeSelect: ChangeToMode successful");
//        // commandHandler->AddStatus(commandPath, Status::Success);
//        return;
//    }
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
