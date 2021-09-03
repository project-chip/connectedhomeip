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
                    Messaging::ExchangeContext * apExchangeContext);

    /**
     *  Shut down the ReadHandler. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown(ShutdownOptions aOptions = ShutdownOptions::KeepCurrentExchange);
    /**
     *  Process a read request.  Parts of the processing may end up being asynchronous, but the ReadHandler
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
    bool IsReportable() const { return mState == HandlerState::Reportable; }
    bool IsReporting() const { return mState == HandlerState::Reporting; }
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

    bool IsInitialReport() { return mInitialReport; }

private:
    enum class HandlerState
    {
        Uninitialized = 0, ///< The handler has not been initialized
        Initialized,       ///< The handler has been initialized and is ready
        Reportable,        ///< The handler has received read request and is waiting for the data to send to be available
        Reporting,         ///< The handler is reporting
    };

    CHIP_ERROR ProcessReadRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR ProcessAttributePathList(AttributePathList::Parser & aAttributePathListParser);
    CHIP_ERROR ProcessEventPathList(EventPathList::Parser & aEventPathListParser);
    CHIP_ERROR OnStatusReport(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                 const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;
    CHIP_ERROR OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);
    void MoveToState(const HandlerState aTargetState);

    const char * GetStateStr() const;

    // Merges aAttributePath inside the existing internal mpAttributeClusterInfoList
    bool MergeOverlappedAttributePath(ClusterInfo & aAttributePath);

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
};
} // namespace app
} // namespace chip
