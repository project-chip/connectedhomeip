/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *     This file defines read handler for a CHIP Interaction Data model
 *
 */

#pragma once

#include <access/AccessControl.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributePathExpandIterator.h>
#include <app/ClusterInfo.h>
#include <app/EventManagement.h>
#include <app/MessageDef/AttributePathIBs.h>
#include <app/MessageDef/EventPathIBs.h>
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

//
// Forward declare the Engine (which is in a different namespace) to be able to use
// it as a friend class below.
//
namespace reporting {
class Engine;
}

/**
 *  @class ReadHandler
 *
 *  @brief The read handler is responsible for processing a read request, asking the attribute/event store
 *         for the relevant data, and sending a reply.
 *
 */
class ReadHandler : public Messaging::ExchangeDelegate
{
public:
    using SubjectDescriptor = Access::SubjectDescriptor;

    enum class InteractionType : uint8_t
    {
        Read,
        Subscribe,
    };

    class Callback
    {
    public:
        virtual ~Callback() = default;

        /*
         * Method that signals to a registered callback that this object
         * has completed doing useful work and is now safe for release/destruction.
         */
        virtual void OnDone(ReadHandler & apReadHandlerObj) = 0;
    };

    /**
     *
     *  Constructor.
     *
     *  The callback passed in has to outlive this handler object.
     *
     */
    ReadHandler(Callback & apCallback, Messaging::ExchangeContext * apExchangeContext, InteractionType aInteractionType);

    /*
     * Destructor - as part of destruction, it will abort the exchange context
     * if a valid one still exists.
     *
     * See Abort() for details on when that might occur.
     */
    ~ReadHandler();

    /**
     *  Process a read/subscribe request.  Parts of the processing may end up being asynchronous, but the ReadHandler
     *  guarantees that it will call Shutdown on itself when processing is done (including if OnReadInitialRequest
     *  returns an error).
     *
     *  @retval #Others If fails to process read request
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR OnInitialRequest(System::PacketBufferHandle && aPayload);

    /**
     *  Send ReportData to initiator
     *
     *  @param[in]    aPayload             A payload that has read request data
     *  @param[in]    aMoreChunks          A flags indicating there will be more chunks expected to be sent for this read request
     *
     *  @retval #Others If fails to send report data
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR SendReportData(System::PacketBufferHandle && aPayload, bool aMoreChunks);

    /**
     *  Returns whether this ReadHandler represents a subscription that was created by the other side of the provided exchange.
     */
    bool IsFromSubscriber(Messaging::ExchangeContext & apExchangeContext);

    bool IsReportable() const { return mState == HandlerState::GeneratingReports && !mHoldReport && (mDirty || !mHoldSync); }
    bool IsGeneratingReports() const { return mState == HandlerState::GeneratingReports; }
    bool IsAwaitingReportResponse() const { return mState == HandlerState::AwaitingReportResponse; }

    ClusterInfo * GetAttributeClusterInfolist() { return mpAttributeClusterInfoList; }
    ClusterInfo * GetEventClusterInfolist() { return mpEventClusterInfoList; }
    EventNumber & GetEventMin() { return mEventMin; }
    PriorityLevel GetCurrentPriority() { return mCurrentPriority; }

    // if current priority is in the middle, it has valid snapshoted last event number, it check cleaness via comparing
    // with snapshotted last event number. if current priority  is in the end, no valid
    // sanpshotted last event, check with latest last event number, re-setup snapshoted checkpoint, and compare again.
    bool CheckEventClean(EventManagement & aEventManager);

    bool IsType(InteractionType type) const { return (mInteractionType == type); }
    bool IsChunkedReport() { return mIsChunkedReport; }
    bool IsPriming() { return mIsPrimingReports; }
    bool IsActiveSubscription() const { return mActiveSubscription; }
    bool IsFabricFiltered() const { return mIsFabricFiltered; }
    CHIP_ERROR OnSubscribeRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);
    void GetSubscriptionId(uint64_t & aSubscriptionId) { aSubscriptionId = mSubscriptionId; }
    AttributePathExpandIterator * GetAttributePathExpandIterator() { return &mAttributePathExpandIterator; }
    void SetDirty()
    {
        mDirty = true;
        // If the contents of the global dirty set have changed, we need to reset the iterator since the paths
        // we've sent up till now are no longer valid and need to be invalidated.
        mAttributePathExpandIterator = AttributePathExpandIterator(mpAttributeClusterInfoList);
        mAttributeEncoderState       = AttributeValueEncoder::AttributeEncodeState();
    }
    void ClearDirty() { mDirty = false; }
    bool IsDirty() { return mDirty; }
    NodeId GetInitiatorNodeId() const { return mInitiatorNodeId; }
    FabricIndex GetAccessingFabricIndex() const { return mSubjectDescriptor.fabricIndex; }

    const SubjectDescriptor & GetSubjectDescriptor() const { return mSubjectDescriptor; }

    void UnblockUrgentEventDelivery()
    {
        mHoldReport = false;
        mDirty      = true;
    }

    const AttributeValueEncoder::AttributeEncodeState & GetAttributeEncodeState() const { return mAttributeEncoderState; }
    void SetAttributeEncodeState(const AttributeValueEncoder::AttributeEncodeState & aState) { mAttributeEncoderState = aState; }
    uint32_t GetLastWrittenEventsBytes() { return mLastWrittenEventsBytes; }

private:
    friend class TestReadInteraction;

    //
    // The engine needs to be able to Abort/Close a ReadHandler instance upon completion of work for a given read/subscribe
    // interaction. We do not want to make these methods public just to give an adjacent class in the IM access, since public
    // should really be taking application usage considerations as well. Hence, make it a friend.
    //
    friend class chip::app::reporting::Engine;

    enum class HandlerState
    {
        Idle,                   ///< The handler has been initialized and is ready
        GeneratingReports,      ///< The handler has received either a Read or Subscribe request and is the process of generating a
                                ///< report.
        AwaitingReportResponse, ///< The handler has sent the report to the client and is awaiting a status response.
        AwaitingDestruction,    ///< The object has completed its work and is awaiting destruction by the application.
    };

    /*
     * This forcibly closes the exchange context if a valid one is pointed to. Such a situation does
     * not arise during normal message processing flows that all normally call Close() above.
     *
     * This will eventually call Close() to drive the process of eventually releasing this object (unless called from the
     * destructor).
     *
     * This is only called by a very narrow set of external objects as needed.
     */
    void Abort(bool aCalledFromDestructor = false);

    /**
     * Called internally to signal the completion of all work on this object, gracefully close the
     * exchange and finally, signal to a registerd callback that it's
     * safe to release this object.
     */
    void Close();

    static void OnUnblockHoldReportCallback(System::Layer * apSystemLayer, void * apAppState);
    static void OnRefreshSubscribeTimerSyncCallback(System::Layer * apSystemLayer, void * apAppState);
    CHIP_ERROR RefreshSubscribeSyncTimer();
    CHIP_ERROR SendSubscribeResponse();
    CHIP_ERROR ProcessSubscribeRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ProcessReadRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ProcessAttributePathList(AttributePathIBs::Parser & aAttributePathListParser);
    CHIP_ERROR ProcessEventPaths(EventPathIBs::Parser & aEventPathsParser);
    CHIP_ERROR ProcessEventFilters(EventFilterIBs::Parser & aEventFiltersParser);
    CHIP_ERROR OnStatusResponse(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;
    CHIP_ERROR OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                System::PacketBufferHandle && aPayload);
    void MoveToState(const HandlerState aTargetState);

    const char * GetStateStr() const;

    Messaging::ExchangeContext * mpExchangeCtx = nullptr;

    // Don't need the response for report data if true
    bool mSuppressResponse = false;

    // Current Handler state
    HandlerState mState                      = HandlerState::Idle;
    ClusterInfo * mpAttributeClusterInfoList = nullptr;
    ClusterInfo * mpEventClusterInfoList     = nullptr;

    PriorityLevel mCurrentPriority = PriorityLevel::Invalid;

    EventNumber mEventMin = 0;

    // The last schedule event number snapshoted in the beginning when preparing to fill new events to reports
    EventNumber mLastScheduledEventNumber      = 0;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Callback & mCallback;

    // Tracks whether we're in the initial phase of receiving priming
    // reports, which is always true for reads and true for subscriptions
    // prior to receiving a subscribe response.
    bool mIsPrimingReports              = true;
    InteractionType mInteractionType    = InteractionType::Read;
    uint64_t mSubscriptionId            = 0;
    uint16_t mMinIntervalFloorSeconds   = 0;
    uint16_t mMaxIntervalCeilingSeconds = 0;
    SessionHolder mSessionHandle;
    // mHoldReport is used to prevent subscription data delivery while we are
    // waiting for the min reporting interval to elapse.  If we have to send a
    // report immediately due to an urgent event being queued,
    // UnblockUrgentEventDelivery can be used to force mHoldReport to false.
    bool mHoldReport         = false;
    bool mDirty              = false;
    bool mActiveSubscription = false;
    // The flag indicating we are in the middle of a series of chunked report messages, this flag will be cleared during sending
    // last chunked message.
    bool mIsChunkedReport                                    = false;
    NodeId mInitiatorNodeId                                  = kUndefinedNodeId;
    AttributePathExpandIterator mAttributePathExpandIterator = AttributePathExpandIterator(nullptr);
    bool mIsFabricFiltered                                   = false;
    // mHoldSync is used to prevent subscription empty report delivery while we
    // are waiting for the max reporting interval to elaps.  When mHoldSync
    // becomes false, we are allowed to send an empty report to keep the
    // subscription alive on the client.
    bool mHoldSync                   = false;
    uint32_t mLastWrittenEventsBytes = 0;
    SubjectDescriptor mSubjectDescriptor;
    // The detailed encoding state for a single attribute, used by list chunking feature.
    AttributeValueEncoder::AttributeEncodeState mAttributeEncoderState;
};
} // namespace app
} // namespace chip
