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
#include <app/MessageDef/ReadRequestMessage.h>
#include <app/MessageDef/StatusIB.h>
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

class InteractionModelEngine;

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
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         */
        virtual void OnReportBegin() {}

        /**
         * Used to signal the completion of processing of the last attribute report in a given exchange.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         */
        virtual void OnReportEnd() {}

        /**
         * Used to deliver event data received through the Read and Subscribe interactions
         *
         * Only one of the apData and apStatus can be non-null.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] aEventHeader The event header in report response.
         * @param[in] apData A TLVReader positioned right on the payload of the event.
         * @param[in] apStatus Event-specific status, containing an InteractionModel::Status code as well as an optional
         *                     cluster-specific status code.
         */
        virtual void OnEventData(const EventHeader & aEventHeader, TLV::TLVReader * apData, const StatusIB * apStatus) {}

        /**
         * Used to deliver attribute data received through the Read and Subscribe interactions.
         *
         * This callback will be called when:
         *   - Receiving attribute data as response of Read interactions
         *   - Receiving attribute data as reports of subscriptions
         *   - Receiving attribute data as initial reports of subscriptions
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] aPath        The attribute path field in report response.
         * @param[in] apData       The attribute data of the given path, will be a nullptr if status is not Success.
         * @param[in] aStatus      Attribute-specific status, containing an InteractionModel::Status code as well as an
         *                         optional cluster-specific status code.
         */
        virtual void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) {}

        /**
         * OnSubscriptionEstablished will be called when a subscription is established for the given subscription transaction.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] aSubscriptionId The identifier of the subscription that was established.
         */
        virtual void OnSubscriptionEstablished(uint64_t aSubscriptionId) {}

        /**
         * OnError will be called when an error occurs *after* a successful call to SendRequest(). The following
         * errors will be delivered through this call in the aError field:
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR encapsulating a StatusIB: If we got a non-path-specific
         *   status response from the server.  In that case,
         *   StatusIB::InitFromChipError can be used to extract the status.
         * - CHIP_ERROR*: All other cases.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] aError       A system error code that conveys the overall error code.
         */
        virtual void OnError(CHIP_ERROR aError) {}

        /**
         * OnDone will be called when ReadClient has finished all work and is safe to destroy and free the
         * allocated CommandSender object.
         *
         * This function will:
         *      - Always be called exactly *once* for a given ReadClient instance.
         *      - Be called even in error circumstances.
         *      - Only be called after a successful call to SendRequest has been
         *        made, when the read completes or the subscription is shut down.
         *
         */
        virtual void OnDone() = 0;

        /**
         * This function is invoked when using SendAutoResubscribeRequest, where the ReadClient was configured to auto re-subscribe
         * and the ReadPrepareParams was moved into this client for management. This will have to be free'ed appropriately by the
         * application. If SendAutoResubscribeRequest fails, this function will be called before it returns the failure. If
         * SendAutoResubscribeRequest succeeds, this function will be called immediately before calling OnDone. If
         * SendAutoResubscribeRequest is not called, this function will not be called.
         */
        virtual void OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams) {}

        /**
         * This function is invoked when using read/subscribeRequest, where the ReadClient would use cached cluster data version to
         * update data version filter list
         */
        virtual uint32_t OnUpdateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                       DataVersionFilter * apDataVersionFilterList,
                                                       size_t aDataVersionFilterListSize)
        {
            return 0;
        }
    };

    enum class InteractionType : uint8_t
    {
        Read,
        Subscribe,
    };

    /**
     *
     *  Constructor.
     *
     *  The callback passed in has to outlive this ReadClient object.
     *
     *  This object can outlive the InteractionModelEngine passed in. However, upon shutdown of the engine,
     *  this object will cease to function correctly since it depends on the engine for a number of critical functions.
     *
     *  @param[in]    apImEngine       A valid pointer to the IM engine.
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apCallback       Callback set by application.
     *  @param[in]    aInteractionType Type of interaction (read or subscribe)
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE incorrect state if it is already initialized
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    ReadClient(InteractionModelEngine * apImEngine, Messaging::ExchangeManager * apExchangeMgr, Callback & apCallback,
               InteractionType aInteractionType);

    /**
     * Destructor.
     *
     * Will abort the exchange context if a valid one still exists. It will also cancel any
     * liveness timers that may be active.
     *
     * OnDone() will not be called.
     */
    ~ReadClient() override;

    /*
     * This forcibly closes the exchange context if a valid one is pointed to. Such a situation does
     * not arise during normal message processing flows that all normally call Close() above. This can only
     * arise due to application-initiated destruction of the object when this object is handling receiving/sending
     * message payloads. Abort() should be called first before the object is destroyed.
     */
    void Abort();

    /**
     *  Send a request.  There can be one request outstanding on a given ReadClient.
     *  If SendRequest returns success, no more SendRequest calls can happen on this ReadClient
     *  until the corresponding OnDone call has happened.
     *
     *  This will send either a Read Request or a Subscribe Request depending on
     *  the InteractionType this read client was initialized with.
     *
     *  @retval #others fail to send read request
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendRequest(ReadPrepareParams & aReadPrepareParams);

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

    ReadClient * GetNextClient() { return mpNext; }
    void SetNextClient(ReadClient * apClient) { mpNext = apClient; }

    // Like SendSubscribeRequest, but the ReadClient will automatically attempt to re-establish the subscription if
    // we decide that the subscription has dropped.  The exact behavior of the re-establishment can be controlled
    // by setting mResubscribePolicy in the ReadPrepareParams.  If not set, a default behavior with exponential backoff will be
    // used.
    //
    // The application has to know to
    // a) allocate a ReadPrepareParams object that will have fields mpEventPathParamsList and mpAttributePathParamsList and
    // mpDataVersionFilterList with lifetimes as long as the ReadClient itself and b) free those up later in the call to
    // OnDeallocatePaths. Note: At a given time in the system, you can either have a single subscription with re-sub enabled that
    // that has mKeepSubscriptions = false, OR, multiple subs with re-sub enabled with mKeepSubscriptions = true. You shall not have
    // a mix of both simultaneously. If SendAutoResubscribeRequest is called at all, it guarantees that it will call
    // OnDeallocatePaths when OnDone is called. SendAutoResubscribeRequest is the only case that calls OnDeallocatePaths, since
    // that's the only case when the consumer moved a ReadParams into the client.
    CHIP_ERROR SendAutoResubscribeRequest(ReadPrepareParams && aReadPrepareParams);

private:
    friend class TestReadInteraction;
    friend class InteractionModelEngine;

    enum class ClientState : uint8_t
    {
        Idle,                      ///< The client has been initialized and is ready for a SendRequest
        AwaitingInitialReport,     ///< The client is waiting for initial report
        AwaitingSubscribeResponse, ///< The client is waiting for subscribe response
        SubscriptionActive,        ///< The client is maintaining subscription
    };

    bool IsMatchingClient(uint64_t aSubscriptionId)
    {
        return aSubscriptionId == mSubscriptionId && mInteractionType == InteractionType::Subscribe;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /**
     *  Check if current read client is being used
     *
     */
    bool IsIdle() const { return mState == ClientState::Idle; }
    bool IsSubscriptionIdle() const { return mState == ClientState::SubscriptionActive; }
    bool IsAwaitingInitialReport() const { return mState == ClientState::AwaitingInitialReport; }
    bool IsAwaitingSubscribeResponse() const { return mState == ClientState::AwaitingSubscribeResponse; }

    CHIP_ERROR GenerateEventPaths(EventPathIBs::Builder & aEventPathsBuilder, EventPathParams * apEventPathParamsList,
                                  size_t aEventPathParamsListSize);
    CHIP_ERROR GenerateAttributePathList(AttributePathIBs::Builder & aAttributePathIBsBuilder,
                                         AttributePathParams * apAttributePathParamsList, size_t aAttributePathParamsListSize);
    CHIP_ERROR GenerateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                             DataVersionFilter * apDataVersionFilterList, size_t aDataVersionFilterListSize,
                                             bool aEnableCachedDataVersionFilter);
    CHIP_ERROR ProcessAttributeReportIBs(TLV::TLVReader & aAttributeDataIBsReader);
    CHIP_ERROR ProcessEventReportIBs(TLV::TLVReader & aEventReportIBsReader);

    void ClearExchangeContext() { mpExchangeCtx = nullptr; }
    static void OnLivenessTimeoutCallback(System::Layer * apSystemLayer, void * apAppState);
    CHIP_ERROR ProcessSubscribeResponse(System::PacketBufferHandle && aPayload);
    CHIP_ERROR RefreshLivenessCheckTimer();
    void CancelLivenessCheckTimer();
    void CancelResubscribeTimer();
    void MoveToState(const ClientState aTargetState);
    CHIP_ERROR ProcessAttributePath(AttributePathIB::Parser & aAttributePath, ConcreteDataAttributePath & aClusterInfo);
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle && aPayload);
    const char * GetStateStr() const;
    bool ResubscribeIfNeeded();
    // Specialized request-sending functions.
    CHIP_ERROR SendReadRequest(ReadPrepareParams & aReadPrepareParams);
    CHIP_ERROR SendSubscribeRequest(ReadPrepareParams & aSubscribePrepareParams);
    void UpdateDataVersionFilters(const ConcreteDataAttributePath & aPath);
    static void OnResubscribeTimerCallback(System::Layer * apSystemLayer, void * apAppState);

    /*
     * Called internally to signal the completion of all work on this object, gracefully close the
     * exchange and finally, signal to the application that it's
     * safe to release this object.
     *
     * If aError != CHIP_NO_ERROR, it is delivered to the application through the OnError callback first.
     *
     */
    void Close(CHIP_ERROR aError);

    void StopResubscription();
    void ClearActiveSubscriptionState();

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    Callback & mpCallback;
    ClientState mState                  = ClientState::Idle;
    bool mIsInitialReport               = true;
    bool mIsPrimingReports              = true;
    bool mPendingMoreChunks             = false;
    uint16_t mMinIntervalFloorSeconds   = 0;
    uint16_t mMaxIntervalCeilingSeconds = 0;
    uint64_t mSubscriptionId            = 0;
    NodeId mPeerNodeId                  = kUndefinedNodeId;
    FabricIndex mFabricIndex            = kUndefinedFabricIndex;
    InteractionType mInteractionType    = InteractionType::Read;
    Timestamp mEventTimestamp;
    EventNumber mEventMin = 0;

    ReadClient * mpNext                 = nullptr;
    InteractionModelEngine * mpImEngine = nullptr;
    ReadPrepareParams mReadPrepareParams;
    uint32_t mNumRetries = 0;

    // Reserved size for the FabricFiltered boolean flag, which takes up 1 byte for the control tag and 1 byte for the context tag.
    static constexpr uint16_t kFabricFiltered = 1 + 1;
    // End Of Container (0x18) uses one byte.
    static constexpr uint16_t kReservedSizeForEndOfContainer = 1;
    // Reserved size for the uint8_t InteractionModelRevision flag, which takes up 1 byte for the control tag and 1 byte for the
    // context tag, 1 byte for value
    static constexpr uint16_t kReservedSizeForIMRevision = 1 + 1 + 1;
    // Reserved buffer for TLV level overhead (the overhead for fabric filter flag, end
    // of RequestMessage (another end of container)).
    static constexpr uint16_t kReservedSizeForTLVEncodingOverhead =
        kFabricFiltered + kReservedSizeForEndOfContainer + kReservedSizeForEndOfContainer;
};

}; // namespace app
}; // namespace chip
