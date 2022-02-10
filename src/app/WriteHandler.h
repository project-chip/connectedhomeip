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
#include <app/AttributeAccessToken.h>
#include <app/AttributePathParams.h>
#include <app/MessageDef/WriteResponseMessage.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
/**
 *  @brief The write handler is responsible for processing a write request and sending a write reply.
 */
class WriteHandler : public Messaging::ExchangeDelegate
{
public:
    /**
     *  Initialize the WriteHandler. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Close is made to terminate the
     *  instance.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR Init();

    /**
     *  Process a write request.  Parts of the processing may end up being asynchronous, but the WriteHandler
     *  guarantees that it will call Close on itself when processing is done (including if OnWriteRequest
     *  returns an error).
     *
     *  @param[in]    apExchangeContext    A pointer to the ExchangeContext.
     *  @param[in]    aPayload             A payload that has read request data
     *  @param[in]    aIsTimedWrite        Whether write is part of a timed interaction.
     *
     *  @retval Status.  Callers are expected to send a status response if the
     *  return status is not Status::Success.
     */
    Protocols::InteractionModel::Status OnWriteRequest(Messaging::ExchangeContext * apExchangeContext,
                                                       System::PacketBufferHandle && aPayload, bool aIsTimedWrite);

    /*
     * This forcibly closes the exchange context if a valid one is pointed to and de-initializes the object. Such a situation does
     * not arise during normal message processing flows that all normally call Close() below.
     */
    void Abort();

    bool IsFree() const { return mState == State::Uninitialized; }

    virtual ~WriteHandler() = default;

    CHIP_ERROR ProcessAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader);
    CHIP_ERROR ProcessGroupAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader);

    CHIP_ERROR AddStatus(const ConcreteDataAttributePath & aPath, const Protocols::InteractionModel::Status aStatus);
    CHIP_ERROR AddStatus(const ConcreteDataAttributePath & aPath, const StatusIB & aStatus);

    CHIP_ERROR AddClusterSpecificSuccess(const AttributePathParams & aAttributePathParams, uint8_t aClusterStatus)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR AddClusterSpecificFailure(const AttributePathParams & aAttributePathParams, uint8_t aClusterStatus)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    FabricIndex GetAccessingFabricIndex() const;

    /**
     * Check whether the WriteRequest we are handling is a timed write.
     */
    bool IsTimedWrite() const { return mIsTimedRequest; }

    bool MatchesExchangeContext(Messaging::ExchangeContext * apExchangeContext) const
    {
        return !IsFree() && mpExchangeCtx == apExchangeContext;
    }

    void CacheACLCheckResult(const AttributeAccessToken & aToken) { mACLCheckCache.SetValue(aToken); }

    bool ACLCheckCacheHit(const AttributeAccessToken & aToken)
    {
        return mACLCheckCache.HasValue() && mACLCheckCache.Value() == aToken;
    }

private:
    enum class State
    {
        Uninitialized = 0, // The handler has not been initialized
        Initialized,       // The handler has been initialized and is ready
        AddStatus,         // The handler has added status code
        Sending,           // The handler has sent out the write response
    };
    Protocols::InteractionModel::Status ProcessWriteRequest(System::PacketBufferHandle && aPayload, bool aIsTimedWrite);
    Protocols::InteractionModel::Status HandleWriteRequestMessage(Messaging::ExchangeContext * apExchangeContext,
                                                                  System::PacketBufferHandle && aPayload, bool aIsTimedWrite);

    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle & packet);
    CHIP_ERROR SendWriteResponse();

    void MoveToState(const State aTargetState);
    void ClearState();
    const char * GetStateStr() const;
    /**
     *  Clean up state when we are done sending the write response.
     */
    void Close();

private: // ExchangeDelegate
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

private:
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    WriteResponseMessage::Builder mWriteResponseBuilder;
    System::PacketBufferTLVWriter mMessageWriter;
    State mState                                  = State::Uninitialized;
    bool mIsTimedRequest                          = false;
    bool mHasMoreChunks                           = false;
    Optional<AttributeAccessToken> mACLCheckCache = NullOptional;
};
} // namespace app
} // namespace chip
