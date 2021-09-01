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
#include <app/MessageDef/AttributeDataList.h>
#include <app/MessageDef/AttributeStatusElement.h>
#include <app/MessageDef/WriteRequest.h>
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

class WriteClientHandle;
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
    /**
     *  Shutdown the WriteClient. This terminates this instance
     *  of the object and releases all held resources.
     */
    void Shutdown();

    CHIP_ERROR PrepareAttribute(const AttributePathParams & attributePathParams);
    CHIP_ERROR FinishAttribute();
    TLV::TLVWriter * GetAttributeDataElementTLVWriter();

    uint64_t GetAppIdentifier() const { return mAppIdentifier; }
    void SetAppIdentifier(uint64_t aAppIdentifier) { mAppIdentifier = aAppIdentifier; }
    NodeId GetSourceNodeId() const
    {
        return mpExchangeCtx != nullptr ? mpExchangeCtx->GetSecureSession().GetPeerNodeId() : kUndefinedNodeId;
    }

private:
    friend class TestWriteInteraction;
    friend class InteractionModelEngine;
    friend class WriteClientHandle;

    enum class State
    {
        Uninitialized = 0, // The client has not been initialized
        Initialized,       // The client has been initialized
        AddAttribute,      // The client has added attribute and ready for a SendWriteRequest
        AwaitingResponse,  // The client has sent out the write request message
    };

    /**
     * Finalize Write Request Message TLV Builder and retrieve final data from tlv builder for later sending
     */
    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle & aPacket);

    /**
     *  Once SendWriteRequest returns successfully, the WriteClient will
     *  handle calling Shutdown on itself once it decides it's done with waiting
     *  for a response (i.e. times out or gets a response). Client can specify
     *  the maximum time to wait for response (in milliseconds) via timeout parameter.
     *  Default timeout value will be used otherwise.
     *  If SendWriteRequest is never called, or the call fails, the API
     *  consumer is responsible for calling Shutdown on the WriteClient.
     */
    CHIP_ERROR SendWriteRequest(NodeId aNodeId, FabricIndex aFabricIndex, Optional<SessionHandle> apSecureSession,
                                uint32_t timeout);

    /**
     *  Initialize the client object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *  @retval #CHIP_ERROR_INCORRECT_STATE incorrect state if it is already initialized
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate,
                    uint64_t aApplicationIdentifier);

    virtual ~WriteClient() = default;

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                 const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /**
     *  Check if current write client is being used
     */
    bool IsFree() const { return mState == State::Uninitialized; };

    void MoveToState(const State aTargetState);
    CHIP_ERROR ProcessWriteResponseMessage(System::PacketBufferHandle && payload);
    CHIP_ERROR ProcessAttributeStatusElement(AttributeStatusElement::Parser & aAttributeStatusElement);
    CHIP_ERROR ConstructAttributePath(const AttributePathParams & aAttributePathParams,
                                      AttributeDataElement::Builder aAttributeDataElement);
    void ClearExistingExchangeContext();
    const char * GetStateStr() const;
    void ClearState();

    /**
     * Internal shutdown method that we use when we know what's going on with
     * our exchange and don't need to manually close it.
     */
    void ShutdownInternal();

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;
    State mState                               = State::Uninitialized;
    System::PacketBufferTLVWriter mMessageWriter;
    WriteRequest::Builder mWriteRequestBuilder;
    uint8_t mAttributeStatusIndex = 0;
    uint64_t mAppIdentifier       = 0;
};

class WriteClientHandle
{
public:
    /**
     * Construct an empty WriteClientHandle.
     */
    WriteClientHandle() : mpWriteClient(nullptr) {}
    WriteClientHandle(decltype(nullptr)) : mpWriteClient(nullptr) {}

    /**
     * Construct a WriteClientHandle that takes ownership of a WriteClient from another.
     */
    WriteClientHandle(WriteClientHandle && aOther)
    {
        mpWriteClient        = aOther.mpWriteClient;
        aOther.mpWriteClient = nullptr;
    }

    ~WriteClientHandle() { SetWriteClient(nullptr); }

    /**
     * Access a WriteClientHandle's public methods.
     */
    WriteClient * operator->() const { return mpWriteClient; }

    /**
     *  Finalize the message and send it to the desired node. The underlying write object will always be released, and the user
     * should not use this object after calling this function.
     */
    CHIP_ERROR SendWriteRequest(NodeId aNodeId, FabricIndex aFabricIndex, Optional<SessionHandle> apSecureSession,
                                uint32_t timeout = kImMessageTimeoutMsec);

    /**
     *  Encode an attribute value that can be directly encoded using TLVWriter::Put
     */
    template <class T>
    CHIP_ERROR EncodeScalarAttributeWritePayload(const chip::app::AttributePathParams & attributePath, T value)
    {
        chip::TLV::TLVWriter * writer = nullptr;

        VerifyOrReturnError(mpWriteClient != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mpWriteClient->PrepareAttribute(attributePath));
        VerifyOrReturnError((writer = mpWriteClient->GetAttributeDataElementTLVWriter()) != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(writer->Put(chip::TLV::ContextTag(chip::app::AttributeDataElement::kCsTag_Data), value));
        ReturnErrorOnFailure(mpWriteClient->FinishAttribute());

        return CHIP_NO_ERROR;
    }

    /**
     *  Set the internal WriteClient of the Handler, expected to be called by InteractionModelEngline only since the user
     * application does not have direct access to apWriteClient.
     */
    void SetWriteClient(WriteClient * apWriteClient)
    {
        if (mpWriteClient != nullptr)
        {
            mpWriteClient->Shutdown();
        }
        mpWriteClient = apWriteClient;
    }

private:
    friend class TestWriteInteraction;

    WriteClientHandle(const WriteClientHandle &) = delete;
    WriteClientHandle & operator=(const WriteClientHandle &) = delete;
    WriteClientHandle & operator=(const WriteClientHandle &&) = delete;

    WriteClient * mpWriteClient = nullptr;
};

} // namespace app
} // namespace chip
