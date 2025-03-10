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

#include <app/AppConfig.h>
#include <app/AttributeAccessToken.h>
#include <app/AttributePathParams.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelDelegatePointers.h>
#include <app/MessageDef/WriteResponseMessage.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/Provider.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeHolder.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace app {

class WriteHandler;

class WriteHandlerDelegate
{
public:
    virtual ~WriteHandlerDelegate() = default;

    /**
     * Returns whether the write operation to the given path is in conflict with another write operation.
     * (i.e. another write transaction is in the middle of processing a chunked write to the given path.)
     */
    virtual bool HasConflictWriteRequests(const WriteHandler * apWriteHandler, const ConcreteAttributePath & aPath) = 0;
};

/**
 *  @brief The write handler is responsible for processing a write request and sending a write reply.
 */
class WriteHandler : public Messaging::ExchangeDelegate
{
public:
    WriteHandler() : mExchangeCtx(*this) {}

    /**
     *  Initialize the WriteHandler. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Close is made to terminate the
     *  instance.
     *
     *  @param[in] apProvider              A valid pointer to the model used to forward writes towards
     *  @param[in] apWriteHandlerDelegate  A Valid pointer to the WriteHandlerDelegate.
     *
     *  @retval #CHIP_ERROR_INVALID_ARGUMENT on invalid pointers
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(DataModel::Provider * apProvider, WriteHandlerDelegate * apWriteHandlerDelegate);

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

    /**
     *  Clean up state when we are done sending the write response.
     */
    void Close();

    bool IsFree() const { return mState == State::Uninitialized; }

    ~WriteHandler() override = default;

    CHIP_ERROR ProcessAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader);
    CHIP_ERROR ProcessGroupAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader);

    CHIP_ERROR AddStatus(const ConcreteDataAttributePath & aPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus);
    CHIP_ERROR AddStatus(const ConcreteDataAttributePath & aPath, const Protocols::InteractionModel::Status aStatus)
    {
        return AddStatus(aPath, Protocols::InteractionModel::ClusterStatusCode{ aStatus });
    }

    CHIP_ERROR AddClusterSpecificSuccess(const ConcreteDataAttributePath & aAttributePathParams, ClusterStatus aClusterStatus);
    CHIP_ERROR AddClusterSpecificFailure(const ConcreteDataAttributePath & aAttributePathParams, ClusterStatus aClusterStatus);

    FabricIndex GetAccessingFabricIndex() const;

    /**
     * Check whether the WriteRequest we are handling is a timed write.
     */
    bool IsTimedWrite() const { return mStateFlags.Has(StateBits::kIsTimedRequest); }

    bool MatchesExchangeContext(Messaging::ExchangeContext * apExchangeContext) const
    {
        return !IsFree() && mExchangeCtx.Get() == apExchangeContext;
    }

    void CacheACLCheckResult(const AttributeAccessToken & aToken) { mACLCheckCache.SetValue(aToken); }

    bool ACLCheckCacheHit(const AttributeAccessToken & aToken)
    {
        return mACLCheckCache.HasValue() && mACLCheckCache.Value() == aToken;
    }

    bool IsCurrentlyProcessingWritePath(const ConcreteAttributePath & aPath)
    {
        return mProcessingAttributePath.HasValue() && mProcessingAttributePath.Value() == aPath;
    }

private:
    friend class TestWriteInteraction;
    enum class State : uint8_t
    {
        Uninitialized = 0, // The handler has not been initialized
        Initialized,       // The handler has been initialized and is ready
        AddStatus,         // The handler has added status code
        Sending,           // The handler has sent out the write response
    };
    using Status = Protocols::InteractionModel::Status;
    Status ProcessWriteRequest(System::PacketBufferHandle && aPayload, bool aIsTimedWrite);
    Status HandleWriteRequestMessage(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload,
                                     bool aIsTimedWrite);

    CHIP_ERROR FinalizeMessage(System::PacketBufferTLVWriter && aMessageWriter, System::PacketBufferHandle & packet);
    CHIP_ERROR SendWriteResponse(System::PacketBufferTLVWriter && aMessageWriter);

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    void DeliverListWriteBegin(const ConcreteAttributePath & aPath);
    void DeliverListWriteEnd(const ConcreteAttributePath & aPath, bool writeWasSuccessful);

    // Deliver the signal that we have delivered all list entries to the AttributeAccessInterface. This function will be called
    // after handling the last chunk of a series of write requests. Or the write handler was shutdown (usually due to transport
    // timeout).
    // This function will become no-op on group writes, since DeliverFinalListWriteEndForGroupWrite will clear the
    // mProcessingAttributePath after processing the AttributeDataIBs from the request.
    void DeliverFinalListWriteEnd(bool writeWasSuccessful);

    // Deliver the signal that we have delivered all list entries to the AttributeAccessInterface. This function will be called
    // after handling the last attribute in a group write request (since group writes will never be chunked writes). Or we failed to
    // process the group write request (usually due to malformed messages). This function should only be called by
    // ProcessGroupAttributeDataIBs.
    CHIP_ERROR DeliverFinalListWriteEndForGroupWrite(bool writeWasSuccessful);

    CHIP_ERROR AddStatusInternal(const ConcreteDataAttributePath & aPath, const StatusIB & aStatus);

    // ExchangeDelegate
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    /// Validate that a write is acceptable on the given path.
    ///
    /// Validates that ACL, writability and Timed interaction settings are ok.
    ///
    /// Returns a success status if all is ok, failure otherwise.
    DataModel::ActionReturnStatus CheckWriteAllowed(const Access::SubjectDescriptor & aSubject,
                                                    const ConcreteAttributePath & aPath);

    // Write the given data to the given path
    CHIP_ERROR WriteClusterData(const Access::SubjectDescriptor & aSubject, const ConcreteDataAttributePath & aPath,
                                TLV::TLVReader & aData);

    /// Checks whether the given path corresponds to a list attribute
    /// Return values:
    ///    true/false: valid attribute path, known if list or not
    ///    std::nulloptr - path not available/valid, unknown if attribute is a list or not
    std::optional<bool> IsListAttributePath(const ConcreteAttributePath & path);

    Messaging::ExchangeHolder mExchangeCtx;
    WriteResponseMessage::Builder mWriteResponseBuilder;
    Optional<ConcreteAttributePath> mProcessingAttributePath;
    Optional<AttributeAccessToken> mACLCheckCache = NullOptional;

    DataModel::Provider * mDataModelProvider = nullptr;
    std::optional<ConcreteAttributePath> mLastSuccessfullyWrittenPath;

    // This may be a "fake" pointer or a real delegate pointer, depending
    // on CHIP_CONFIG_STATIC_GLOBAL_INTERACTION_MODEL_ENGINE setting.
    //
    // When this is not a real pointer, it checks that the value is always
    // set to the global InteractionModelEngine and the size of this
    // member is 1 byte.
    InteractionModelDelegatePointer<WriteHandlerDelegate> mDelegate;

    // bit level enums to save storage for this object. InteractionModelEngine maintains
    // several of these objects, so every bit of storage multiplies storage usage.
    enum class StateBits : uint8_t
    {
        kIsTimedRequest            = 0x01,
        kSuppressResponse          = 0x02,
        kHasMoreChunks             = 0x04,
        kProcessingAttributeIsList = 0x08,
        // We record the Status when AddStatus is called to determine whether all data of a list write is accepted.
        // This value will be used by DeliverListWriteEnd and DeliverFinalListWriteEnd but it won't be used by group writes based on
        // the fact that the errors that won't be delivered to AttributeAccessInterface are:
        //  (1) Attribute not found
        //  (2) Access control failed
        //  (3) Write request to a read-only attribute
        //  (4) Data version mismatch
        //  (5) Not using timed write.
        //  Where (1)-(3) will be consistent among the whole list write request, while (4) and (5) are not appliable to group
        //  writes.
        kAttributeWriteSuccessful = 0x10,
    };

    BitFlags<StateBits> mStateFlags;
    State mState = State::Uninitialized;
};

} // namespace app
} // namespace chip
