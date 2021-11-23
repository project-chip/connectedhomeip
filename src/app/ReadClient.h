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
#include <app/ConcreteAttributePath.h>
#include <app/EventHeader.h>
#include <app/EventPathParams.h>
#include <app/InteractionModelDelegate.h>
#include <app/MessageDef/ReadRequestMessage.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <app/MessageDef/SubscribeRequestMessage.h>
#include <app/MessageDef/SubscribeResponseMessage.h>
#include <app/ReadPrepareParams.h>
#include <app/data-model/Decode.h>
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
         * Used to signal the commencement of processing of the first attribute report received in a given exchange.
         */
        virtual void OnReportBegin(const ReadClient * apReadClient) {}

        /**
         * Used to signal the completion of processing of the last attribute report in a given exchange.
         */
        virtual void OnReportEnd(const ReadClient * apReadClient) {}

        /**
         * The ReadClient object MUST continue to exist after this call is completed.
         *
         * This callback will be called when receiving event data received in the Read and Subscribe interactions
         *
         * @param[in] apReadClient: The read client object that initiated the read or subscribe transaction.
         * @param[in] aEventHeader: The event header in report response.
         * @param[in] apData: A TLVReader positioned right on the payload of the event. This will be set to null if the apStatus is
         * not null.
         * @param[in] apStatus: Event-specific status, containing an InteractionModel::Status code as well as an optional
         *                     cluster-specific status code.
         */
        virtual void OnEventData(const ReadClient * apReadClient, const EventHeader & aEventHeader, TLV::TLVReader * apData,
                                 const StatusIB * apStatus)
        {}

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
        virtual void OnAttributeData(const ReadClient * apReadClient, const ConcreteDataAttributePath & aPath,
                                     TLV::TLVReader * apData, const StatusIB & aStatus)
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
    bool IsSubscriptionIdle() const { return mState == ClientState::SubscriptionActive; }
    bool IsAwaitingInitialReport() const { return mState == ClientState::AwaitingInitialReport; }
    bool IsAwaitingSubscribeResponse() const { return mState == ClientState::AwaitingSubscribeResponse; }

    CHIP_ERROR GenerateEventPaths(EventPaths::Builder & aEventPathsBuilder, EventPathParams * apEventPathParamsList,
                                  size_t aEventPathParamsListSize);
    CHIP_ERROR GenerateAttributePathList(AttributePathIBs::Builder & aAttributePathIBsBuilder,
                                         AttributePathParams * apAttributePathParamsList, size_t aAttributePathParamsListSize);
    CHIP_ERROR ProcessAttributeReportIBs(TLV::TLVReader & aAttributeDataIBsReader);
    CHIP_ERROR ProcessEventReportIBs(TLV::TLVReader & aEventReportIBsReader);

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
     * Validate that the Event ID and Cluster ID in the header match that of the type information present in the object and
     * decode the data. The template parameter T is generally expected to be a ClusterName::Events::EventName::Type struct
     *
     * @param [in] aEventHeader  The header of the event being validated.
     * @param [in] aEvent        The event data.
     * @param [in] aReader       The tlv reader.
     */
    template <typename EventDataT>
    CHIP_ERROR DecodeEvent(const EventHeader & aEventHeader, const EventDataT & aEvent, TLV::TLVReader & aReader)
    {
        VerifyOrReturnError((aEventHeader.mPath.mEventId == aEvent.GetEventId()) &&
                                (aEventHeader.mPath.mClusterId == aEvent.GetClusterId()),
                            CHIP_ERROR_INVALID_ARGUMENT);
        return DataModel::Decode(aReader, aEvent);
    }

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
    bool mPendingMoreChunks                    = false;
    uint16_t mMinIntervalFloorSeconds          = 0;
    uint16_t mMaxIntervalCeilingSeconds        = 0;
    uint64_t mSubscriptionId                   = 0;
    NodeId mPeerNodeId                         = kUndefinedNodeId;
    FabricIndex mFabricIndex                   = kUndefinedFabricIndex;
    InteractionType mInteractionType           = InteractionType::Read;
    Timestamp mEventTimestamp;
};

}; // namespace app
}; // namespace chip
