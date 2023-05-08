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
 *      A handler for incoming Invoke interactions.
 *
 *      Allows adding responses to be sent in an InvokeResponse: see the various
 *      "Add*" methods.
 *
 *      Allows adding the responses asynchronously.  See the documentation
 *      for the CommandHandler::Handle class below.
 *
 */

#pragma once

#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeHolder.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>

namespace chip {
namespace app {

class CommandHandler : public Messaging::ExchangeDelegate
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;

        /*
         * Method that signals to a registered callback that this object
         * has completed doing useful work and is now safe for release/destruction.
         */
        virtual void OnDone(CommandHandler & apCommandObj) = 0;

        /*
         * Upon processing of a CommandDataIB, this method is invoked to dispatch the command
         * to the right server-side handler provided by the application.
         */
        virtual void DispatchCommand(CommandHandler & apCommandObj, const ConcreteCommandPath & aCommandPath,
                                     TLV::TLVReader & apPayload) = 0;

        /*
         * Check to see if a command implementation exists for a specific
         * concrete command path.  If it does, Success will be returned.  If
         * not, one of UnsupportedEndpoint, UnsupportedCluster, or
         * UnsupportedCommand will be returned, depending on how the command
         * fails to exist.
         */
        virtual Protocols::InteractionModel::Status CommandExists(const ConcreteCommandPath & aCommandPath) = 0;
    };

    /**
     * Class that allows asynchronous command processing before sending a
     * response.  When such processing is desired:
     *
     * 1) Create a Handle initialized with the CommandHandler that delivered the
     *    incoming command.
     * 2) Ensure the Handle, or some Handle it's moved into via the move
     *    constructor or move assignment operator, remains alive during the
     *    course of the asynchronous processing.
     * 3) Ensure that the ConcreteCommandPath involved will be known when
     *    sending the response.
     * 4) When ready to send the response:
     *    * Ensure that no other Matter tasks are running in parallel (e.g. by
     *      running on the Matter event loop or holding the Matter stack lock).
     *    * Call Get() to get the CommandHandler.
     *    * Check that Get() did not return null.
     *    * Add the response to the CommandHandler via one of the Add* methods.
     *    * Let the Handle get destroyed, or manually call Handle::Release() if
     *      destruction of the Handle is not desirable for some reason.
     *
     * The Invoke Response will not be sent until all outstanding Handles have
     * been destroyed or have had Release called.
     */
    class Handle
    {
    public:
        Handle() {}
        Handle(const Handle & handle) = delete;
        Handle(Handle && handle)
        {
            mpHandler        = handle.mpHandler;
            mMagic           = handle.mMagic;
            handle.mpHandler = nullptr;
            handle.mMagic    = 0;
        }
        Handle(decltype(nullptr)) {}
        Handle(CommandHandler * handle);
        ~Handle() { Release(); }

        Handle & operator=(Handle && handle)
        {
            Release();
            mpHandler        = handle.mpHandler;
            mMagic           = handle.mMagic;
            handle.mpHandler = nullptr;
            handle.mMagic    = 0;
            return *this;
        }

        Handle & operator=(decltype(nullptr))
        {
            Release();
            return *this;
        }

        /**
         * Get the CommandHandler object it holds. Get() may return a nullptr if the CommandHandler object is holds is no longer
         * valid.
         */
        CommandHandler * Get();

        void Release();

    private:
        CommandHandler * mpHandler = nullptr;
        uint32_t mMagic            = 0;
    };

    /*
     * Constructor.
     *
     * The callback passed in has to outlive this CommandHandler object.
     */
    CommandHandler(Callback * apCallback);

    /*
     * Main entrypoint for this class to handle an invoke request.
     *
     * This function will always call the OnDone function above on the registered callback
     * before returning.
     *
     * isTimedInvoke is true if and only if this is part of a Timed Invoke
     * transaction (i.e. was preceded by a Timed Request).  If we reach here,
     * the timer verification has already been done.
     */
    void OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                System::PacketBufferHandle && payload, bool isTimedInvoke);
    CHIP_ERROR AddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus);

    CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus);

    CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus);

    Protocols::InteractionModel::Status ProcessInvokeRequest(System::PacketBufferHandle && payload, bool isTimedInvoke);
    CHIP_ERROR PrepareCommand(const ConcreteCommandPath & aCommandPath, bool aStartDataStruct = true);
    CHIP_ERROR FinishCommand(bool aEndDataStruct = true);
    CHIP_ERROR PrepareStatus(const ConcreteCommandPath & aCommandPath);
    CHIP_ERROR FinishStatus();
    TLV::TLVWriter * GetCommandDataIBTLVWriter();

    /**
     * GetAccessingFabricIndex() may only be called during synchronous command
     * processing.  Anything that runs async (while holding a
     * CommandHandler::Handle or equivalent) must not call this method, because
     * it will not work right if the session we're using was evicted.
     */
    FabricIndex GetAccessingFabricIndex() const;

    /**
     * API for adding a data response.  The template parameter T is generally
     * expected to be a ClusterName::Commands::CommandName::Type struct, but any
     * object that can be encoded using the DataModel::Encode machinery and
     * exposes the right command id will work.
     *
     * @param [in] aRequestCommandPath the concrete path of the command we are
     *             responding to.
     * @param [in] aData the data for the response.
     */
    template <typename CommandData>
    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, const CommandData & aData)
    {
        // TryAddResponseData will ensure we are in the correct state when calling AddResponseData.
        CHIP_ERROR err = TryAddResponseData(aRequestCommandPath, aData);
        if (err != CHIP_NO_ERROR)
        {
            // The state guarantees that either we can rollback or we don't have to rollback the buffer, so we don't care about the
            // return value of RollbackResponse.
            RollbackResponse();
        }
        return err;
    }

    /**
     * API for adding a response.  This will try to encode a data response (response command), and if that fails will encode a a
     * Protocols::InteractionModel::Status::Failure status response instead.
     *
     * The template parameter T is generally expected to be a ClusterName::Commands::CommandName::Type struct, but any object that
     * can be encoded using the DataModel::Encode machinery and exposes the right command id will work.
     *
     * Since the function will call AddStatus when it fails to encode the data, it cannot send any response when it fails to encode
     * a status code since another AddStatus call will also fail. The error from AddStatus will just be logged.
     *
     * @param [in] aRequestCommandPath the concrete path of the command we are
     *             responding to.
     * @param [in] aData the data for the response.
     */
    template <typename CommandData>
    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, const CommandData & aData)
    {
        if (CHIP_NO_ERROR != AddResponseData(aRequestCommandPath, aData))
        {
            CHIP_ERROR err = AddStatus(aRequestCommandPath, Protocols::InteractionModel::Status::Failure);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement, "Failed to encode status: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
    }

    /**
     * Check whether the InvokeRequest we are handling is a timed invoke.
     */
    bool IsTimedInvoke() const { return mTimedRequest; }

    /**
     * Gets the inner exchange context object, without ownership.
     *
     * WARNING: This is dangerous, since it is directly interacting with the
     *          exchange being managed automatically by mExchangeCtx and
     *          if not done carefully, may end up with use-after-free errors.
     *
     * @return The inner exchange context, might be nullptr if no
     *         exchange context has been assigned or the context
     *         has been released.
     */
    Messaging::ExchangeContext * GetExchangeContext() const { return mExchangeCtx.Get(); }

    /**
     * @brief Flush acks right away for a slow command
     *
     * Some commands that do heavy lifting of storage/crypto should
     * ack right away to improve reliability and reduce needless retries. This
     * method can be manually called in commands that are especially slow to
     * immediately schedule an acknowledgement (if needed) since the delayed
     * stand-alone ack timer may actually not hit soon enough due to blocking command
     * execution.
     *
     */
    void FlushAcksRightAwayOnSlowCommand()
    {
        VerifyOrReturn(mExchangeCtx);
        auto * msgContext = mExchangeCtx->GetReliableMessageContext();
        VerifyOrReturn(msgContext != nullptr);
        msgContext->FlushAcks();
    }

    /**
     * GetSubjectDescriptor() may only be called during synchronous command
     * processing.  Anything that runs async (while holding a
     * CommandHandler::Handle or equivalent) must not call this method, because
     * it might not work right if the session we're using was evicted.
     */
    Access::SubjectDescriptor GetSubjectDescriptor() const
    {
        VerifyOrDie(!mGoneAsync);
        return mExchangeCtx->GetSessionHandle()->GetSubjectDescriptor();
    }

private:
    friend class TestCommandInteraction;
    friend class CommandHandler::Handle;

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override
    {
        //
        // We're not expecting responses to any messages we send out on this EC.
        //
        VerifyOrDie(false);
    }

    enum class State
    {
        Idle,                ///< Default state that the object starts out in, where no work has commenced
        Preparing,           ///< We are prepaing the command or status header.
        AddingCommand,       ///< In the process of adding a command.
        AddedCommand,        ///< A command has been completely encoded and is awaiting transmission.
        CommandSent,         ///< The command has been sent successfully.
        AwaitingDestruction, ///< The object has completed its work and is awaiting destruction by the application.
    };

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    /**
     * Rollback the state to before encoding the current ResponseData (before calling PrepareCommand / PrepareStatus)
     */
    CHIP_ERROR RollbackResponse();

    /*
     * This forcibly closes the exchange context if a valid one is pointed to. Such a situation does
     * not arise during normal message processing flows that all normally call Close() above. This can only
     * arise due to application-initiated destruction of the object when this object is handling receiving/sending
     * message payloads.
     */
    void Abort();

    /**
     * IncrementHoldOff will increase the inner refcount of the CommandHandler.
     *
     * Users should use CommandHandler::Handle for management the lifespan of the CommandHandler.
     * DefRef should be released in reasonable time, and Close() should only be called when the refcount reached 0.
     */
    void IncrementHoldOff();

    /**
     * DecrementHoldOff is used by CommandHandler::Handle for decreasing the refcount of the CommandHandler.
     * When refcount reached 0, CommandHandler will send the response to the peer and shutdown.
     */
    void DecrementHoldOff();

    /*
     * Allocates a packet buffer used for encoding an invoke response payload.
     *
     * This can be called multiple times safely, as it will only allocate the buffer once for the lifetime
     * of this object.
     */
    CHIP_ERROR AllocateBuffer();

    CHIP_ERROR Finalize(System::PacketBufferHandle & commandPacket);

    /**
     * Called internally to signal the completion of all work on this object, gracefully close the
     * exchange (by calling into the base class) and finally, signal to a registerd callback that it's
     * safe to release this object.
     */
    void Close();

    /**
     * ProcessCommandDataIB is only called when a unicast invoke command request is received
     * It requires the endpointId in its command path to be able to dispatch the command
     */
    Protocols::InteractionModel::Status ProcessCommandDataIB(CommandDataIB::Parser & aCommandElement);

    /**
     * ProcessGroupCommandDataIB is only called when a group invoke command request is received
     * It doesn't need the endpointId in it's command path since it uses the GroupId in message metadata to find it
     */
    Protocols::InteractionModel::Status ProcessGroupCommandDataIB(CommandDataIB::Parser & aCommandElement);
    CHIP_ERROR SendCommandResponse();
    CHIP_ERROR AddStatusInternal(const ConcreteCommandPath & aCommandPath, const StatusIB & aStatus);

    /**
     * If this function fails, it may leave our TLV buffer in an inconsistent state.  Callers should snapshot as needed before
     * calling this function, and roll back as needed afterward.
     *
     * @param [in] aRequestCommandPath the concrete path of the command we are
     *             responding to.
     * @param [in] aData the data for the response.
     */
    template <typename CommandData>
    CHIP_ERROR TryAddResponseData(const ConcreteCommandPath & aRequestCommandPath, const CommandData & aData)
    {
        ConcreteCommandPath path = { aRequestCommandPath.mEndpointId, aRequestCommandPath.mClusterId, CommandData::GetCommandId() };
        ReturnErrorOnFailure(PrepareCommand(path, false));
        TLV::TLVWriter * writer = GetCommandDataIBTLVWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, TLV::ContextTag(CommandDataIB::Tag::kFields), aData));

        return FinishCommand(/* aEndDataStruct = */ false);
    }

    Messaging::ExchangeHolder mExchangeCtx;
    Callback * mpCallback = nullptr;
    InvokeResponseMessage::Builder mInvokeResponseBuilder;
    TLV::TLVType mDataElementContainerType = TLV::kTLVType_NotSpecified;
    size_t mPendingWork                    = 0;
    bool mSuppressResponse                 = false;
    bool mTimedRequest                     = false;

    bool mSentStatusResponse = false;

    State mState = State::Idle;
    chip::System::PacketBufferTLVWriter mCommandMessageWriter;
    TLV::TLVWriter mBackupWriter;
    bool mBufferAllocated = false;
    // If mGoneAsync is true, we have finished out initial processing of the
    // incoming invoke.  After this point, our session could go away at any
    // time.
    bool mGoneAsync = false;
};

} // namespace app
} // namespace chip
