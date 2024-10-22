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
#include <app/InteractionModelTimeout.h>
#include <app/MessageDef/AttributeDataIBs.h>
#include <app/MessageDef/AttributeStatusIB.h>
#include <app/MessageDef/StatusIB.h>
#include <app/MessageDef/WriteRequestMessage.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <app/data-model/List.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeHolder.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/LockTracker.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace app {

class InteractionModelEngine;

/**
 *  @brief The write client represents the initiator side of a Write Interaction, and is responsible
 *  for generating one Write Request for a particular set of attributes, and handling the Write response.
 *  Consumer can allocate one write client, then call PrepareAttribute, insert attribute value, followed
 *  by FinishAttribute for every attribute it wants to insert in write request, then call SendWriteRequest
 *
 *  Note: When writing lists, you may receive multiple write status responses for a single list.
 *  Please see ChunkedWriteCallback.h for a high level API which will merge status codes for
 *  chunked write requests.
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
        virtual void OnResponse(const WriteClient * apWriteClient, const ConcreteDataAttributePath & aPath,
                                StatusIB attributeStatus)
        {}

        /**
         * OnError will be called when an error occurs *after* a successful call to SendWriteRequest(). The following
         * errors will be delivered through this call in the aError field:
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR encapsulating a StatusIB: If we got a non-path-specific
         *   status response from the server.  In that case, constructing
         *   a StatusIB from the error can be used to extract the status.
         * - CHIP_ERROR*: All other cases.
         *
         * The WriteClient object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call before it shuts down the object.
         *
         * @param[in] apWriteClient The write client object that initiated the attribute write transaction.
         * @param[in] aError        A system error code that conveys the overall error code.
         */
        virtual void OnError(const WriteClient * apWriteClient, CHIP_ERROR aError) {}

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
     *  @param[in]    apCallback       Callback set by application.
     *  @param[in]    aTimedWriteTimeoutMs If provided, do a timed write using this timeout.
     *  @param[in]    aSuppressResponse If provided, set SuppressResponse field to the provided value
     */
    WriteClient(Messaging::ExchangeManager * apExchangeMgr, Callback * apCallback, const Optional<uint16_t> & aTimedWriteTimeoutMs,
                bool aSuppressResponse = false) :
        mpExchangeMgr(apExchangeMgr),
        mExchangeCtx(*this), mpCallback(apCallback), mTimedWriteTimeoutMs(aTimedWriteTimeoutMs),
        mSuppressResponse(aSuppressResponse)
    {
        assertChipStackLockedByCurrentThread();
    }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    WriteClient(Messaging::ExchangeManager * apExchangeMgr, Callback * apCallback, const Optional<uint16_t> & aTimedWriteTimeoutMs,
                uint16_t aReservedSize) :
        mpExchangeMgr(apExchangeMgr),
        mExchangeCtx(*this), mpCallback(apCallback), mTimedWriteTimeoutMs(aTimedWriteTimeoutMs), mReservedSize(aReservedSize)
    {
        assertChipStackLockedByCurrentThread();
    }
#endif

    ~WriteClient() { assertChipStackLockedByCurrentThread(); }

    /**
     *  Encode an attribute value that can be directly encoded using DataModel::Encode. Will create a new chunk when necessary.
     */
    template <class T>
    CHIP_ERROR EncodeAttribute(const AttributePathParams & attributePath, const T & value,
                               const Optional<DataVersion> & aDataVersion = NullOptional)
    {
        ReturnErrorOnFailure(EnsureMessage());

        // Here, we are using kInvalidEndpointId for missing endpoint id, which is used when sending group write requests.
        return EncodeSingleAttributeDataIB(
            ConcreteDataAttributePath(attributePath.HasWildcardEndpointId() ? kInvalidEndpointId : attributePath.mEndpointId,
                                      attributePath.mClusterId, attributePath.mAttributeId, aDataVersion),
            value);
    }

    /**
     *  Encode a possibly-chunked list attribute value.  Will create a new chunk when necessary.
     */
    template <class T>
    CHIP_ERROR EncodeAttribute(const AttributePathParams & attributePath, const DataModel::List<T> & value,
                               const Optional<DataVersion> & aDataVersion = NullOptional)
    {
        // Here, we are using kInvalidEndpointId for missing endpoint id, which is used when sending group write requests.
        ConcreteDataAttributePath path =
            ConcreteDataAttributePath(attributePath.HasWildcardEndpointId() ? kInvalidEndpointId : attributePath.mEndpointId,
                                      attributePath.mClusterId, attributePath.mAttributeId, aDataVersion);

        ReturnErrorOnFailure(EnsureMessage());

        // Encode an empty list for the chunking protocol.
        ReturnErrorOnFailure(EncodeSingleAttributeDataIB(path, DataModel::List<uint8_t>()));

        path.mListOp = ConcreteDataAttributePath::ListOperation::AppendItem;
        for (size_t i = 0; i < value.size(); i++)
        {
            ReturnErrorOnFailure(EncodeSingleAttributeDataIB(path, value.data()[i]));
        }

        return CHIP_NO_ERROR;
    }

    /**
     * Encode a Nullable attribute value.  This needs a separate overload so it can dispatch to the right
     * EncodeAttribute when writing a nullable list.
     */
    template <class T>
    CHIP_ERROR EncodeAttribute(const AttributePathParams & attributePath, const DataModel::Nullable<T> & value,
                               const Optional<DataVersion> & aDataVersion = NullOptional)
    {
        ReturnErrorOnFailure(EnsureMessage());

        if (value.IsNull())
        {
            // Here, we are using kInvalidEndpointId to for missing endpoint id, which is used when sending group write requests.
            return EncodeSingleAttributeDataIB(
                ConcreteDataAttributePath(attributePath.HasWildcardEndpointId() ? kInvalidEndpointId : attributePath.mEndpointId,
                                          attributePath.mClusterId, attributePath.mAttributeId, aDataVersion),
                value);
        }

        return EncodeAttribute(attributePath, value.Value(), aDataVersion);
    }

    /**
     * Encode an attribute value which is already encoded into a TLV. The TLVReader is expected to be initialized and the read head
     * is expected to point to the element to be encoded.
     *
     * Note: When encoding lists with this function, you may receive more than one write status for a single list. You can refer
     * to ChunkedWriteCallback.h for a high level API which will merge status codes for chunked write requests.
     */
    CHIP_ERROR PutPreencodedAttribute(const ConcreteDataAttributePath & attributePath, const TLV::TLVReader & data);

    /**
     *  Once SendWriteRequest returns successfully, the WriteClient will
     *  handle calling Shutdown on itself once it decides it's done with waiting
     *  for a response (i.e. times out or gets a response). Client can specify
     *  the maximum time to wait for response (in milliseconds) via timeout parameter.
     *  If the timeout is missing or is set to System::Clock::kZero, a value based on the MRP timeouts of the session will be used.
     *  If SendWriteRequest is never called, or the call fails, the API
     *  consumer is responsible for calling Shutdown on the WriteClient.
     */
    CHIP_ERROR SendWriteRequest(const SessionHandle & session, System::Clock::Timeout timeout = System::Clock::kZero);

private:
    friend class TestWriteInteraction;
    friend class InteractionModelEngine;

    enum class State
    {
        Initialized = 0,     // The client has been initialized
        AddAttribute,        // The client has added attribute and ready for a SendWriteRequest
        AwaitingTimedStatus, // Sent a Tiemd Request, waiting for response.
        AwaitingResponse,    // The client has sent out the write request message
        ResponseReceived,    // We have gotten a response after sending write request
        AwaitingDestruction, // The object has completed its work and is awaiting destruction by the application.
    };

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    void MoveToState(const State aTargetState);
    CHIP_ERROR ProcessWriteResponseMessage(System::PacketBufferHandle && payload);
    CHIP_ERROR ProcessAttributeStatusIB(AttributeStatusIB::Parser & aAttributeStatusIB);
    const char * GetStateStr() const;

    /**
     *  Encode an attribute value that can be directly encoded using DataModel::Encode.
     */
    template <class T, std::enable_if_t<!DataModel::IsFabricScoped<T>::value, int> = 0>
    CHIP_ERROR TryEncodeSingleAttributeDataIB(const ConcreteDataAttributePath & attributePath, const T & value)
    {
        chip::TLV::TLVWriter * writer = nullptr;

        ReturnErrorOnFailure(PrepareAttributeIB(attributePath));
        VerifyOrReturnError((writer = GetAttributeDataIBTLVWriter()) != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, chip::TLV::ContextTag(chip::app::AttributeDataIB::Tag::kData), value));
        ReturnErrorOnFailure(FinishAttributeIB());

        return CHIP_NO_ERROR;
    }

    template <class T, std::enable_if_t<DataModel::IsFabricScoped<T>::value, int> = 0>
    CHIP_ERROR TryEncodeSingleAttributeDataIB(const ConcreteDataAttributePath & attributePath, const T & value)
    {
        chip::TLV::TLVWriter * writer = nullptr;

        ReturnErrorOnFailure(PrepareAttributeIB(attributePath));
        VerifyOrReturnError((writer = GetAttributeDataIBTLVWriter()) != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(
            DataModel::EncodeForWrite(*writer, chip::TLV::ContextTag(chip::app::AttributeDataIB::Tag::kData), value));
        ReturnErrorOnFailure(FinishAttributeIB());

        return CHIP_NO_ERROR;
    }

    /**
     * A wrapper for TryEncodeSingleAttributeDataIB which will start a new chunk when failed with CHIP_ERROR_NO_MEMORY or
     * CHIP_ERROR_BUFFER_TOO_SMALL.
     */
    template <class T>
    CHIP_ERROR EncodeSingleAttributeDataIB(const ConcreteDataAttributePath & attributePath, const T & value)
    {
        TLV::TLVWriter backupWriter;

        mWriteRequestBuilder.GetWriteRequests().Checkpoint(backupWriter);

        // First attempt to write this attribute.
        CHIP_ERROR err = TryEncodeSingleAttributeDataIB(attributePath, value);
        if (err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL)
        {
            // If it failed with no memory, then we create a new chunk for it.
            mWriteRequestBuilder.GetWriteRequests().Rollback(backupWriter);
            ReturnErrorOnFailure(StartNewMessage());
            ReturnErrorOnFailure(TryEncodeSingleAttributeDataIB(attributePath, value));
        }
        else
        {
            ReturnErrorOnFailure(err);
        }

        return CHIP_NO_ERROR;
    }

    /**
     * Encode a preencoded attribute data, returns TLV encode error if the ramaining space of current chunk is too small for the
     * AttributeDataIB.
     */
    CHIP_ERROR TryPutSinglePreencodedAttributeWritePayload(const ConcreteDataAttributePath & attributePath,
                                                           const TLV::TLVReader & data);

    /**
     * Encode a preencoded attribute data, will try to create a new chunk when necessary.
     */
    CHIP_ERROR PutSinglePreencodedAttributeWritePayload(const ConcreteDataAttributePath & attributePath,
                                                        const TLV::TLVReader & data);

    CHIP_ERROR EnsureMessage();

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

    // Send our queued-up Write Request message.  Assumes the exchange is ready
    // and mPendingWriteData is populated.
    CHIP_ERROR SendWriteRequest();

    // Encodes the header of an AttributeDataIB, a special case for attributePath is its EndpointId can be kInvalidEndpointId, this
    // is used when sending group write requests.
    // TODO(#14935) Update AttributePathParams to support more list operations.
    CHIP_ERROR PrepareAttributeIB(const ConcreteDataAttributePath & attributePath);
    CHIP_ERROR FinishAttributeIB();
    TLV::TLVWriter * GetAttributeDataIBTLVWriter();

    /**
     * Create a new message (or a new chunk) for the write request.
     */
    CHIP_ERROR StartNewMessage();

    /**
     * Finalize Write Request Message TLV Builder and retrieve final data from tlv builder for later sending
     */
    CHIP_ERROR FinalizeMessage(bool aHasMoreChunks);

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeHolder mExchangeCtx;
    Callback * mpCallback = nullptr;
    State mState          = State::Initialized;
    System::PacketBufferTLVWriter mMessageWriter;
    WriteRequestMessage::Builder mWriteRequestBuilder;
    // TODO Maybe we should change PacketBufferTLVWriter so we can finalize it
    // but have it hold on to the buffer, and get the buffer from it later.
    // Then we could avoid this extra pointer-sized member.
    System::PacketBufferHandle mPendingWriteData;
    // If mTimedWriteTimeoutMs has a value, we are expected to do a timed
    // write.
    Optional<uint16_t> mTimedWriteTimeoutMs;
    bool mSuppressResponse = false;

    // A list of buffers, one buffer for each chunk.
    System::PacketBufferHandle mChunks;

    // TODO: This file might be compiled with different build flags on Darwin platform (when building WriteClient.cpp and
    // CHIPClustersObjc.mm), which will cause undefined behavior when building write requests. Uncomment the #if and #endif after
    // resolving it.
    // #if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    uint16_t mReservedSize = 0;
    // #endif

    /**
     * Below we define several const variables for encoding overheads.
     * WriteRequestMessage =
     * {
     *  timedRequest = false,
     *  AttributeDataIBs =
     *  [
     *     AttributeDataIB =             \
     *     {                              |
     *        DataVersion = 0x0,          |
     *        AttributePathIB =           |
     *        {                           |
     *           Endpoint = 0x2,          |  "atomically" encoded via
     *           Cluster = 0x50f,          > EncodeAttribute or
     *           Attribute = 0x0000_0006, |  PutPreencodedAttribute
     *           ListIndex = Null,        |
     *        }                           |
     *        Data = ...                  |
     *     },                             /
     *     (...)
     *  ],                           <-- 1 byte  "end of AttributeDataIB" (end of container)
     *  moreChunkedMessages = false, <-- 2 bytes "kReservedSizeForMoreChunksFlag"
     *  InteractionModelRevision = 1,<-- 3 bytes "kReservedSizeForIMRevision"
     * }                             <-- 1 byte  "end of WriteRequestMessage" (end of container)
     */

    // Reserved size for the MoreChunks boolean flag, which takes up 1 byte for the control tag and 1 byte for the context tag.
    static constexpr uint16_t kReservedSizeForMoreChunksFlag = 1 + 1;
    // End Of Container (0x18) uses one byte.
    static constexpr uint16_t kReservedSizeForEndOfContainer = 1;
    // Reserved size for the uint8_t InteractionModelRevision flag, which takes up 1 byte for the control tag and 1 byte for the
    // context tag, 1 byte for value
    static constexpr uint16_t kReservedSizeForIMRevision = 1 + 1 + 1;
    // Reserved buffer for TLV level overhead (the overhead for end of AttributeDataIBs (end of container), more chunks flag, end
    // of WriteRequestMessage (another end of container)).
    static constexpr uint16_t kReservedSizeForTLVEncodingOverhead = kReservedSizeForIMRevision + kReservedSizeForMoreChunksFlag +
        kReservedSizeForEndOfContainer + kReservedSizeForEndOfContainer;
    bool mHasDataVersion = false;
};

} // namespace app
} // namespace chip
