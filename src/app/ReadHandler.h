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

#include <app/InteractionModelDelegate.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
/**
 *  @class ReadHandler
 *
 *  @brief The read handler is responsible for processing read request, asks reporting engine to pack report data and sending
 * reports via this handler.
 *
 */
class ReadHandler
{
public:
    /**
     *  Initialize the ReadHandler. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate);

    /**
     *  Shutdown the ReadHandler. This terminates this instance
     *  of the object and releases all held resources.
     *
     */
    void Shutdown();
    /**
     *  Process a read request
     *
     *  @param[in]    apExchangeContext    A pointer to the ExchangeContext.
     *  @param[in]    aPayload             A payload that has read request data
     *
     *  @retval #Others If fails to process read request
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR OnReadRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle aPayload);

    /**
     *  Send ReportData to initiator
     *
     *  @param[in]    aPayload             A payload that has read request data
     *
     *  @retval #Others If fails to send report data
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR SendReportData(System::PacketBufferHandle aPayload);

    bool IsFree() const { return mState == HandlerState::Uninitialized; }

    virtual ~ReadHandler() = default;

private:
    enum class HandlerState
    {
        Uninitialized = 0, //< The handler has not been initialized
        Initialized,       //< The handler has been initialized and is ready
        ReportDataSending, //< The handler is sending out reportData
    };

    CHIP_ERROR ProcessReadRequest(System::PacketBufferHandle aPayload);

    void OnReportProcessingComplete();

    void MoveToState(const HandlerState aTargetState);

    const char * GetStateStr() const;
    CHIP_ERROR ClearExistingExchangeContext();

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;

    // Don't need the response for report data if true
    bool mSuppressResponse;

    // Retrieve all events
    bool mGetToAllEvents;

    // Current Handler state
    HandlerState mState;
};
} // namespace app
} // namespace chip
