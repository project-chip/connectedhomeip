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
#include <app/AttributePathParams.h>
#include <app/DataVersionFilter.h>
#include <app/EventManagement.h>
#include <app/EventPathParams.h>
#include <app/MessageDef/AttributePathIBs.h>
#include <app/MessageDef/EventPathIBs.h>
#include <app/ObjectList.h>
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
class TestReportingEngine;
} // namespace reporting

class InteractionModelEngine;

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

    /*
     * A callback used to interact with the application.
     */
    class ApplicationCallback
    {
    public:
        virtual ~ApplicationCallback() = default;

        /*
         * Called right after a SubscribeRequest has been parsed and processed. This notifies an interested application
         * of a subscription that is about to be established. It also provides an avenue for altering the parameters of the
         * subscription (specifically, the min/max negotiated intervals) or even outright rejecting the subscription for
         * application-specific reasons.
         *
         * TODO: Need a new IM status code to convey application-rejected subscribes. Currently, a Failure IM status code is sent
         * back to the subscriber, which isn't sufficient.
         *
         * To reject the subscription, a CHIP_ERROR code that is not equivalent to CHIP_NO_ERROR should be returned.
         *
         * More information about the set of paths associated with this subscription can be retrieved by calling the appropriate
         * Get* methods below.
         *
         * aReadHandler:            Reference to the ReadHandler associated with the subscription.
         * aSecureSession:          A reference to the underlying secure session associated with the subscription.
         *
         */
        virtual CHIP_ERROR OnSubscriptionRequested(ReadHandler & aReadHandler, Transport::SecureSession & aSecureSession)
        {
            return CHIP_NO_ERROR;
        }

        /*
         * Called after a subscription has been fully established.
         */
        virtual void OnSubscriptionEstablished(ReadHandler & aReadHandler){};

        /*
         * Called right before a subscription is about to get terminated. This is only called on subscriptions that were terminated
         * after they had been fully established (and therefore had called OnSubscriptionEstablished).
         * OnSubscriptionEstablishment().
         */
        virtual void OnSubscriptionTerminated(ReadHandler & aReadHandler){};
    };

    /*
     * A callback used to manage the lifetime of the ReadHandler object.
     */
    class ManagementCallback
    {
    public:
        virtual ~ManagementCallback() = default;

        /*
         * Method that signals to a registered callback that this object
         * has completed doing useful work and is now safe for release/destruction.
         */
        virtual void OnDone(ReadHandler & apReadHandlerObj) = 0;

        /*
         * Retrieve the ApplicationCallback (if a valid one exists) from our management entity. This avoids
         * storing multiple references to the application provided callback and having to subsequently manage lifetime
         * issues w.r.t the ReadHandler itself.
         */
        virtual ApplicationCallback * GetAppCallback() = 0;
    };

    /*
     * Destructor - as part of destruction, it will abort the exchange context
     * if a valid one still exists.
     *
     * See Abort() for details on when that might occur.
     */
    ~ReadHandler() override;

    /**
     *
     *  Constructor.
     *
     *  The callback passed in has to outlive this handler object.
     *
     */
    ReadHandler(ManagementCallback & apCallback, Messaging::ExchangeContext * apExchangeContext, InteractionType aInteractionType);

    const ObjectList<AttributePathParams> * GetAttributePathList() const { return mpAttributePathList; }
    const ObjectList<EventPathParams> * GetEventPathList() const { return mpEventPathList; }
    const ObjectList<DataVersionFilter> * GetDataVersionFilterList() const { return mpDataVersionFilterList; }

    void GetReportingIntervals(uint16_t & aMinInterval, uint16_t & aMaxInterval) const
    {
        aMinInterval = mMinIntervalFloorSeconds;
        aMaxInterval = mMaxIntervalCeilingSeconds;
    }

    /*
     * Set the reporting intervals for the subscription. This SHALL only be called
     * from the OnSubscriptionRequested callback above.
     */
    CHIP_ERROR SetReportingIntervals(uint16_t aMinInterval, uint16_t aMaxInterval)
    {
        VerifyOrReturnError(IsIdle(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(aMinInterval <= aMaxInterval, CHIP_ERROR_INVALID_ARGUMENT);

        mMinIntervalFloorSeconds   = aMinInterval;
        mMaxIntervalCeilingSeconds = aMaxInterval;
        return CHIP_NO_ERROR;
    }

private:
    PriorityLevel GetCurrentPriority() const { return mCurrentPriority; }
    EventNumber & GetEventMin() { return mEventMin; }

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
    bool IsFromSubscriber(Messaging::ExchangeContext & apExchangeContext) const;

    bool IsIdle() const { return mState == HandlerState::Idle; }
    bool IsReportable() const { return mState == HandlerState::GeneratingReports && !mHoldReport && (IsDirty() || !mHoldSync); }
    bool IsGeneratingReports() const { return mState == HandlerState::GeneratingReports; }
    bool IsAwaitingReportResponse() const { return mState == HandlerState::AwaitingReportResponse; }

    // Resets the path iterator to the beginning of the whole report for generating a series of new reports.
    void ResetPathIterator();

    CHIP_ERROR ProcessDataVersionFilterList(DataVersionFilterIBs::Parser & aDataVersionFilterListParser);

    // if current priority is in the middle, it has valid snapshoted last event number, it check cleaness via comparing
    // with snapshotted last event number. if current priority  is in the end, no valid
    // sanpshotted last event, check with latest last event number, re-setup snapshoted checkpoint, and compare again.
    bool CheckEventClean(EventManagement & aEventManager);

    bool IsType(InteractionType type) const { return (mInteractionType == type); }
    bool IsChunkedReport() const { return mIsChunkedReport; }
    // Is reporting indicates whether we are in the middle of a series chunks. As we will set mIsChunkedReport on the first chunk
    // and clear that flag on the last chunk, we can use mIsChunkedReport to indicate this state.
    bool IsReporting() const { return mIsChunkedReport; }
    bool IsPriming() const { return mIsPrimingReports; }
    bool IsActiveSubscription() const { return mActiveSubscription; }
    bool IsFabricFiltered() const { return mIsFabricFiltered; }
    CHIP_ERROR OnSubscribeRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);
    void GetSubscriptionId(uint64_t & aSubscriptionId) const { aSubscriptionId = mSubscriptionId; }
    AttributePathExpandIterator * GetAttributePathExpandIterator() { return &mAttributePathExpandIterator; }

    /**
     * Notify the read handler that a set of attribute paths has been marked dirty.
     */
    void SetDirty(const AttributePathParams & aAttributeChanged);
    bool IsDirty() const { return (mDirtyGeneration > mPreviousReportsBeginGeneration) || mForceDirty; }
    void ClearDirty() { mForceDirty = false; }

    NodeId GetInitiatorNodeId() const { return mInitiatorNodeId; }
    FabricIndex GetAccessingFabricIndex() const { return mSubjectDescriptor.fabricIndex; }

    const SubjectDescriptor & GetSubjectDescriptor() const { return mSubjectDescriptor; }

    void UnblockUrgentEventDelivery()
    {
        mHoldReport = false;
        mForceDirty = true;
    }

    const AttributeValueEncoder::AttributeEncodeState & GetAttributeEncodeState() const { return mAttributeEncoderState; }
    void SetAttributeEncodeState(const AttributeValueEncoder::AttributeEncodeState & aState) { mAttributeEncoderState = aState; }
    uint32_t GetLastWrittenEventsBytes() const { return mLastWrittenEventsBytes; }
    CHIP_ERROR SendStatusReport(Protocols::InteractionModel::Status aStatus);

    friend class TestReadInteraction;
    friend class chip::app::reporting::TestReportingEngine;

    //
    // The engine needs to be able to Abort/Close a ReadHandler instance upon completion of work for a given read/subscribe
    // interaction. We do not want to make these methods public just to give an adjacent class in the IM access, since public
    // should really be taking application usage considerations as well. Hence, make it a friend.
    //
    friend class chip::app::reporting::Engine;
    friend class chip::app::InteractionModelEngine;

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
    HandlerState mState                                     = HandlerState::Idle;
    ObjectList<AttributePathParams> * mpAttributePathList   = nullptr;
    ObjectList<EventPathParams> * mpEventPathList           = nullptr;
    ObjectList<DataVersionFilter> * mpDataVersionFilterList = nullptr;

    PriorityLevel mCurrentPriority = PriorityLevel::Invalid;

    EventNumber mEventMin = 0;

    // The last schedule event number snapshoted in the beginning when preparing to fill new events to reports
    EventNumber mLastScheduledEventNumber      = 0;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    ManagementCallback & mManagementCallback;

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
    bool mHoldSync = false;

    // The current generation of the reporting engine dirty set the last time we were notified that a path we're interested in was
    // marked dirty.
    //
    // This allows us to detemine whether any paths we care about might have
    // been marked dirty after we had already sent reports for them, which would
    // mean we should report those paths again, by comparing this generation to the
    // current generation when we started sending the last set reports that we completed.
    //
    // This allows us to reset the iterator to the beginning of the current
    // cluster instead of the beginning of the whole report in SetDirty, without
    // permanently missing dirty any paths.
    uint64_t mDirtyGeneration = 0;
    // For subscriptions, we record the dirty set generation when we started to generate the last report.
    // The mCurrentReportsBeginGeneration records the generation at the start of the current report.  This only/
    // has a meaningful value while IsReporting() is true.
    //
    // mPreviousReportsBeginGeneration will be set to mCurrentReportsBeginGeneration after we send the last
    // chunk of the current report.  Anything that was dirty with a generation earlier than
    // mPreviousReportsBeginGeneration has had its value sent to the client.
    bool mForceDirty = false;
    // For subscriptions, we record the timestamp when we started to generate the last report.
    // The mCurrentReportsBeginGeneration records the timestamp for the current report, which won;t be used for checking if this
    // ReadHandler is dirty.
    // mPreviousReportsBeginGeneration will be set to mCurrentReportsBeginGeneration after we sent the last chunk of the current
    // report.
    uint64_t mPreviousReportsBeginGeneration = 0;
    uint64_t mCurrentReportsBeginGeneration  = 0;
    /*
     *           (mDirtyGeneration = b > a, this is a dirty read handler)
     *        +- Start Report -> mCurrentReportsBeginGeneration = c
     *        |      +- SetDirty (Attribute Y) -> mDirtyGeneration = d
     *        |      |     +- Last Chunk -> mPreviousReportsBeginGeneration = mCurrentReportsBeginGeneration = c
     *        |      |     |   +- (mDirtyGeneration = d) > (mPreviousReportsBeginGeneration = c), this is a dirty read handler
     *        |      |     |   |  Attribute X has a dirty generation less than c, Attribute Y has a dirty generation larger than c
     *        |      |     |   |  So Y will be included in the report but X will not be inclued in this report.
     * -a--b--c------d-----e---f---> Generation
     *  |  |
     *  |  +- SetDirty (Attribute X) (mDirtyGeneration = b)
     *  +- mPreviousReportsBeginGeneration
     * For read handler, if mDirtyGeneration > mPreviousReportsBeginGeneration, then we regard it as a dirty read handler, and it
     * should generate report on timeout reached.
     */

    uint32_t mLastWrittenEventsBytes = 0;
    SubjectDescriptor mSubjectDescriptor;
    // The detailed encoding state for a single attribute, used by list chunking feature.
    AttributeValueEncoder::AttributeEncodeState mAttributeEncoderState;
};
} // namespace app
} // namespace chip
