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
#include <app/MessageDef/ReadRequestMessage.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <app/MessageDef/SubscribeRequestMessage.h>
#include <app/MessageDef/SubscribeResponseMessage.h>
#include <app/ReadPrepareParams.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
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
    class Callback
    {
    public:
        virtual ~Callback() = default;
        /**
         * Notification that a list of events is received on the given read client.
         * The ReadClient object MUST continue to exist after this call is completed.
         *
         * @param[in]  apReadClient         The read client which initialized the read transaction.
         * @param[in]  aEventReports        TLV reader positioned at the list that contains the events.  The
         *                                  implementation of EventStreamReceived is expected to call Next() on the reader to
         *                                  advance it to the first element of the list, then process the elements from beginning to
         *                                  the end. The callee is expected to consume all events.
         */
        virtual void OnEventData(const ReadClient * apReadClient, TLV::TLVReader & aEventReports) {}

        /**
         * OnResponse will be called when a report data response has been received and processed for the given path.
         *
         * The ReadClient object MUST continue to exist after this call is completed.
         *
         * This callback will be called when:
         *   - Receiving attribute data as response of Read interactions
         *   - Receiving attribute data as reports of subscriptions
         *   - Receiving attribute data as initial reports of subscriptions
         *
         * @param[in] apReadClient The read client object that initiated the read or subscribe transaction.
         * @param[in] aPath        The attribute path field in report response.
         * @param[in] apData       The attribute data of the given path, will be a nullptr if status is not Success.
         * @param[in] aStatus      Attribute-specific status, containing an InteractionModel::Status code as well as an
         *                         optional cluster-specific status code.
         */
        virtual void OnAttributeData(const ReadClient * apReadClient, const ConcreteAttributePath & aPath, TLV::TLVReader * apData,
                                     const StatusIB & aStatus)
        {}

        /**
         * OnSubscriptionEstablished will be called when a subscription is established for the given subscription transaction.
         *
         * The ReadClient object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call before it shuts down the object.
         *
         * @param[in] apReadClient The read client object that initiated the read transaction.
         */
        virtual void OnSubscriptionEstablished(const ReadClient * apReadClient) {}

        /**
         * OnError will be called when an error occurs *after* a successful call to SendReadRequest(). The following
         * errors will be delivered through this call in the aError field:
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR*: All other cases.
         *
         * The ReadClient object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call before it shuts down the object.
         *
         * @param[in] apReadClient The read client object that initiated the attribute read transaction.
         * @param[in] aError       A system error code that conveys the overall error code.
         */
        virtual void OnError(const ReadClient * apReadClient, CHIP_ERROR aError) {}

        /**
         * OnDone will be called when ReadClient has finished all work and is reserved for future ReadClient ownership change.
         * (#10366) Users may use this function to release their own objects related to this write interaction.
         *
         * This function will:
         *      - Always be called exactly *once* for a given WriteClient instance.
         *      - Be called even in error circumstances.
         *      - Only be called after a successful call to SendWriteRequest as been made.
         *
         * @param[in] apReadClient The read client object of the terminated read transaction.
         */
        virtual void OnDone(ReadClient * apReadClient) = 0;
    };

    enum class InteractionType : uint8_t
    {
        Read,
        Subscribe,
    };
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
     *  until the corresponding InteractionModelDelegate::ReadDone call happens with guarantee.
     *
     *  @retval #others fail to send read request
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendReadRequest(ReadPrepareParams & aReadPrepareParams);

    /**
     *  Send a subscribe Request.  There can be one Subscribe Request outstanding on a given ReadClient.
     *  If SendSubscribeRequest returns success, no more subscribe Requests can be sent on this ReadClient
     *  until the corresponding InteractionModelDelegate::ReadDone call happens with guarantee.
     *
     *  @retval #others fail to send subscribe request
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendSubscribeRequest(ReadPrepareParams & aSubscribePrepareParams);
    CHIP_ERROR OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);

    auto GetSubscriptionId() const
    {
        using returnType = Optional<decltype(mSubscriptionId)>;
        return mInteractionType == InteractionType::Subscribe ? returnType(mSubscriptionId) : returnType::Missing();
    }

    FabricIndex GetFabricIndex() const { return mFabricIndex; }
    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    bool IsReadType() { return mInteractionType == InteractionType::Read; }
    bool IsSubscriptionType() const { return mInteractionType == InteractionType::Subscribe; };
    CHIP_ERROR SendStatusResponse(CHIP_ERROR aError);

private:
    friend class TestReadInteraction;
    friend class InteractionModelEngine;

    enum class ClientState : uint8_t
    {
        Uninitialized = 0,         ///< The client has not been initialized
        Initialized,               ///< The client has been initialized and is ready for a SendReadRequest
        AwaitingInitialReport,     ///< The client is waiting for initial report
        AwaitingSubscribeResponse, ///< The client is waiting for subscribe response
        SubscriptionActive,        ///< The client is maintaining subscription
    };

    bool IsMatchingClient(uint64_t aSubscriptionId)
    {
        return aSubscriptionId == mSubscriptionId && mInteractionType == InteractionType::Subscribe;
    }
    /**
     *  Initialize the client object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  The following callbacks are expected to be invoked on the InteractionModelDelegate:
     *      - EventStreamReceived
     *      - OnReportData
     *      - ReportProcessed
     *      - ReadError
     *      - ReadDone
     *
     *  When processing subscriptions, these callbacks are invoked as well:
     *      - SubscribeResponseProcessed
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apCallback       InteractionModelDelegate set by application.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE incorrect state if it is already initialized
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, Callback * apCallback, InteractionType aInteractionType);

    virtual ~ReadClient() = default;

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /**
     *  Check if current read client is being used
     *
     */
    bool IsFree() const { return mState == ClientState::Uninitialized; }
    bool IsSubscriptionTypeIdle() const { return mState == ClientState::SubscriptionActive; }
    bool IsAwaitingInitialReport() const { return mState == ClientState::AwaitingInitialReport; }
    bool IsAwaitingSubscribeResponse() const { return mState == ClientState::AwaitingSubscribeResponse; }

    CHIP_ERROR GenerateEventPaths(EventPaths::Builder & aEventPathsBuilder, EventPathParams * apEventPathParamsList,
                                  size_t aEventPathParamsListSize);
    CHIP_ERROR GenerateAttributePathList(AttributePathIBs::Builder & aAttributePathIBsBuilder,
                                         AttributePathParams * apAttributePathParamsList, size_t aAttributePathParamsListSize);
    CHIP_ERROR ProcessAttributeReportIBs(TLV::TLVReader & aAttributeDataIBsReader);

    void ClearExchangeContext() { mpExchangeCtx = nullptr; }
    static void OnLivenessTimeoutCallback(System::Layer * apSystemLayer, void * apAppState);
    CHIP_ERROR ProcessSubscribeResponse(System::PacketBufferHandle && aPayload);
    CHIP_ERROR RefreshLivenessCheckTimer();
    void CancelLivenessCheckTimer();
    void MoveToState(const ClientState aTargetState);
    CHIP_ERROR ProcessAttributePath(AttributePathIB::Parser & aAttributePath, ClusterInfo & aClusterInfo);
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle && aPayload);
    CHIP_ERROR AbortExistingExchangeContext();
    const char * GetStateStr() const;

    /**
     * Internal shutdown method that we use when we know what's going on with
     * our exchange and don't need to manually close it.
     */
    void ShutdownInternal(CHIP_ERROR aError);
    bool IsInitialReport() { return mInitialReport; }
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    Callback * mpCallback                      = nullptr;
    ClientState mState                         = ClientState::Uninitialized;
    bool mInitialReport                        = true;
    uint16_t mMinIntervalFloorSeconds          = 0;
    uint16_t mMaxIntervalCeilingSeconds        = 0;
    uint64_t mSubscriptionId                   = 0;
    NodeId mPeerNodeId                         = kUndefinedNodeId;
    FabricIndex mFabricIndex                   = kUndefinedFabricIndex;
    InteractionType mInteractionType           = InteractionType::Read;
};

}; // namespace app
}; // namespace chip
