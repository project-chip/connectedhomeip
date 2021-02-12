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
#include <app/InteractionModelDelegate.h>

namespace chip {
namespace app {
/* A structure representing a path parameters to a event */
struct EventPathParams
{
    EventPathParams(chip::NodeId aNodeId, chip::EndpointId aEndpointId, chip::ClusterId aClusterId, chip::EventId aEventId) : mNodeId(aNodeId), mEndpointId(aEndpointId), mClusterId(aClusterId), mEventId(aEventId) {}
    chip::NodeId mNodeId;
    chip::EndpointId mEndpointId;
    chip::ClusterId mClusterId;
    chip::EventId mEventId;
};

class ReadClient : public Messaging::ExchangeDelegate
{
public:
    enum ClientState
    {
        kState_Uninitialized = 0,  //< The client has not been initialized
        kState_Initialized,        //< The client has been initialized and is ready
        kState_ReadRequestSending, //< The client has sent out the read request message
    };

    /**
     * @brief Set delegate and pointer to associated state object for ReadClient specific call backs
     *
     * @param[in]  apDelegate  	A fInteractionModelDelegate for event call back
     */
    void SetDelegate(InteractionModelDelegate * apDelegate) { mpDelegate = apDelegate; };

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
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
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
     *  Reset the ReadHandler to initialized state
     *
     */
    void Reset();

    CHIP_ERROR SendReadRequest(NodeId aNodeId, Transport::AdminId aAdminId);

    virtual ~ReadClient() = default;

    bool IsFree() const { return (nullptr == mpExchangeCtx); };

private:
    friend class InteractionModelEngine;

    void MoveToState(const ClientState aTargetState);
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ClearExistingExchangeContext();
    void ClearState();
    const char * GetStateStr() const;

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    InteractionModelDelegate * mpDelegate = nullptr;
    ClientState mState;
    bool mMoreChunkedMessages;
};

}; // namespace app
}; // namespace chip

