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
#include <app/AttributePathExpandIterator.h>
#include <app/AttributePathParams.h>
#include <app/AttributeValueEncoder.h>
#include <app/CASESessionManager.h>
#include <app/DataVersionFilter.h>
#include <app/EventManagement.h>
#include <app/EventPathParams.h>
#include <app/MessageDef/AttributePathIBs.h>
#include <app/MessageDef/DataVersionFilterIBs.h>
#include <app/MessageDef/EventFilterIBs.h>
#include <app/MessageDef/EventPathIBs.h>
#include <app/OperationalSessionSetup.h>
#include <app/SubscriptionResumptionSessionEstablisher.h>
#include <app/SubscriptionResumptionStorage.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/LinkedList.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeHolder.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

// https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/61a9d19e6af12fdfb0872bcff26d19de6c680a1a/src/Ch02_Architecture.adoc#1122-subscribe-interaction-limits
inline constexpr uint16_t kSubscriptionMaxIntervalPublisherLimit = 3600; // seconds (60 minutes)

namespace chip {
namespace app {

//
// Forward declare the Engine (which is in a different namespace) to be able to use
// it as a friend class below.
//
namespace reporting {
class Engine;
class TestReportingEngine;
class ReportScheduler;
class TestReportScheduler;
} // namespace reporting

class InteractionModelEngine;
class TestInteractionModelEngine;

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

        /*
         * Retrieve the InteractionalModelEngine that holds this ReadHandler.
         */
        virtual InteractionModelEngine * GetInteractionModelEngine() = 0;
    };

    // TODO (#27675) : Merge existing callback and observer into one class and have an observer pool in the Readhandler to notify
    // every
    /*
     * Observer class for ReadHandler, meant to allow multiple objects to observe the ReadHandler. Currently only one observer is
     * supported but all above callbacks should be merged into observer type and an observer pool should be added to allow multiple
     * objects to observe ReadHandler
     */
    class Observer
    {
    public:
        virtual ~Observer() = default;

        /// @brief Callback invoked to notify a subscription was successfully established for the ReadHandler
        /// @param[in] apReadHandler  ReadHandler that completed its subscription
        virtual void OnSubscriptionEstablished(ReadHandler * apReadHandler) = 0;

        /// @brief Callback invoked when a ReadHandler went from a non reportable state to a reportable state. Indicates to the
        /// observer that a report should be emitted when the min interval allows it.
        ///
        /// This will only be invoked for subscribe-type ReadHandler objects, and only after
        /// OnSubscriptionEstablished has been called.
        ///
        /// @param[in] apReadHandler  ReadHandler that became dirty and in HandlerState::CanStartReporting state
        virtual void OnBecameReportable(ReadHandler * apReadHandler) = 0;

        /// @brief Callback invoked when the read handler needs to make sure to send a message to the subscriber within the next
        /// maxInterval time period.
        /// @param[in] apReadHandler ReadHandler that has generated a report
        virtual void OnSubscriptionReportSent(ReadHandler * apReadHandler) = 0;

        /// @brief Callback invoked when a ReadHandler is getting removed so it can be unregistered
        /// @param[in] apReadHandler  ReadHandler getting destroyed
        virtual void OnReadHandlerDestroyed(ReadHandler * apReadHandler) = 0;
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
    ReadHandler(ManagementCallback & apCallback, Messaging::ExchangeContext * apExchangeContext, InteractionType aInteractionType,
                Observer * observer, DataModel::Provider * apDataModel);

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    /**
     *
     *  Constructor in preparation for resuming a persisted subscription
     *
     *  The callback passed in has to outlive this handler object.
     *
     */
    ReadHandler(ManagementCallback & apCallback, Observer * observer, DataModel::Provider * apDataModel);
#endif

    const SingleLinkedListNode<AttributePathParams> * GetAttributePathList() const { return mpAttributePathList; }
    const SingleLinkedListNode<EventPathParams> * GetEventPathList() const { return mpEventPathList; }
    const SingleLinkedListNode<DataVersionFilter> * GetDataVersionFilterList() const { return mpDataVersionFilterList; }

    /**
     * @brief Returns the reporting intervals that will used by the ReadHandler for the subscription being requested.
     *        After the subscription is established, these will be the set reporting intervals and cannot be changed.
     *
     * @param[out] aMinInterval minimum time delta between two reports for the subscription
     * @param[in] aMaxInterval maximum time delta between two reports for the subscription
     */
    void GetReportingIntervals(uint16_t & aMinInterval, uint16_t & aMaxInterval) const
    {
        aMinInterval = mMinIntervalFloorSeconds;
        aMaxInterval = mMaxInterval;
    }

    /**
     * @brief Returns the maximum reporting interval that was initially requested by the subscriber
     *        This is the same value as the mMaxInterval member if the max interval is not changed by the publisher.
     *
     * @note If the device is an ICD, the MaxInterval of a subscription is automatically set to a multiple of the IdleModeDuration.
     *       This function is the only way to get the requested max interval once the OnSubscriptionRequested application callback
     *       is called.
     *
     * @return uint16_t subscriber requested maximum reporting interval
     */
    inline uint16_t GetSubscriberRequestedMaxInterval() const { return mSubscriberRequestedMaxInterval; }

    CHIP_ERROR SetMinReportingIntervalForTests(uint16_t aMinInterval)
    {
        VerifyOrReturnError(IsIdle(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(aMinInterval <= mMaxInterval, CHIP_ERROR_INVALID_ARGUMENT);
        // Ensures the new min interval is higher than the subscriber established one.
        mMinIntervalFloorSeconds = std::max(mMinIntervalFloorSeconds, aMinInterval);
        return CHIP_NO_ERROR;
    }

    /*
     * Set the maximum reporting interval for the subscription. This SHALL only be called
     * from the OnSubscriptionRequested callback above. The restriction is as below
     * MinIntervalFloor ≤ MaxInterval ≤ MAX(SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT, MaxIntervalCeiling)
     * Where SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT is set to 60m in the spec.
     */
    CHIP_ERROR SetMaxReportingInterval(uint16_t aMaxInterval)
    {
        VerifyOrReturnError(IsIdle(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mMinIntervalFloorSeconds <= aMaxInterval, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(aMaxInterval <= std::max(GetPublisherSelectedIntervalLimit(), mSubscriberRequestedMaxInterval),
                            CHIP_ERROR_INVALID_ARGUMENT);
        mMaxInterval = aMaxInterval;
        return CHIP_NO_ERROR;
    }

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    /**
     *
     *  @brief Initialize a ReadHandler for a resumed subsciption
     *
     *  Used after the SubscriptionResumptionSessionEstablisher establishs the CASE session
     */
    void OnSubscriptionResumed(const SessionHandle & sessionHandle, SubscriptionResumptionSessionEstablisher & sessionEstablisher);
#endif

private:
    PriorityLevel GetCurrentPriority() const { return mCurrentPriority; }
    EventNumber & GetEventMin() { return mEventMin; }

    /**
     * Returns SUBSCRIPTION_MAX_INTERVAL_PUBLISHER_LIMIT
     * For an ICD publisher, this SHALL be set to the idle mode duration.
     * Otherwise, this SHALL be set to 60 minutes.
     */
    uint16_t GetPublisherSelectedIntervalLimit();

    enum class ReadHandlerFlags : uint8_t
    {
        // The flag indicating we are in the middle of a series of chunked report messages, this flag will be cleared during
        // sending last chunked message.
        ChunkedReport = (1 << 0),

        // Tracks whether we're in the initial phase of receiving priming
        // reports, which is always true for reads and true for subscriptions
        // prior to receiving a subscribe response.
        PrimingReports     = (1 << 1),
        ActiveSubscription = (1 << 2),
        FabricFiltered     = (1 << 3),
        // For subscriptions, we record the dirty set generation when we started to generate the last report.
        // The mCurrentReportsBeginGeneration records the generation at the start of the current report.  This only/
        // has a meaningful value while IsReporting() is true.
        //
        // mPreviousReportsBeginGeneration will be set to mCurrentReportsBeginGeneration after we send the last
        // chunk of the current report.  Anything that was dirty with a generation earlier than
        // mPreviousReportsBeginGeneration has had its value sent to the client.
        // when receiving initial request, it needs mark current handler as dirty.
        // when there is urgent event, it needs mark current handler as dirty.
        ForceDirty = (1 << 4),

        // Don't need the response for report data if true
        SuppressResponse = (1 << 5),
    };

    /**
     *  Process a read/subscribe request.  Parts of the processing may end up being asynchronous, but the ReadHandler
     *  guarantees that it will call Shutdown on itself when processing is done (including if OnReadInitialRequest
     *  returns an error).
     *
     *  @retval #Others If fails to process read request
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    void OnInitialRequest(System::PacketBufferHandle && aPayload);

    /**
     *  Send ReportData to initiator
     *
     *  @param[in]    aPayload             A payload that has read request data
     *  @param[in]    aMoreChunks          A flags indicating there will be more chunks expected to be sent for this read request
     *
     *  @retval #Others If fails to send report data
     *  @retval #CHIP_NO_ERROR On success.
     *
     *  If an error is returned, the ReadHandler guarantees that it is not in
     *  a state where it's waiting for a response.
     */
    CHIP_ERROR SendReportData(System::PacketBufferHandle && aPayload, bool aMoreChunks);

    /*
     * Get the appropriate size of a packet buffer to allocate for encoding a Report message.
     * This size might depend on the underlying session used by the ReadHandler.
     *
     * The size returned here is the size not including the various prepended headers
     * (what System::PacketBuffer calls the "available size").
     */
    size_t GetReportBufferMaxSize();

    /**
     *  Returns whether this ReadHandler represents a subscription that was created by the other side of the provided exchange.
     */
    bool IsFromSubscriber(Messaging::ExchangeContext & apExchangeContext) const;

    bool IsIdle() const { return mState == HandlerState::Idle; }

    /// @brief Returns whether the ReadHandler is in a state where it can send a report and there is data to report.
    bool ShouldStartReporting() const
    {
        // Important: Anything that changes ShouldStartReporting() from false to true
        // (which can only happen for subscriptions) must call
        // mObserver->OnBecameReportable(this).
        return CanStartReporting() && (ShouldReportUnscheduled() || IsDirty());
    }
    /// @brief CanStartReporting() is true if the ReadHandler is in a state where it could generate
    /// a (possibly empty) report if someone asked it to.
    bool CanStartReporting() const { return mState == HandlerState::CanStartReporting; }
    /// @brief ShouldReportUnscheduled() is true if the ReadHandler should be asked to generate reports
    /// without consulting the report scheduler.
    bool ShouldReportUnscheduled() const
    {
        return CanStartReporting() && (IsType(ReadHandler::InteractionType::Read) || IsPriming());
    }
    bool IsAwaitingReportResponse() const { return mState == HandlerState::AwaitingReportResponse; }

    // Resets the path iterator to the beginning of the whole report for generating a series of new reports.
    void ResetPathIterator();

    CHIP_ERROR ProcessDataVersionFilterList(DataVersionFilterIBs::Parser & aDataVersionFilterListParser);

    // if current priority is in the middle, it has valid snapshoted last event number, it check cleaness via comparing
    // with snapshotted last event number. if current priority  is in the end, no valid
    // sanpshotted last event, check with latest last event number, re-setup snapshoted checkpoint, and compare again.
    bool CheckEventClean(EventManagement & aEventManager);

    bool IsType(InteractionType type) const { return (mInteractionType == type); }
    bool IsChunkedReport() const { return mFlags.Has(ReadHandlerFlags::ChunkedReport); }
    // Is reporting indicates whether we are in the middle of a series chunks. As we will set mIsChunkedReport on the first chunk
    // and clear that flag on the last chunk, we can use mIsChunkedReport to indicate this state.
    bool IsReporting() const { return mFlags.Has(ReadHandlerFlags::ChunkedReport); }
    bool IsPriming() const { return mFlags.Has(ReadHandlerFlags::PrimingReports); }
    bool IsActiveSubscription() const { return mFlags.Has(ReadHandlerFlags::ActiveSubscription); }
    bool IsFabricFiltered() const { return mFlags.Has(ReadHandlerFlags::FabricFiltered); }
    CHIP_ERROR OnSubscribeRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);
    void GetSubscriptionId(SubscriptionId & aSubscriptionId) const { aSubscriptionId = mSubscriptionId; }
    AttributePathExpandIterator * GetAttributePathExpandIterator() { return &mAttributePathExpandIterator; }

    /// @brief Notifies the read handler that a set of attribute paths has been marked dirty. This will schedule a reporting engine
    /// run if the change to the attribute path makes the ReadHandler reportable.
    /// @param aAttributeChanged Path to the attribute that was changed.
    void AttributePathIsDirty(const AttributePathParams & aAttributeChanged);
    bool IsDirty() const
    {
        return (mDirtyGeneration > mPreviousReportsBeginGeneration) || mFlags.Has(ReadHandlerFlags::ForceDirty);
    }
    void ClearForceDirtyFlag() { ClearStateFlag(ReadHandlerFlags::ForceDirty); }
    NodeId GetInitiatorNodeId() const
    {
        auto session = GetSession();
        return session == nullptr ? kUndefinedNodeId : session->GetPeerNodeId();
    }

    FabricIndex GetAccessingFabricIndex() const
    {
        auto session = GetSession();
        return session == nullptr ? kUndefinedFabricIndex : session->GetFabricIndex();
    }

    Transport::SecureSession * GetSession() const;
    SubjectDescriptor GetSubjectDescriptor() const { return GetSession()->GetSubjectDescriptor(); }

    auto GetTransactionStartGeneration() const { return mTransactionStartGeneration; }

    /// @brief Forces the read handler into a dirty state, regardless of what's going on with attributes.
    /// This can lead to scheduling of a reporting run immediately, if the min interval has been reached,
    /// or after the min interval is reached if it has not yet been reached.
    void ForceDirtyState();

    const AttributeEncodeState & GetAttributeEncodeState() const { return mAttributeEncoderState; }
    void SetAttributeEncodeState(const AttributeEncodeState & aState) { mAttributeEncoderState = aState; }
    uint32_t GetLastWrittenEventsBytes() const { return mLastWrittenEventsBytes; }

    // Returns the number of interested paths, including wildcard and concrete paths.
    size_t GetAttributePathCount() const { return mpAttributePathList == nullptr ? 0 : mpAttributePathList->Count(); };
    size_t GetEventPathCount() const { return mpEventPathList == nullptr ? 0 : mpEventPathList->Count(); };
    size_t GetDataVersionFilterCount() const { return mpDataVersionFilterList == nullptr ? 0 : mpDataVersionFilterList->Count(); };

    CHIP_ERROR SendStatusReport(Protocols::InteractionModel::Status aStatus);

    friend class TestReadInteraction;
    friend class chip::app::reporting::TestReportingEngine;
    friend class chip::app::reporting::TestReportScheduler;

    //
    // The engine needs to be able to Abort/Close a ReadHandler instance upon completion of work for a given read/subscribe
    // interaction. We do not want to make these methods public just to give an adjacent class in the IM access, since public
    // should really be taking application usage considerations as well. Hence, make it a friend.
    //
    friend class chip::app::reporting::Engine;
    friend class chip::app::InteractionModelEngine;
    friend class TestInteractionModelEngine;

    // The report scheduler needs to be able to access StateFlag private functions ShouldStartReporting(), CanStartReporting(),
    // ForceDirtyState() and IsDirty() to know when to schedule a run so it is declared as a friend class.
    friend class chip::app::reporting::ReportScheduler;

    enum class HandlerState : uint8_t
    {
        Idle,                   ///< The handler has been initialized and is ready
        CanStartReporting,      ///< The handler has is now capable of generating reports and may generate one immediately
                                ///< or later when other criteria are satisfied (e.g hold-off for min reporting interval).
        AwaitingReportResponse, ///< The handler has sent the report to the client and is awaiting a status response.
        AwaitingDestruction,    ///< The object has completed its work and is awaiting destruction by the application.
    };

    enum class CloseOptions
    {
        kDropPersistedSubscription,
        kKeepPersistedSubscription
    };
    /**
     * Called internally to signal the completion of all work on this objecta and signal to a registered callback that it's
     * safe to release this object.
     *
     *  @param    options             This specifies whether to drop or keep the subscription
     *
     */
    void Close(CloseOptions options = CloseOptions::kDropPersistedSubscription);

    CHIP_ERROR SendSubscribeResponse();
    CHIP_ERROR ProcessSubscribeRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ProcessReadRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ProcessAttributePaths(AttributePathIBs::Parser & aAttributePathListParser);
    CHIP_ERROR ProcessEventPaths(EventPathIBs::Parser & aEventPathsParser);
    CHIP_ERROR ProcessEventFilters(EventFilterIBs::Parser & aEventFiltersParser);
    CHIP_ERROR OnStatusResponse(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload,
                                bool & aSendStatusResponse);
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;
    void MoveToState(const HandlerState aTargetState);

    const char * GetStateStr() const;

    void PersistSubscription();

    /// @brief Modifies a state flag in the read handler. If the read handler went from a
    /// non-reportable state to a reportable state, schedules a reporting engine run.
    /// @param aFlag Flag to set
    /// @param aValue Flag new value
    void SetStateFlag(ReadHandlerFlags aFlag, bool aValue = true);

    /// @brief This function call SetStateFlag with the flag value set to false, thus possibly emitting a report
    /// generation.
    /// @param aFlag Flag to clear
    void ClearStateFlag(ReadHandlerFlags aFlag);

    AttributePathExpandIterator mAttributePathExpandIterator;

    // The current generation of the reporting engine dirty set the last time we were notified that a path we're interested in was
    // marked dirty.
    //
    // This allows us to detemine whether any paths we care about might have
    // been marked dirty after we had already sent reports for them, which would
    // mean we should report those paths again, by comparing this generation to the
    // current generation when we started sending the last set reports that we completed.
    //
    // This allows us to reset the iterator to the beginning of the current
    // cluster instead of the beginning of the whole report in AttributePathIsDirty, without
    // permanently missing dirty any paths.
    uint64_t mDirtyGeneration = 0;

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
     *        |      +- AttributePathIsDirty (Attribute Y) -> mDirtyGeneration = d
     *        |      |     +- Last Chunk -> mPreviousReportsBeginGeneration = mCurrentReportsBeginGeneration = c
     *        |      |     |   +- (mDirtyGeneration = d) > (mPreviousReportsBeginGeneration = c), this is a dirty read handler
     *        |      |     |   |  Attribute X has a dirty generation less than c, Attribute Y has a dirty generation larger than c
     *        |      |     |   |  So Y will be included in the report but X will not be inclued in this report.
     * -a--b--c------d-----e---f---> Generation
     *  |  |
     *  |  +- AttributePathIsDirty (Attribute X) (mDirtyGeneration = b)
     *  +- mPreviousReportsBeginGeneration
     * For read handler, if mDirtyGeneration > mPreviousReportsBeginGeneration, then we regard it as a dirty read handler, and it
     * should generate report on timeout reached.
     */

    // When we don't have enough resources for a new subscription, the oldest subscription might be evicted by interaction model
    // engine, the "oldest" subscription is the subscription with the smallest generation.
    uint64_t mTransactionStartGeneration = 0;

    SubscriptionId mSubscriptionId           = 0;
    uint16_t mMinIntervalFloorSeconds        = 0;
    uint16_t mMaxInterval                    = 0;
    uint16_t mSubscriberRequestedMaxInterval = 0;

    EventNumber mEventMin = 0;

    // The last schedule event number snapshoted in the beginning when preparing to fill new events to reports
    EventNumber mLastScheduledEventNumber = 0;

    // TODO: We should shutdown the transaction when the session expires.
    SessionHolder mSessionHandle;

    Messaging::ExchangeHolder mExchangeCtx;
#if CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE
    // TODO: this should be replaced by a pointer to the InteractionModelEngine that created the ReadHandler
    // once InteractionModelEngine is no longer a singleton (see issue 23625)
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
#endif // CHIP_CONFIG_UNSAFE_SUBSCRIPTION_EXCHANGE_MANAGER_USE

    SingleLinkedListNode<AttributePathParams> * mpAttributePathList   = nullptr;
    SingleLinkedListNode<EventPathParams> * mpEventPathList           = nullptr;
    SingleLinkedListNode<DataVersionFilter> * mpDataVersionFilterList = nullptr;

    ManagementCallback & mManagementCallback;

    uint32_t mLastWrittenEventsBytes = 0;

    // The detailed encoding state for a single attribute, used by list chunking feature.
    // The size of AttributeEncoderState is 2 bytes for now.
    AttributeEncodeState mAttributeEncoderState;

    // Current Handler state
    HandlerState mState            = HandlerState::Idle;
    PriorityLevel mCurrentPriority = PriorityLevel::Invalid;
    BitFlags<ReadHandlerFlags> mFlags;
    InteractionType mInteractionType = InteractionType::Read;

    // TODO (#27675): Merge all observers into one and that one will dispatch the callbacks to the right place.
    Observer * mObserver = nullptr;
};

} // namespace app
} // namespace chip
