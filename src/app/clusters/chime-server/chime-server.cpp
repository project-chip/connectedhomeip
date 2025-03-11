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
#include <app/SafeAttributePersistenceProvider.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::Chime::Attributes;
using chip::Protocols::InteractionModel::Status;
using ChimeSoundStructType = Structs::ChimeSoundStruct::Type;

namespace chip {
namespace app {
namespace Clusters {

ChimeServer::ChimeServer(EndpointId endpointId, ChimeDelegate & delegate) :
    AttributeAccessInterface(MakeOptional(endpointId), Chime::Id), CommandHandlerInterface(MakeOptional(endpointId), Chime::Id),
    mDelegate(delegate), mSelectedChime(0), mEnabled(true)
{
    mDelegate.SetChimeServer(this);
}

ChimeServer::~ChimeServer()
{
    // null out the ref to us on the delegate
    mDelegate.SetChimeServer(nullptr);

    // unregister
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR ChimeServer::Init()
{
    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

void ChimeServer::LoadPersistentAttributes()
{
    // Load Active Chime ID
    uint8_t storedSelectedChime = 0;
    CHIP_ERROR err              = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(GetEndpointId(), Chime::Id, SelectedChime::Id), storedSelectedChime);
    if (err == CHIP_NO_ERROR)
    {
        mSelectedChime = storedSelectedChime;
    }
    else
    {
        // otherwise defaults
        ChipLogDetail(Zcl, "Chime: Unable to load the SelectedChime attribute from the KVS. Defaulting to %u", mSelectedChime);
    }

    // Load Enabled
    bool storedEnabled = false;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(ConcreteAttributePath(GetEndpointId(), Chime::Id, Enabled::Id),
                                                                 storedEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mEnabled = storedEnabled;
    }
    else
    {
        // otherwise take the default
        ChipLogDetail(Zcl, "Chime: Unable to load the Enabled attribute from the KVS. Defaulting to %u", mEnabled);
    }
}

// AttributeAccessInterface
CHIP_ERROR ChimeServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Chime::Id);

    switch (aPath.mAttributeId)
    {
    case SelectedChime::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mSelectedChime));
        break;
    case Enabled::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mEnabled));
        break;
    case InstalledChimeSounds::Id:
        ChimeServer * cs = this;
        CHIP_ERROR err =
            aEncoder.EncodeList([cs](const auto & encoder) -> CHIP_ERROR { return cs->EncodeSupportedChimeSounds(encoder); });
        return err;
    }

    return CHIP_NO_ERROR;
}

uint8_t ChimeServer::GetSelectedChime() const
{
    return mSelectedChime;
}

bool ChimeServer::GetEnabled() const
{
    return mEnabled;
}

// helper method to get the Chime Sounds one by one and encode into a list
CHIP_ERROR ChimeServer::EncodeSupportedChimeSounds(const AttributeValueEncoder::ListEncodeHelper & encoder)
{

    for (uint8_t i = 0; true; i++)
    {
        ChimeSoundStructType chimeSound;

        // Get the chime sound
        // We pass in a MutableCharSpan to avoid any ownership issues - Delegate needs to use
        // CopyCharSpanToMutableCharSpan to copy data in
        char buffer[kMaxChimeSoundNameSize];
        MutableCharSpan name(buffer);
        auto err = mDelegate.GetChimeSoundByIndex(i, chimeSound.chimeID, name);

        // return if we've run off the end of the Chime Sound List on the delegate
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(err);

        // set the name on the struct
        chimeSound.name = name;

        // and now encode the struct
        ReturnErrorOnFailure(encoder.Encode(chimeSound));
    }
    return CHIP_NO_ERROR;
}

// helper method to check if the chimeID param is supported by the delegate
bool ChimeServer::IsSupportedChimeID(uint8_t chimeID)
{
    uint8_t supportedChimeID;
    for (uint8_t i = 0; mDelegate.GetChimeIDByIndex(i, supportedChimeID) != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; i++)
    {
        if (supportedChimeID == chimeID)
        {
            return true;
        }
    }

    ChipLogDetail(Zcl, "Cannot find a supported ChimeID with value %u", chimeID);
    return false;
}

CHIP_ERROR ChimeServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == Chime::Id);
    Status status;

    switch (aPath.mAttributeId)
    {
    case SelectedChime::Id: {
        uint8_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        status = SetSelectedChime(newValue);
        return StatusIB(status).ToChipError();
    }
    case Enabled::Id: {
        bool newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        status = SetEnabled(newValue);
        return StatusIB(status).ToChipError();
    }

    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

Status ChimeServer::SetSelectedChime(uint8_t chimeID)
{
    if (!IsSupportedChimeID(chimeID))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    bool activeIDChanged = !(mSelectedChime == chimeID);
    if (activeIDChanged)
    {
        mSelectedChime = chimeID;

        // Write new value to persistent storage.
        auto endpointId            = GetEndpointId();
        ConcreteAttributePath path = ConcreteAttributePath(endpointId, Chime::Id, SelectedChime::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSelectedChime);

        // and mark as dirty
        MatterReportingAttributeChangeCallback(path);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status ChimeServer::SetEnabled(bool Enabled)
{
    bool enableChanged = !(mEnabled == Enabled);

    if (enableChanged)
    {
        mEnabled = Enabled;

        // Write new value to persistent storage.
        auto endpointId            = GetEndpointId();
        ConcreteAttributePath path = ConcreteAttributePath(endpointId, Chime::Id, Enabled::Id);
        GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mEnabled);

        // and mark as dirty
        MatterReportingAttributeChangeCallback(path);
    }

    return Protocols::InteractionModel::Status::Success;
}

void ChimeServer::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::PlayChimeSound::Id:
        CommandHandlerInterface::HandleCommand<Commands::PlayChimeSound::DecodableType>(
            ctx, [this](HandlerContext & innerCtx, const auto & req) { HandlePlayChimeSound(innerCtx, req); });
        break;
    }
}

void ChimeServer::HandlePlayChimeSound(HandlerContext & ctx, const Commands::PlayChimeSound::DecodableType & req)
{

    ChipLogDetail(Zcl, "Chime: PlayChimeSound");

    // call the delegate to play the chime
    Status status = mDelegate.PlayChimeSound();
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
void MatterChimePluginServerInitCallback() {}
