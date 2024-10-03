/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

class ChimeDelegate;

class ChimeServer : private AttributeAccessInterface, private CommandHandlerInterface
{
public:
    ChimeServer(EndpointId endpointId, ChimeDelegate & delegate);
    ~ChimeServer();

    CHIP_ERROR Init();

private:
    ChimeDelegate & mDelegate;

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
    CHIP_ERROR SetActiveChimeId(uint8_t chimeSoundId);
    CHIP_ERROR SetEnabled(bool enabled);

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    void HandlePlayChimeSound(HandlerContext & ctx, const Chime::Commands::PlayChimeSound::DecodableType & req);
};

/** @brief
 *  Defines methods for implementing application-specific logic for the Chime Cluster.
 */
class ChimeDelegate
{
public:
    ChimeDelegate() = default;

    virtual ~ChimeDelegate() = default;

    // Get Attribute Methods

    /**
     *   Get the chime sounds.
     *   @param index The index of the chime sound to be returned. It is assumed that chime sounds are indexable from 0 and with no
     * gaps.
     *   @param chimeSound A reference to receive the chime sound struct on success.
     *   @return Returns a CHIP_NO_ERROR if there was no error and the chime sound was returned successfully,
     *   CHIP_ERROR_NOT_FOUND if the index in beyond the list of available chime sounds.
     */
    virtual CHIP_ERROR GetChimeSoundByIndex(uint8_t index, Chime::Structs::ChimeSoundStruct::Type & chimeSound) = 0;
    virtual uint8_t GetActiveChimeId()                                                                          = 0;
    virtual bool GetEnabled()                                                                                   = 0;

    // Set Attribute Methods
    virtual CHIP_ERROR SetActiveChimeId(uint8_t chimeSoundId) = 0;
    virtual CHIP_ERROR SetEnabled(bool enabled)               = 0;

    // Commands
    /**
     * @brief Delegate should implement a handler to play the currently active chime sound.
     * It should report Status::Success if successful and may
     * return other Status codes if it fails
     */
    virtual Protocols::InteractionModel::Status playChimeSound() = 0;

private:
    friend class ChimeServer;

    ChimeServer * mChimeServer = nullptr;

    void SetChimeServer(ChimeServer * chimeServer) { mChimeServer = chimeServer; }

protected:
    ChimeServer * GetChimeServer() const { return mChimeServer; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
