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

#include <app/AttributePathParams.h>
#include <app/EventPathParams.h>
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
 *  @brief The read client represents the initiator side of a Read Interaction, and is responsible
 *  for generating one Read Request for a particular set of attributes and/or events, and handling the Report Data response.
 *
 */
class ReadClient : public Messaging::ExchangeDelegate
{
public:
    /**
     *  Shut down the Client. This terminates this instance of the object and releases
     *  all held resources.  The object must not be used after Shutdown() is called.
     *
     *  SDK consumer can choose when to shut down the ReadClient.
     *  The ReadClient will automatically shut itself down when it receives a
     *  response or the response times out.  So manual shutdown is only needed
     *  to shut down a ReadClient before one of those two things has happened,
     *  (e.g. if SendReadRequest returned failure).
     */
    void Shutdown();

    /**
     *  Send a Read Request.  There can be one Read Request outstanding on a given ReadClient.
     *  If SendReadRequest returns success, no more Read Requests can be sent on this ReadClient
     *  until the corresponding InteractionModelDelegate::ReportProcessed or InteractionModelDelegate::ReportError
     *  call happens with guarantee.
     *
     *  @retval #others fail to send read request
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendReadRequest(NodeId aNodeId, Transport::AdminId aAdminId, SecureSessionHandle * aSecureSession,
                               EventPathParams * apEventPathParamsList, size_t aEventPathParamsListSize,
                               AttributePathParams * apAttributePathParamsList, size_t aAttributePathParamsListSize,
                               EventNumber aEventNumber);

    intptr_t GetAppIdentifier() const { return mAppIdentifier; }
    Messaging::ExchangeContext * GetExchangeContext() const { return mpExchangeCtx; }

private:
    friend class TestReadInteraction;
    friend class InteractionModelEngine;

    enum class ClientState
    {
        Uninitialized = 0, ///< The client has not been initialized
        Initialized,       ///< The client has been initialized and is ready for a SendReadRequest
        AwaitingResponse,  ///< The client has sent out the read request message
    };

    /**
     *  Initialize the client object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *  @param[in]    aAppState        Application defined object to distinguish different read requests.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE incorrect state if it is already initialized
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate, intptr_t aAppIdentifier);

    virtual ~ReadClient() = default;

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                 const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /**
     *  Check if current read client is being used
     *
     */
    bool IsFree() const { return mState == ClientState::Uninitialized; };

    CHIP_ERROR GenerateEventPathList(ReadRequest::Builder & aRequest, EventPathParams * apEventPathParamsList,
                                     size_t aEventPathParamsListSize, EventNumber & aEventNumber);
    CHIP_ERROR GenerateAttributePathList(ReadRequest::Builder & aRequest, AttributePathParams * apAttributePathParamsList,
                                         size_t aAttributePathParamsListSize);
    CHIP_ERROR ProcessAttributeDataList(TLV::TLVReader & aAttributeDataListReader);

    void MoveToState(const ClientState aTargetState);
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle && aPayload);
    CHIP_ERROR AbortExistingExchangeContext();
    const char * GetStateStr() const;

    /**
     * Internal shutdown method that we use when we know what's going on with
     * our exchange and don't need to manually close it.
     */
    void ShutdownInternal();

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;
    ClientState mState                         = ClientState::Uninitialized;
    intptr_t mAppIdentifier                    = 0;
};

}; // namespace app
}; // namespace chip
