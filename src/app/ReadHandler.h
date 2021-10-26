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

#include <app/ClusterInfo.h>
#include <app/EventManagement.h>
#include <app/InteractionModelDelegate.h>
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
 *  @class ReadHandler
 *
 *  @brief The read handler is responsible for processing a read request, asking the attribute/event store
 *         for the relevant data, and sending a reply.
 *
 */
class ReadHandler : public Messaging::ExchangeDelegate
{
public:
    enum class ShutdownOptions
    {
        KeepCurrentExchange,
        AbortCurrentExchange,
    };

    enum class InteractionType : uint8_t
    {
        Read,
        Subscribe,
    };

    /**
     *  Initialize the ReadHandler. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate,
                    Messaging::ExchangeContext * apExchangeContext, InteractionType aInteractionType);

    /**
     *  Shut down the ReadHandler. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown(ShutdownOptions aOptions = ShutdownOptions::KeepCurrentExchange);
    /**
     *  Process a read/subscribe request.  Parts of the processing may end up being asynchronous, but the ReadHandler
     *  guarantees that it will call Shutdown on itself when processing is done (including if OnReadInitialRequest
     *  returns an error).
     *
     *  @retval #Others If fails to process read request
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR OnReadInitialRequest(System::PacketBufferHandle && aPayload);

    /**
     *  Send ReportData to initiator
     *
     *  @param[in]    aPayload             A payload that has read request data
     *
     *  @retval #Others If fails to send report data
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR SendReportData(System::PacketBufferHandle && aPayload);

    bool IsFree() const { return mState == HandlerState::Uninitialized; }
    bool IsReportable() const { return mState == HandlerState::GeneratingReports && !mHoldReport; }
    bool IsGeneratingReports() const { return mState == HandlerState::GeneratingReports; }
    bool IsAwaitingReportResponse() const { return mState == HandlerState::AwaitingReportResponse; }
    virtual ~ReadHandler() = default;

    ClusterInfo * GetAttributeClusterInfolist() { return mpAttributeClusterInfoList; }
    ClusterInfo * GetEventClusterInfolist() { return mpEventClusterInfoList; }
    EventNumber * GetVendedEventNumberList() { return mSelfProcessedEvents; }
    PriorityLevel GetCurrentPriority() { return mCurrentPriority; }

    // if current priority is in the middle, it has valid snapshoted last event number, it check cleaness via comparing
    // with snapshotted last event number. if current priority  is in the end, no valid
    // sanpshotted last event, check with latest last event number, re-setup snapshoted checkpoint, and compare again.
    bool CheckEventClean(EventManagement & aEventManager);

    // Move to the next dirty priority from critical high priority to debug low priority, where last schedule event number
    // is larger than current self vended event number
    void MoveToNextScheduledDirtyPriority();

    bool IsReadType() { return mInteractionType == InteractionType::Read; }
    bool IsSubscriptionType() { return mInteractionType == InteractionType::Subscribe; }
    bool IsInitialReport() { return mInitialReport; }
    bool IsActiveSubscription() const { return mActiveSubscription; }
    CHIP_ERROR OnSubscribeRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);
    void GetSubscriptionId(uint64_t & aSubscriptionId) { aSubscriptionId = mSubscriptionId; }
    void SetDirty() { mDirty = true; }
    void ClearDirty() { mDirty = false; }
    bool IsDirty() { return mDirty; }
    NodeId GetInitiatorNodeId() const { return mInitiatorNodeId; }
    FabricIndex GetFabricIndex() const { return mFabricIndex; }

private:
    friend class TestReadInteraction;
    enum class HandlerState
    {
        Uninitialized = 0,      ///< The handler has not been initialized
        Initialized,            ///< The handler has been initialized and is ready
        GeneratingReports,      ///< The handler has received either a Read or Subscribe request and is the process of generating a
                                ///< report.
        AwaitingReportResponse, ///< The handler has sent the report to the client and is awaiting a status response.
    };

    static void OnRefreshSubscribeTimerSyncCallback(System::Layer * apSystemLayer, void * apAppState);
    CHIP_ERROR RefreshSubscribeSyncTimer();
    CHIP_ERROR SendSubscribeResponse();
    CHIP_ERROR ProcessSubscribeRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ProcessReadRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ProcessAttributePathList(AttributePathList::Parser & aAttributePathListParser);
    CHIP_ERROR ProcessEventPathList(EventPathList::Parser & aEventPathListParser);
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
    HandlerState mState                      = HandlerState::Uninitialized;
    ClusterInfo * mpAttributeClusterInfoList = nullptr;
    ClusterInfo * mpEventClusterInfoList     = nullptr;

    PriorityLevel mCurrentPriority = PriorityLevel::Invalid;

    // The event number of the last processed event for each priority level
    EventNumber mSelfProcessedEvents[kNumPriorityLevel];

    // The last schedule event number snapshoted in the beginning when preparing to fill new events to reports
    EventNumber mLastScheduledEventNumber[kNumPriorityLevel];
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;
    bool mInitialReport                        = false;
    InteractionType mInteractionType           = InteractionType::Read;
    uint64_t mSubscriptionId                   = 0;
    uint16_t mMinIntervalFloorSeconds          = 0;
    uint16_t mMaxIntervalCeilingSeconds        = 0;
    Optional<SessionHandle> mSessionHandle;
    bool mHoldReport         = false;
    bool mDirty              = false;
    bool mActiveSubscription = false;
    NodeId mInitiatorNodeId  = kUndefinedNodeId;
    FabricIndex mFabricIndex = 0;
};
} // namespace app
} // namespace chip
