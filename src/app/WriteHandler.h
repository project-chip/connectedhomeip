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

#pragma once
#include <app/AttributePathParams.h>
#include <app/InteractionModelDelegate.h>
#include <app/MessageDef/WriteResponse.h>
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
 *  @brief The write handler is responsible for processing a write request and sending a write reply.
 */
class WriteHandler
{
public:
    /**
     *  Initialize the WriteHandler. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR Init(InteractionModelDelegate * apDelegate);

    /**
     *  Process a write request.  Parts of the processing may end up being asynchronous, but the WriteHandler
     *  guarantees that it will call Shutdown on itself when processing is done (including if OnWriteRequest
     *  returns an error).
     *
     *  @param[in]    apExchangeContext    A pointer to the ExchangeContext.
     *  @param[in]    aPayload             A payload that has read request data
     *
     *  @retval #Others If fails to process read request
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload);

    bool IsFree() const { return mState == State::Uninitialized; }

    virtual ~WriteHandler() = default;

    CHIP_ERROR ProcessAttributeDataList(TLV::TLVReader & aAttributeDataListReader);

    CHIP_ERROR AddAttributeStatusCode(const AttributePathParams & aAttributePathParams,
                                      const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                      const Protocols::Id aProtocolId,
                                      const Protocols::InteractionModel::ProtocolCode aProtocolCode);

private:
    enum class State
    {
        Uninitialized = 0,      // The handler has not been initialized
        Initialized,            // The handler has been initialized and is ready
        AddAttributeStatusCode, // The handler has added attribute status code
        Sending,                // The handler has sent out the write response
    };
    CHIP_ERROR ProcessWriteRequest(System::PacketBufferHandle && aPayload);
    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle & packet);
    CHIP_ERROR SendWriteResponse();
    CHIP_ERROR ConstructAttributePath(const AttributePathParams & aAttributePathParams,
                                      AttributeStatusElement::Builder aAttributeStatusElement);

    void MoveToState(const State aTargetState);
    void ClearState();
    const char * GetStateStr() const;
    /**
     *  Clean up state when we are done sending the write response.
     */
    void Shutdown();

    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    WriteResponse::Builder mWriteResponseBuilder;
    System::PacketBufferTLVWriter mMessageWriter;
    State mState = State::Uninitialized;
};
} // namespace app
} // namespace chip
