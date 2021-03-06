/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      This file defines read client for a CHIP Interaction Data model
 *
 */

#pragma once

#include <app/InteractionModelDelegate.h>
#include <app/MessageDef/ReadRequest.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
/**
 *  @class ReadClient
 *
 *  @brief The read client represent the initiator side of a Read Interaction or Read Transaction, and is responsible
 *  for generating one read request with particular path on attributes and/or events, and expect to receive report data if any.
 *
 */
class ReadClient : public Messaging::ExchangeDelegate
{
public:
    void OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /**
     *  Initialize the client object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE incorrect state if it is already initialized
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate);

    /**
     *  Shutdown the Client. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();

    /**
     *  Send Read Request
     *
     *  @param[in]    aNodeId    Node Id
     *  @param[in]    aAdminId   Admin ID
     *
     *  @retval #others fail to send read request
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendReadRequest(NodeId aNodeId, Transport::AdminId aAdminId);

    virtual ~ReadClient() = default;

    /**
     *  Check if current read client is being used
     *
     */
    bool IsFree() const { return mState == ClientState::Uninitialized; };

private:
    friend class TestReadInteraction;

    enum class ClientState
    {
        Uninitialized = 0, //< The client has not been initialized
        Initialized,       //< The client has been initialized and is ready
        WaitingResponse,   //< The client has sent out the read request message
    };

    void MoveToState(const ClientState aTargetState);
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle aPayload);
    CHIP_ERROR ClearExistingExchangeContext();
    const char * GetStateStr() const;

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;
    ClientState mState                         = ClientState::Uninitialized;
};

}; // namespace app
}; // namespace chip
