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

#ifndef _CHIP_INTERACTION_MODEL_READ_HANDLER_H
#define _CHIP_INTERACTION_MODEL_READ_HANDLER_H

#include <app/reporting/EventLoggingTypes.h>
#include <app/reporting/LoggingManagement.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <map>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
namespace reporting {
class ReportingEngine;
}
class ReadHandler
{
public:
    enum EventID
    {
        kEvent_OnReadRequestParsed = 0,
    };

    union InEventParam
    {
        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            bool mGetToAllEvents;
            // Do not close the EC
            Messaging::ExchangeContext * mpEC;

            // Do not modify the message content
            const PacketHeader * mpPacketHeader; ///< A pointer to the message information for the read request
            // chip::EventId mNextVendedEvents[kPriorityLevel_Last - kPriorityLevel_First + 1];

            ReadHandler * mReadHandler;
        } mReadRequestParsed;
    };

    union OutEventParam
    {
        void Clear(void) { memset(this, 0, sizeof(*this)); }
    };

    typedef void (*EventCallback)(void * const aAppState, EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam);

    static void DefaultEventHandler(EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam);

    void OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);

    /**
     *  Initialize the ReadHandler. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr);

    /**
     *  Shutdown the ReadHandler. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();

    void Reset();

    virtual ~ReadHandler() = default;

    bool IsFree() { return (nullptr == mpExchangeCtx); };

private:
    friend class InteractionModelEngine;
    friend class reporting::ReportingEngine;

    enum HandlerState
    {
        kState_Uninitialized = 0, ///< The handler has not been initialized
        kState_Initialized,       ///< The handler has been initialized and is ready
        kState_ReportDataSending, ///< The handler is sending out reportData
    };

    reporting::PriorityLevel FindNextPriorityForTransfer(void);

    CHIP_ERROR SetEventLogEndpoint(reporting::LoggingManagement & aLogger);

    bool CheckEventUpToDate(reporting::LoggingManagement & inLogger);

    CHIP_ERROR SendReportData(System::PacketBufferHandle && aPayload);

    CHIP_ERROR ProcessReadRequest(System::PacketBufferHandle && aPayload);

    void OnReportProcessingComplete();

    bool IsReportable(void) { return (mState == kState_Initialized); }
    bool IsReporting(void) { return (mState == kState_ReportDataSending); }

    void MoveToState(const HandlerState aTargetState);
    void ClearState();
    const char * GetStateStr() const;
    CHIP_ERROR ClearExistingExchangeContext();

    chip::EventId mSelfVendedEvents[reporting::kPriorityLevel_Last - reporting::kPriorityLevel_First + 1];
    chip::EventId mLastScheduledEventId[reporting::kPriorityLevel_Last - reporting::kPriorityLevel_First + 1];
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;

    reporting::PriorityLevel mCurrentPriority;
    size_t mBytesOffloaded;
    bool mSuppressResponse;
    bool mGetToAllEvents;
    HandlerState mState;
    EventCallback mEventCallback;
};
} // namespace app
} // namespace chip

#endif // _CHIP_INTERACTION_MODEL_COMMAND_HANDLER_H
