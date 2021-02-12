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

#ifndef _CHIP_INTERACTION_MODEL_READ_CLIENT_H
#define _CHIP_INTERACTION_MODEL_READ_CLIENT_H

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
/* A structure representing a path parameters to a event */
struct EventPathParams
{
    EventPathParams() {}
    EventPathParams(chip::EndpointId aEndpointId, chip::ClusterId aClusterId) : mEndpointId(aEndpointId), mClusterId(aClusterId) {}
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
        kState_Uninitialized = 0,  ///< The client has not been initialized
        kState_Initialized,        ///< The client has been initialized and is ready
        kState_ReadRequestSending, ///< The client has sent out the read request message
    };

    enum EventID
    {
        kEvent_OnReadRequestPrepareNeeded = 1,

        /**
         * Generated on a receipt of a ReportDataRequest.
         */
        kEvent_OnReportDataRequest = 2,

        /**
         * Generated when InteractionModelEngine finishes processing a
         * ReportDataRequest message.
         */
        kEvent_OnReportProcessed = 3,

        /**
         * Generated when the InteractionModelEngine encounters an event stream
         * within the ReportDataRequest message.
         */
        kEvent_OnEventStreamReceived = 4,
    };

    // union of structures for each event some of them might be empty
    union InEventParam
    {
        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            ReadClient * mpClient;
        } mReadRequestPrepareNeeded;

        struct
        {
            ReadClient * mpClient;
            // Do not close the EC
            Messaging::ExchangeContext * mpEC;

            // Do not modify the message content
            const PacketHeader * mpPacketHeader; ///< A pointer to the message information for the report data request
        } mReportDataRequest;

        struct
        {
            ReadClient * mpClient;
        } mReportProcessed;

        struct
        {
            chip::TLV::TLVReader * mpReader;
            const PacketHeader * mpPacketHeader; ///< A pointer to the message information for the command request
            ReadClient * mpClient;
        } mEventStreamReceived;
    };

    union OutEventParam
    {
        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            EventPathParams * mpEventPathParamsList; ///< Pointer to a list of event path parameter
            size_t mEventPathParamsListSize;         ///< Number of event paths in mpEventPathList
            uint64_t mEventNumber;                   ///< A event number it has already
        } mReadRequestPrepareNeeded;
    };

    /**
     * @brief Set the event back function and pointer to associated state object for SubscriptionEngine specific call backs
     *
     * @param[in]  apAppState    A pointer to application layer supplied state object
     * @param[in]  aEvent       A function pointer for event call back
     * @param[in]  aInParam     A const reference to the input parameter for this event
     * @param[out] aOutParam    A reference to the output parameter for this event
     */
    typedef void (*EventCallback)(void * apAppState, EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam);

    /**
     * @brief Set the event back function and pointer to associated state object for SubscriptionEngine specific call backs
     *
     * @param[in]  apAppState  		A pointer to application layer supplied state object
     * @param[in]  aEventCallback  	A function pointer for event call back
     */
    void SetEventCallback(void * apAppState, EventCallback aEventCallback);
    static void DefaultEventHandler(EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam);

    void OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apEc) override;

    /**
     *  Initialize the client object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    aEventCallback   Callback set by appliation.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, EventCallback const aEventCallback, void * const apAppState);

    /**
     *  Shutdown the Clieent. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();
    void Reset();

    CHIP_ERROR SendReadRequest(NodeId aNodeId, Transport::AdminId aAdminId);

    virtual ~ReadClient() = default;

    bool IsFree() { return (nullptr == mpExchangeCtx); };

private:
    friend class InteractionModelEngine;

    void MoveToState(const ClientState aTargetState);
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle && aPayload);

    void ClearState();
    const char * GetStateStr() const;

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;

    CHIP_ERROR ClearExistingExchangeContext();
    ClientState mState;
    EventCallback mEventCallback;
    bool mMoreChunkedMessages;
    void * mpAppState;
};

}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_READ_CLIENT_H
