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
#include "system/SystemClock.h"
#include <app/AppConfig.h>
#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventHeader.h>
#include <app/EventPathParams.h>
#include <app/MessageDef/ReadRequestMessage.h>
#include <app/MessageDef/StatusIB.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <app/MessageDef/SubscribeRequestMessage.h>
#include <app/MessageDef/SubscribeResponseMessage.h>
#include <app/OperationalSessionSetup.h>
#include <app/ReadPrepareParams.h>
#include <app/data-model/Decode.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeHolder.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

#if CHIP_CONFIG_ENABLE_READ_CLIENT
namespace chip {
namespace app {

class InteractionModelEngine;

/**
 *  @class ReadClient
 *
 *  @brief The read client represents the initiator side of a Read Or Subscribe Interaction (depending on the APIs invoked).
 *
 *         When used to manage subscriptions, the client provides functionality to automatically re-subscribe as needed,
 *         including re-establishing CASE under certain conditions (see Callback::OnResubscriptionNeeded for more info).
 *         This is the default behavior. A consumer can completely opt-out of this behavior by over-riding
 *         Callback::OnResubscriptionNeeded and providing an alternative implementation.
 *
 */
class ReadClient : public Messaging::ExchangeDelegate
{
public:
    class Callback
    {
    public:
        Callback() = default;

        // Callbacks are not expected to be copyable or movable.
        Callback(const Callback &)             = delete;
        Callback(Callback &&)                  = delete;
        Callback & operator=(const Callback &) = delete;
        Callback & operator=(Callback &&)      = delete;

        virtual ~Callback() = default;

        /**
         * Used to notify a (maybe empty) report data is received from peer and the subscription and the peer is alive.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         */
        virtual void NotifySubscriptionStillActive(const ReadClient & apReadClient) {}

        /**
         * Used to signal the commencement of processing of the first attribute or event report received in a given exchange.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * Once OnReportBegin has been called, either OnReportEnd or OnError will be called before OnDone.
         *
         */
        virtual void OnReportBegin() {}

        /**
         * Used to signal the completion of processing of the last attribute or event report in a given exchange.
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
         * If using auto resubscription, OnSubscriptionEstablished will be called whenever resubscription is established.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] aSubscriptionId The identifier of the subscription that was established.
         */
        virtual void OnSubscriptionEstablished(SubscriptionId aSubscriptionId) {}

        /**
         * OnResubscriptionNeeded will be called when a subscription that was started with SendAutoResubscribeRequest has terminated
         * and re-subscription is needed. The termination cause is provided to help inform subsequent re-subscription logic.
         *
         * The base implementation automatically re-subscribes at appropriate intervals taking the termination cause into account
         * (see ReadClient::DefaultResubscribePolicy for more details). If the default implementation doesn't suffice, the logic of
         * ReadClient::DefaultResubscribePolicy is broken down into its constituent methods that are publicly available for
         * applications to call and sequence.
         *
         * If the peer is LIT ICD, and the timeout is reached, `aTerminationCause` will be
         * `CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT`. In this case, returning `CHIP_NO_ERROR` will still trigger a resubscribe
         * attempt, while returning `CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT` will put the subscription in the
         * `InactiveICDSubscription` state.  In the latter case, OnResubscriptionNeeded will be called again when
         * `OnActiveModeNotification` is called.
         *
         * If the method is over-ridden, it's the application's responsibility to take the appropriate steps needed to eventually
         * call-back into the ReadClient object to schedule a re-subscription (by invoking ReadClient::ScheduleResubscription).
         *
         * If the application DOES NOT want re-subscription to happen on a particular invocation of this method, returning anything
         * other than CHIP_NO_ERROR will terminate the interaction and result in OnError, OnDeallocatePaths and OnDone being called
         * in that sequence.
         *
         * This object MUST continue to exist after this call is completed. The
         * application shall wait until it receives an OnDone call to destroy the object.
         *
         * @param[in] aTerminationCause The cause of failure of the subscription that just terminated.
         */
        virtual CHIP_ERROR OnResubscriptionNeeded(ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
        {
            return apReadClient->DefaultResubscribePolicy(aTerminationCause);
        }

        /**
         * OnError will be called when an error occurs *after* a successful call to SendRequest(). The following
         * errors will be delivered through this call in the aError field:
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR encapsulating a StatusIB: If we got a non-path-specific
         *   status response from the server.  In that case, constructing
         *   a StatusIB from the error can be used to extract the status.
         * - CHIP_ERROR*: All other cases.
         *
         * This object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] aError       A system error code that conveys the overall error code.
         */
        virtual void OnError(CHIP_ERROR aError) {}

        /**
         * OnDone will be called when ReadClient has finished all work and it is
         * safe to destroy and free the allocated ReadClient object and any
         * other objects associated with the Read or Subscribe interaction.
         *
         * This function will:
         *      - Always be called exactly *once* for a given ReadClient instance.
         *      - Be called even in error circumstances.
         *      - Only be called after a successful call to SendRequest has been
         *        made, when the read completes or the subscription is shut down.
         *
         * @param[in] apReadClient the ReadClient for the completed interaction.
         */
        virtual void OnDone(ReadClient * apReadClient) = 0;

        /**
         * This function is invoked when using SendAutoResubscribeRequest, where the ReadClient was configured to auto re-subscribe
         * and the ReadPrepareParams was moved into this client for management. This will have to be free'ed appropriately by the
         * application. If SendAutoResubscribeRequest fails, this function will be called before it returns the failure. If
         * SendAutoResubscribeRequest succeeds, this function will be called immediately before calling OnDone, or
         * when the ReadClient is destroyed, if that happens before OnDone. If  SendAutoResubscribeRequest is not called,
         * this function will not be called.
         */
        virtual void OnDeallocatePaths(ReadPrepareParams && aReadPrepareParams) {}

        /**
         * This function is invoked when constructing a read/subscribeRequest that does not have data
         * version filters specified, to give the callback a chance to provide some.
         *
         * This function is expected to encode as many complete data version filters as will fit into
         * the buffer, rolling back any partially-encoded filters if it runs out of space, and set the
         * aEncodedDataVersionList boolean to true if it has successfully encoded at least one data version filter.
         *
         * Otherwise aEncodedDataVersionList will be set to false.
         */
        virtual CHIP_ERROR OnUpdateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                                         const Span<AttributePathParams> & aAttributePaths,
                                                         bool & aEncodedDataVersionList)
        {
            aEncodedDataVersionList = false;
            return CHIP_NO_ERROR;
        }

        /*
         * Get highest received event number.
         * If the application does not want to filter events by event number, it should call ClearValue() on aEventNumber
         * and return CHIP_NO_ERROR.  An error return from this function will fail the entire read client interaction.
         */
        virtual CHIP_ERROR GetHighestReceivedEventNumber(Optional<EventNumber> & aEventNumber)
        {
            aEventNumber.ClearValue();
            return CHIP_NO_ERROR;
        }

        /**
         * OnUnsolicitedMessageFromPublisher will be called for a subscription
         * ReadClient when any incoming message is received from a matching
         * node on the fabric.
         *
         * This callback will be called:
         *   - When receiving any unsolicited communication from the node
         *   - Even for disconnected subscriptions.
         *
         * Callee MUST not synchronously destroy ReadClients in this callback.
         *
         * @param[in] apReadClient the ReadClient for the subscription.
         */
        virtual void OnUnsolicitedMessageFromPublisher(ReadClient * apReadClient) {}

        /**
         * OnCASESessionEstablished will be called for a subscription ReadClient when
         * it finishes setting up a CASE session, as part of either automatic
         * re-subscription or doing an initial subscribe based on ScopedNodeId.
         *
         * The callee is allowed to modify the ReadPrepareParams (e.g. to change
         * things like min/max intervals based on the session parameters).
         */
        virtual void OnCASESessionEstablished(const SessionHandle & aSession, ReadPrepareParams & aSubscriptionParams) {}
    };

    enum class InteractionType : uint8_t
    {
        Read,
        Subscribe,
    };

    enum class PeerType : uint8_t
    {
        kNormal,
        kLITICD,
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
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object. Allowed to be null
     *                                 if the version of SendAutoResubscribeRequest that takes a
     *                                 ScopedNodeId is used.
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

    /**
     *  Send a request.  There can be one request outstanding on a given ReadClient.
     *  If SendRequest returns success, no more SendRequest calls can happen on this ReadClient
     *  until the corresponding OnDone call has happened.
     *
     *  This will send either a Read Request or a Subscribe Request depending on
     *  the InteractionType this read client was initialized with.
     *
     *  If the params contain more data version filters than can fit in the request packet
     *  the list will be truncated as needed, i.e. filter inclusion is on a best effort basis.
     *
     *  @retval #others fail to send read request
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendRequest(ReadPrepareParams & aReadPrepareParams);

    /**
     *  Re-activate an inactive subscription.
     *
     *  This function should be called when the peer is an ICD that is checking in and this ReadClient represents a subscription
     * that would cause that ICD to not need to check in anymore.
     *
     *  This API only works when issuing subscription via SendAutoResubscribeRequest.
     */
    void OnActiveModeNotification();

    void OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);

    void OnUnsolicitedMessageFromPublisher()
    {
        TriggerResubscribeIfScheduled("unsolicited message");

        // Then notify callbacks
        mpCallback.OnUnsolicitedMessageFromPublisher(this);
    }

    auto GetSubscriptionId() const
    {
        using returnType = Optional<decltype(mSubscriptionId)>;
        return mInteractionType == InteractionType::Subscribe ? returnType(mSubscriptionId) : returnType::Missing();
    }

    FabricIndex GetFabricIndex() const { return mPeer.GetFabricIndex(); }
    NodeId GetPeerNodeId() const { return mPeer.GetNodeId(); }
    bool IsReadType() { return mInteractionType == InteractionType::Read; }
    bool IsSubscriptionType() const { return mInteractionType == InteractionType::Subscribe; };

    /*
     * Retrieve the reporting intervals associated with an active subscription. This should only be called if we're of subscription
     * interaction type and after a subscription has been established.
     */
    CHIP_ERROR GetReportingIntervals(uint16_t & aMinIntervalFloorSeconds, uint16_t & aMaxIntervalCeilingSeconds) const
    {
        VerifyOrReturnError(IsSubscriptionType(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(IsSubscriptionActive(), CHIP_ERROR_INCORRECT_STATE);

        aMinIntervalFloorSeconds   = mMinIntervalFloorSeconds;
        aMaxIntervalCeilingSeconds = mMaxInterval;

        return CHIP_NO_ERROR;
    }

    ReadClient * GetNextClient() { return mpNext; }
    void SetNextClient(ReadClient * apClient) { mpNext = apClient; }

    /**
     *  Like SendSubscribeRequest, but the ReadClient will automatically attempt to re-establish the subscription if
     *  we decide that the subscription has dropped.  The exact behavior of the re-establishment can be controlled
     *  by setting mResubscribePolicy in the ReadPrepareParams.  If not set, a default behavior with exponential backoff will be
     *  used.
     *
     *  The application has to know to
     *  a) allocate a ReadPrepareParams object that will have fields mpEventPathParamsList and mpAttributePathParamsList and
     *  mpDataVersionFilterList with lifetimes as long as the ReadClient itself and b) free those up later in the call to
     *  OnDeallocatePaths. Note: At a given time in the system, you can either have a single subscription with re-sub enabled that
     *  has mKeepSubscriptions = false, OR, multiple subs with re-sub enabled with mKeepSubscriptions = true. You shall not
     *  have a mix of both simultaneously. If SendAutoResubscribeRequest is called at all, it guarantees that it will call
     *  OnDeallocatePaths (either befor returning error, or when OnDone is called). SendAutoResubscribeRequest is the only case
     *  that calls OnDeallocatePaths, since that's the only case when the consumer moved a ReadParams into the client.
     *
     */
    CHIP_ERROR SendAutoResubscribeRequest(ReadPrepareParams && aReadPrepareParams);

    /**
     * Like SendAutoResubscribeRequest above, but without a session being
     * available in the ReadPrepareParams.  When this is used, the ReadClient is
     * responsible for setting up the CASE session itself.
     *
     * When using this version of SendAutoResubscribeRequest, any session to
     * which ReadPrepareParams has a reference will be ignored.
     */
    CHIP_ERROR SendAutoResubscribeRequest(const ScopedNodeId & aPublisherId, ReadPrepareParams && aReadPrepareParams);

    /**
     *   This provides a standard re-subscription policy implementation that given a termination cause, does the following:
     *   - Calculates the time till next subscription with fibonacci back-off (implemented by ComputeTimeTillNextSubscription()).
     *   - Schedules the next subscription attempt at the computed interval from the previous step. Operational discovery and
     *     CASE establishment will be attempted if aTerminationCause was CHIP_ERROR_TIMEOUT. In all other cases, it will attempt
     *     to re-use a previously established session.
     */
    CHIP_ERROR DefaultResubscribePolicy(CHIP_ERROR aTerminationCause);

    /**
     * Computes the time, in milliseconds, until the next re-subscription over
     * an ever increasing window following a fibonacci sequence with the current retry count
     * used as input to the fibonacci algorithm.
     *
     * CHIP_RESUBSCRIBE_MAX_FIBONACCI_STEP_INDEX is the maximum value the retry count can tick up to.
     *
     */
    uint32_t ComputeTimeTillNextSubscription();

    /**
     * Schedules a re-subscription aTimeTillNextResubscriptionMs into the future.
     *
     * If an application wants to set up CASE on their own, they should call ComputeTimeTillNextSubscription() to compute the next
     * interval at which they should attempt CASE and attempt CASE at that time. On successful CASE establishment, this method
     * should be called with the new SessionHandle provided through 'aNewSessionHandle', 'aTimeTillNextResubscriptionMs' set to 0
     * (i.e async, but as soon as possible) and 'aReestablishCASE' set to false.
     *
     * Otherwise, if aReestablishCASE is true, operational discovery and CASE will be attempted at that time before
     * the actual IM interaction is initiated.
     *
     * aReestablishCASE SHALL NOT be set to true if a valid SessionHandle is provided through newSessionHandle.
     */
    CHIP_ERROR ScheduleResubscription(uint32_t aTimeTillNextResubscriptionMs, Optional<SessionHandle> aNewSessionHandle,
                                      bool aReestablishCASE);

    // Like SendSubscribeRequest, but allows sending certain forms of invalid
    // subscribe requests that servers are expected to reject, for testing
    // purposes.  Should only be called from tests.
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    CHIP_ERROR SendSubscribeRequestWithoutValidation(const ReadPrepareParams & aReadPrepareParams)
    {
        return SendSubscribeRequestImpl(aReadPrepareParams);
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

    /**
     * Override the interval at which liveness of the subscription is assessed.
     * By default, this is set set to the max interval of the subscription + ACK timeout of the underlying session.
     *
     * This can be only be called once a subscription has been established and is active. Once called, this will cancel any existing
     * liveness timers and schedule a new one.
     *
     * This can be called from the Callback::OnSubscriptionEstablished callback.
     *
     */
    void OverrideLivenessTimeout(System::Clock::Timeout aLivenessTimeout);

    /**
     * If the ReadClient currently has a resubscription attempt scheduled,
     * trigger that attempt right now.  This is generally useful when a consumer
     * has some sort of indication that the server side is currently up and
     * communicating, so right now is a good time to try to resubscribe.
     *
     * The reason string is used for logging if a resubscribe is triggered.
     */
    void TriggerResubscribeIfScheduled(const char * reason);

    /**
     * Returns the timeout after which we consider the subscription to have
     * dropped, if we have received no messages within that amount of time.
     *
     * Returns NullOptional if a subscription has not yet been established (and
     * hence the MaxInterval is not yet known), or if the subscription session
     * is gone and hence the relevant MRP parameters can no longer be determined.
     */
    Optional<System::Clock::Timeout> GetSubscriptionTimeout();

private:
    friend class TestReadInteraction;
    friend class InteractionModelEngine;

    enum class ClientState : uint8_t
    {
        Idle,                      ///< The client has been initialized and is ready for a SendRequest
        AwaitingInitialReport,     ///< The client is waiting for initial report
        AwaitingSubscribeResponse, ///< The client is waiting for subscribe response
        SubscriptionActive,        ///< The client is maintaining subscription
        InactiveICDSubscription,   ///< The client is waiting to resubscribe for LIT device
    };

    enum class ReportType
    {
        // kUnsolicited reports are the first message in an exchange.
        kUnsolicited,
        // kContinuingTransaction reports are responses to a message we sent.
        kContinuingTransaction
    };

    bool IsMatchingSubscriptionId(SubscriptionId aSubscriptionId)
    {
        return aSubscriptionId == mSubscriptionId && mInteractionType == InteractionType::Subscribe;
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /**
     *  Updates the type (LIT ICD or not) of the peer.
     *
     *  When the subscription is active, this function will just set the flag. When the subscription is an InactiveICDSubscription,
     * setting the peer type to SIT or normal devices will also trigger a resubscription attempt.
     *
     */
    void OnPeerTypeChange(PeerType aType);

    /**
     *  Check if current read client is being used
     *
     */
    bool IsIdle() const { return mState == ClientState::Idle; }
    bool IsInactiveICDSubscription() const { return mState == ClientState::InactiveICDSubscription; }
    bool IsSubscriptionActive() const { return mState == ClientState::SubscriptionActive; }
    bool IsAwaitingInitialReport() const { return mState == ClientState::AwaitingInitialReport; }
    bool IsAwaitingSubscribeResponse() const { return mState == ClientState::AwaitingSubscribeResponse; }

    CHIP_ERROR GenerateEventPaths(EventPathIBs::Builder & aEventPathsBuilder, const Span<EventPathParams> & aEventPaths);
    CHIP_ERROR GenerateAttributePaths(AttributePathIBs::Builder & aAttributePathIBsBuilder,
                                      const Span<AttributePathParams> & aAttributePaths);

    CHIP_ERROR GenerateDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                             const Span<AttributePathParams> & aAttributePaths,
                                             const Span<DataVersionFilter> & aDataVersionFilters, bool & aEncodedDataVersionList);
    CHIP_ERROR BuildDataVersionFilterList(DataVersionFilterIBs::Builder & aDataVersionFilterIBsBuilder,
                                          const Span<AttributePathParams> & aAttributePaths,
                                          const Span<DataVersionFilter> & aDataVersionFilters, bool & aEncodedDataVersionList);
    CHIP_ERROR ReadICDOperatingModeFromAttributeDataIB(TLV::TLVReader && aReader, PeerType & aType);
    CHIP_ERROR ProcessAttributeReportIBs(TLV::TLVReader & aAttributeDataIBsReader);
    CHIP_ERROR ProcessEventReportIBs(TLV::TLVReader & aEventReportIBsReader);

    static void OnLivenessTimeoutCallback(System::Layer * apSystemLayer, void * apAppState);
    CHIP_ERROR ProcessSubscribeResponse(System::PacketBufferHandle && aPayload);
    CHIP_ERROR RefreshLivenessCheckTimer();
    CHIP_ERROR ComputeLivenessCheckTimerTimeout(System::Clock::Timeout * aTimeout);
    void CancelLivenessCheckTimer();
    void CancelResubscribeTimer();
    void TriggerResubscriptionForLivenessTimeout(CHIP_ERROR aReason);
    void MoveToState(const ClientState aTargetState);
    CHIP_ERROR ProcessAttributePath(AttributePathIB::Parser & aAttributePath, ConcreteDataAttributePath & aClusterInfo);
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle && aPayload, ReportType aReportType);
    const char * GetStateStr() const;

    /*
     * Checks if we should re-subscribe based on the specified re-subscription policy. If we should, re-subscription is scheduled
     * aNextResubscribeIntervalMsec is updated accordingly, and true is returned.
     *
     * If we should not resubscribe, false is returned.
     *
     *  @param[out]    aNextResubscribeIntervalMsec    How long we will wait before trying to auto-resubscribe.
     */
    bool ResubscribeIfNeeded(uint32_t & aNextResubscribeIntervalMsec);

    // Specialized request-sending functions.
    CHIP_ERROR SendReadRequest(ReadPrepareParams & aReadPrepareParams);
    // SendSubscribeRequest performs som validation on aSubscribePrepareParams
    // and then calls SendSubscribeRequestImpl.
    CHIP_ERROR SendSubscribeRequest(const ReadPrepareParams & aSubscribePrepareParams);
    CHIP_ERROR SendSubscribeRequestImpl(const ReadPrepareParams & aSubscribePrepareParams);
    void UpdateDataVersionFilters(const ConcreteDataAttributePath & aPath);
    static void OnResubscribeTimerCallback(System::Layer * apSystemLayer, void * apAppState);
    // Called to ensure OnReportBegin is called before calling OnEventData or OnAttributeData
    void NoteReportingData();

    /*
     * Called internally to signal the completion of all work on this object, gracefully close the
     * exchange and finally, signal to the application that it's
     * safe to release this object.
     *
     * If aError != CHIP_NO_ERROR, this will trigger re-subscriptions if allowResubscription is true
     * AND if this ReadClient instance is tracking a subscription AND the applications decides to do so
     * in their implementation of Callback::OnResubscriptionNeeded().
     */
    void Close(CHIP_ERROR aError, bool allowResubscription = true);

    void StopResubscription();
    void ClearActiveSubscriptionState();

    static void HandleDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                      const SessionHandle & sessionHandle);
    static void HandleDeviceConnectionFailure(void * context, const OperationalSessionSetup::ConnectionFailureInfo & failureInfo);

    CHIP_ERROR GetMinEventNumber(const ReadPrepareParams & aReadPrepareParams, Optional<EventNumber> & aEventMin);

    /**
     * Start setting up a CASE session to our peer, if we can locate a
     * CASESessionManager.  Returns error if we did not even manage to kick off
     * a CASE attempt.
     */
    CHIP_ERROR EstablishSessionToPeer();

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeHolder mExchange;
    Callback & mpCallback;
    ClientState mState    = ClientState::Idle;
    bool mIsReporting     = false;
    bool mIsInitialReport = true;
    // boolean to check if client is waiting for the first priming report
    bool mWaitingForFirstPrimingReport = true;
    bool mPendingMoreChunks            = false;
    uint16_t mMinIntervalFloorSeconds  = 0;
    uint16_t mMaxInterval              = 0;
    SubscriptionId mSubscriptionId     = 0;
    ScopedNodeId mPeer;
    InteractionType mInteractionType = InteractionType::Read;
    Timestamp mEventTimestamp;

    bool mForceCaseOnNextResub      = true;
    bool mIsResubscriptionScheduled = false;

    // mMinimalResubscribeDelay is used to store the delay returned with a BUSY
    // response to a Sigma1 message.
    System::Clock::Milliseconds16 mMinimalResubscribeDelay = System::Clock::kZero;

    chip::Callback::Callback<OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<OperationalSessionSetup::OnSetupFailure> mOnConnectionFailureCallback;

    ReadClient * mpNext                 = nullptr;
    InteractionModelEngine * mpImEngine = nullptr;

    //
    // This stores the params associated with the interaction in a specific set of cases:
    //      1. Stores all parameters when used with subscriptions initiated using SendAutoResubscribeRequest.
    //      2. Stores just the SessionHolder when used with any subscriptions.
    //
    ReadPrepareParams mReadPrepareParams;
    uint32_t mNumRetries = 0;

    System::Clock::Timeout mLivenessTimeoutOverride = System::Clock::kZero;

    bool mIsPeerLIT = false;

    // End Of Container (0x18) uses one byte.
    static constexpr uint16_t kReservedSizeForEndOfContainer = 1;
    // Reserved size for the uint8_t InteractionModelRevision flag, which takes up 1 byte for the control tag and 1 byte for the
    // context tag, 1 byte for value
    static constexpr uint16_t kReservedSizeForIMRevision = 1 + 1 + 1;
    // Reserved buffer for TLV level overhead (the overhead for data version filter IBs EndOfContainer, IM reversion end
    // of RequestMessage (another end of container)).
    static constexpr uint16_t kReservedSizeForTLVEncodingOverhead =
        kReservedSizeForEndOfContainer + kReservedSizeForIMRevision + kReservedSizeForEndOfContainer;
};

};     // namespace app
};     // namespace chip
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT
