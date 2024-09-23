/*
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
 */

/****************************************************************************'
 * @file
 * @brief Implementation for the Chime Server Cluster
 ***************************************************************************/


#include "chime-server.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::Chime::Attributes;
using chip::Protocols::InteractionModel::Status;


namespace chip {
namespace app {
namespace Clusters {

ChimeServer::ChimeServer(EndpointId endpointId, ChimeDelegate & delegate) :
    AttributeAccessInterface(MakeOptional(endpointId), Chime::Id),
    CommandHandlerInterface(MakeOptional(endpointId), Chime::Id), mDelegate(delegate)
{
    mDelegate.SetChimeServer(this);
}

ChimeServer::~ChimeServer()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
}


CHIP_ERROR ChimeServer::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR ChimeServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Chime::Id);
    
    switch (aPath.mAttributeId)
    {
    case InstalledChimeSounds::Id:
        return aEncoder.Encode(mDelegate.GetInstalledChimeSounds());

    case ActiveChimeSoundId::Id:
        return aEncoder.Encode(mDelegate.GetActiveChimeSoundId());

    case Enabled::Id:
        return aEncoder.Encode(mDelegate.GetEnabled());
    }
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChimeServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == Chime::Id);

    switch (aPath.mAttributeId)
    {
    case ActiveChimeSoundId::Id: {
        uint8_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        ReturnErrorOnFailure(SetActiveChimeSoundId(newValue));
        return CHIP_NO_ERROR;

    }
    case Enabled::Id: {
        bool newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        ReturnErrorOnFailure(mDelegate.SetEnabled(newValue));
        return CHIP_NO_ERROR;
    }
        
    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

}

CHIP_ERROR ChimeServer::SetActiveChimeSoundId(uint8_t soundId)
{
    uint8_t currentSoundId = mDelegate.GetActiveChimeSoundId();
    bool activeSoundIdChanged = !(currentSoundId == soundId);

    VerifyOrReturnError(activeSoundIdChanged, CHIP_NO_ERROR);
    VerifyOrDie(mDelegate.SetActiveChimeSoundId(soundId) == CHIP_NO_ERROR);
    MatterReportingAttributeChangeCallback(GetEndpointId(), Chime::Id, ActiveChimeSoundId::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChimeServer::SetEnabled(bool enabled)
{
    bool currentlyEnabled = mDelegate.GetEnabled();
    bool enableChanged = !(currentlyEnabled == enabled);

    VerifyOrReturnError(enableChanged, CHIP_NO_ERROR);
    VerifyOrDie(mDelegate.SetEnabled(enabled) == CHIP_NO_ERROR);
    MatterReportingAttributeChangeCallback(GetEndpointId(), Chime::Id, Enabled ::Id);

    return CHIP_NO_ERROR;
}

void ChimeServer::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::PlayChimeSound::Id:
        CommandHandlerInterface::HandleCommand<Commands::PlayChimeSound::DecodableType>(
            ctx, [this](HandlerContext & ctx, const auto & req) { HandlePlayChimeSound(ctx, req); });
        break;
    }
}

void ChimeServer::HandlePlayChimeSound(HandlerContext & ctx, const Commands::PlayChimeSound::DecodableType & req)
{

    ChipLogDetail(Zcl, "Chime: PlayChimeSound");

    // call the delegate to play the chime 
    Status status = mDelegate.playChimeSound();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Chime Cluster Server Init
 *
 * Server Init
 *
 */
void MatterChimePluginServerInitCallback(){}
