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
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelDelegate.h>
#include <app/MessageDef/AttributeDataIBs.h>
#include <app/MessageDef/AttributeStatusIB.h>
#include <app/MessageDef/StatusIB.h>
#include <app/MessageDef/WriteRequestMessage.h>
#include <app/data-model/Encode.h>
#include <app/data-model/List.h>
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
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace app {

class InteractionModelEngine;

/**
 *  @brief The read client represents the initiator side of a Write Interaction, and is responsible
 *  for generating one Write Request for a particular set of attributes, and handling the Write response.
 *  Consumer can allocate one write client, then call PrepareAttribute, insert attribute value, followed by FinishAttribute for
 * every attribute it wants to insert in write request, then call SendWriteRequest
 *
 */
class WriteClient : public Messaging::ExchangeDelegate
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;

        /**
         * OnResponse will be called when a write response has been received
         * and processed for the given path.
         *
         * The WriteClient object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call before it shuts down the object.
         *
         * @param[in] apWriteClient   The write client object that initiated the write transaction.
         * @param[in] aPath           The attribute path field in write response.
         * @param[in] attributeStatus Attribute-specific status, containing an InteractionModel::Status code as well as
         *                            an optional cluster-specific status code.
         */
        virtual void OnResponse(const WriteClient * apWriteClient, const ConcreteAttributePath & aPath, StatusIB attributeStatus) {}

        /**
         * OnError will be called when an error occurs *after* a successful call to SendWriteRequest(). The following
         * errors will be delivered through this call in the aError field:
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR*: All other cases.
         *
         * The WriteClient object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call before it shuts down the object.
         *
         * @param[in] apWriteClient The write client object that initiated the attribute write transaction.
         * @param[in] aStatus       A status response if we got one; might be Status::Failure if we did not.
         * @param[in] aError        A system error code that conveys the overall error code.
         */
        virtual void OnError(const WriteClient * apWriteClient, const StatusIB & aStatus, CHIP_ERROR aError) {}

        /**
         * OnDone will be called when WriteClient has finished all work and is reserved for future WriteClient ownership change.
         * (#10366) Users may use this function to release their own objects related to this write interaction.
         *
         * This function will:
         *      - Always be called exactly *once* for a given WriteClient instance.
         *      - Be called even in error circumstances.
         *      - Only be called after a successful call to SendWriteRequest has been made.
         *
         * @param[in] apWriteClient The write client object of the terminated write transaction.
         */
        virtual void OnDone(WriteClient * apWriteClient) = 0;
    };

    /**
     *  Construct the client object. Within the lifetime
     *  of this instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *  @param[in]    aTimedWriteTimeoutMs If provided, do a timed write using this timeout.
     */
    WriteClient(Messaging::ExchangeManager * apExchangeMgr, Callback * apCallback,
                const Optional<uint16_t> & aTimedWriteTimeoutMs) :
        mpExchangeMgr(apExchangeMgr),
        mpCallback(apCallback), mTimedWriteTimeoutMs(aTimedWriteTimeoutMs)
    {}

    /**
     *  Encode an attribute value that can be directly encoded using TLVWriter::Put
     */
    template <class T>
    CHIP_ERROR EncodeAttributeWritePayload(const chip::app::AttributePathParams & attributePath, const T & value)
    {
        chip::TLV::TLVWriter * writer = nullptr;

        ReturnErrorOnFailure(PrepareAttribute(attributePath));
        VerifyOrReturnError((writer = GetAttributeDataIBTLVWriter()) != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(
            DataModel::Encode(*writer, chip::TLV::ContextTag(to_underlying(chip::app::AttributeDataIB::Tag::kData)), value));
        ReturnErrorOnFailure(FinishAttribute());

        return CHIP_NO_ERROR;
    }

    /**
     *  Once SendWriteRequest returns successfully, the WriteClient will
     *  handle calling Shutdown on itself once it decides it's done with waiting
     *  for a response (i.e. times out or gets a response). Client can specify
     *  the maximum time to wait for response (in milliseconds) via timeout parameter.
     *  Default timeout value will be used otherwise.
     *  If SendWriteRequest is never called, or the call fails, the API
     *  consumer is responsible for calling Shutdown on the WriteClient.
     */
    CHIP_ERROR SendWriteRequest(const SessionHandle & session, System::Clock::Timeout timeout = kImMessageTimeout);

    /**
     *  Shutdown the WriteClient. This terminates this instance
     *  of the object and releases all held resources.
     */
    void Shutdown();

    CHIP_ERROR PrepareAttribute(const AttributePathParams & attributePathParams);
    CHIP_ERROR FinishAttribute();
    TLV::TLVWriter * GetAttributeDataIBTLVWriter();

    /*
     * Destructor - as part of destruction, it will abort the exchange context
     * if a valid one still exists.
     *
     * See Abort() for details on when that might occur.
     */
    virtual ~WriteClient() { Abort(); }

private:
    friend class TestWriteInteraction;
    friend class InteractionModelEngine;

    enum class State
    {
        Uninitialized = 0,   // The client has not been initialized
        Initialized,         // The client has been initialized
        AddAttribute,        // The client has added attribute and ready for a SendWriteRequest
        AwaitingTimedStatus, // Sent a Tiemd Request, waiting for response.
        AwaitingResponse,    // The client has sent out the write request message
        ResponseReceived,    // We have gotten a response after sending write request
        AwaitingDestruction, // The object has completed its work and is awaiting destruction by the application.
    };

    /**
     * The actual init function, called during encoding first attribute data.
     */
    CHIP_ERROR Init();

    /**
     * Finalize Write Request Message TLV Builder and retrieve final data from tlv builder for later sending
     */
    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle & aPacket);

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /**
     *  Check if current write client is being used
     */
    bool IsFree() const { return mState == State::Uninitialized; };

    void MoveToState(const State aTargetState);
    CHIP_ERROR ProcessWriteResponseMessage(System::PacketBufferHandle && payload);
    CHIP_ERROR ProcessAttributeStatusIB(AttributeStatusIB::Parser & aAttributeStatusIB);
    const char * GetStateStr() const;
    void ClearState();

    /**
     * Called internally to signal the completion of all work on this object, gracefully close the
     * exchange (by calling into the base class) and finally, signal to the application that it's
     * safe to release this object.
     */
    void Close();

    /**
     * This forcibly closes the exchange context if a valid one is pointed to. Such a situation does
     * not arise during normal message processing flows that all normally call Close() above. This can only
     * arise due to application-initiated destruction of the object when this object is handling receiving/sending
     * message payloads.
     */
    void Abort();

    // Handle a message received when we are expecting a status response to a
    // Timed Request.  The caller is assumed to have already checked that our
    // exchange context member is the one the message came in on.
    //
    // aStatusIB will be populated with the returned status if we can parse it
    // successfully.
    CHIP_ERROR HandleTimedStatus(const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload,
                                 StatusIB & aStatusIB);

    // Send our queued-up Write Request message.  Assumes the exchange is ready
    // and mPendingWriteData is populated.
    CHIP_ERROR SendWriteRequest();

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    Callback * mpCallback                      = nullptr;
    State mState                               = State::Uninitialized;
    System::PacketBufferTLVWriter mMessageWriter;
    WriteRequestMessage::Builder mWriteRequestBuilder;
    // TODO Maybe we should change PacketBufferTLVWriter so we can finalize it
    // but have it hold on to the buffer, and get the buffer from it later.
    // Then we could avoid this extra pointer-sized member.
    System::PacketBufferHandle mPendingWriteData;
    // If mTimedWriteTimeoutMs has a value, we are expected to do a timed
    // write.
    Optional<uint16_t> mTimedWriteTimeoutMs;
};

} // namespace app
} // namespace chip
