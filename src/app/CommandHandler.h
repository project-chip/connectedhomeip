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

#include "CommandPathRegistry.h"
#include "CommandResponseSender.h"

#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Scoped.h>
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

class CommandHandler
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

    // Previously we kept adding arguments with default values individually as parameters. This is because there
    // is legacy code outside of the SDK that would call PrepareCommand. With the new PrepareInvokeResponseCommand
    // replacing PrepareCommand, we took this opportunity to create a new parameter structure to make it easier to
    // add new parameters without there needing to be an ever increasing parameter list with defaults.
    struct InvokeResponseParameters
    {
        InvokeResponseParameters(ConcreteCommandPath aRequestCommandPath) : mRequestCommandPath(aRequestCommandPath) {}

        InvokeResponseParameters & SetStartOrEndDataStruct(bool aStartOrEndDataStruct)
        {
            mStartOrEndDataStruct = aStartOrEndDataStruct;
            return *this;
        }

        ConcreteCommandPath mRequestCommandPath;
        /**
         * Whether the method this is being provided to should start/end the TLV container for the CommandFields element
         * within CommandDataIB.
         */
        bool mStartOrEndDataStruct = true;
    };

    class TestOnlyMarker
    {
    };

    /*
     * Constructor.
     *
     * The callback passed in has to outlive this CommandHandler object.
     */
    CommandHandler(Callback * apCallback);

    /*
     * Constructor to override number of supported paths per invoke.
     *
     * The callback and command path registry passed in has to outlive this CommandHandler object.
     * For testing purposes.
     */
    CommandHandler(TestOnlyMarker aTestMarker, Callback * apCallback, CommandPathRegistry * apCommandPathRegistry);

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

    /**
     * Checks that all CommandDataIB within InvokeRequests satisfy the spec's general
     * constraints for CommandDataIB. Additionally checks that InvokeRequestMessage is
     * properly formatted.
     *
     * This also builds a registry that to ensure that all commands can be responded
     * to with the data required as per spec.
     */
    CHIP_ERROR ValidateInvokeRequestMessageAndBuildRegistry(InvokeRequestMessage::Parser & invokeRequestMessage);

    /**
     * Adds the given command status and returns any failures in adding statuses (e.g. out
     * of buffer space) to the caller
     */
    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus,
                                 const char * context = nullptr);

    /**
     * Adds a status when the caller is unable to handle any failures. Logging is performed
     * and failure to register the status is checked with VerifyOrDie.
     */
    void AddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus,
                   const char * context = nullptr);

    CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus);

    CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus);

    Protocols::InteractionModel::Status ProcessInvokeRequest(System::PacketBufferHandle && payload, bool isTimedInvoke);

    /**
     * This adds a new CommandDataIB element into InvokeResponses for the associated
     * aRequestCommandPath. This adds up until the `CommandFields` element within
     * `CommandDataIB`.
     *
     * This call will fail if CommandHandler is already in the middle of building a
     * CommandStatusIB or CommandDataIB (i.e. something has called Prepare*, without
     * calling Finish*), or is already sending InvokeResponseMessage.
     *
     * Upon success, the caller is expected to call `FinishCommand` once they have added
     * all the fields into the CommandFields element of CommandDataIB.
     *
     * @param [in] aResponseCommandPath the concrete response path that we are sending to Requester.
     * @param [in] aPrepareParameters struct containing paramters needs for preparing a command. Data
     *             such as request path, and whether this method should start the CommandFields element within
     *             CommandDataIB.
     */
    CHIP_ERROR PrepareInvokeResponseCommand(const ConcreteCommandPath & aResponseCommandPath,
                                            const InvokeResponseParameters & aPrepareParameters);

    [[deprecated("PrepareCommand now needs the requested command path. Please use PrepareInvokeResponseCommand")]] CHIP_ERROR
    PrepareCommand(const ConcreteCommandPath & aCommandPath, bool aStartDataStruct = true);

    /**
     * Finishes the CommandDataIB element within the InvokeResponses.
     *
     * Caller must have first successfully called `PrepareInvokeResponseCommand`.
     *
     * @param [in] aEndDataStruct end the TLV container for the CommandFields element within
     *             CommandDataIB. This should match the boolean passed into Prepare*.
     *
     * @return CHIP_ERROR_INCORRECT_STATE
     *                      If device has not previously successfully called
     *                      `PrepareInvokeResponseCommand`.
     * @return CHIP_ERROR_BUFFER_TOO_SMALL
     *                      If writing the values needed to finish the InvokeReponseIB
     *                      with the current contents of the InvokeResponseMessage
     *                      would exceed the limit. When this error occurs, it is possible
     *                      we have already closed some of the IB Builders that were
     *                      previously started in `PrepareInvokeResponseCommand`.
     * @return CHIP_ERROR_NO_MEMORY
     *                      If TLVWriter attempted to allocate an output buffer failed due to
     *                      lack of memory.
     * @return other        Other TLVWriter related errors. Typically occurs if
     *                      `GetCommandDataIBTLVWriter()` was called and used incorrectly.
     */
    // TODO(#30453): We should be able to eliminate the chances of OOM issues with reserve.
    // This will be completed in a follow up PR.
    CHIP_ERROR FinishCommand(bool aEndDataStruct = true);

    TLV::TLVWriter * GetCommandDataIBTLVWriter();

    /**
     * GetAccessingFabricIndex() may only be called during synchronous command
     * processing.  Anything that runs async (while holding a
     * CommandHandler::Handle or equivalent) must not call this method, because
     * it will not work right if the session we're using was evicted.
     */
    FabricIndex GetAccessingFabricIndex() const;

    /**
     * @brief Best effort to add InvokeResponse to InvokeResponseMessage.
     *
     * Tries to add response using lambda. Upon failure to add response, attempts
     * to rollback the InvokeResponseMessage to a known good state. If failure is due
     * to insufficient space in the current InvokeResponseMessage:
     *  - Finalizes the current InvokeResponseMessage.
     *  - Allocates a new InvokeResponseMessage.
     *  - Reattempts to add the InvokeResponse to the new InvokeResponseMessage.
     *
     * @param [in] addResponseFunction A lambda function responsible for adding the
     *             response to the current InvokeResponseMessage.
     */
    template <typename Function>
    CHIP_ERROR TryAddingResponse(Function && addResponseFunction)
    {
        // Invalidate any existing rollback backups. The addResponseFunction is
        // expected to create a new backup during either PrepareInvokeResponseCommand
        // or PrepareStatus execution. Direct invocation of
        // CreateBackupForResponseRollback is avoided since the buffer used by
        // InvokeResponseMessage might not be allocated until a Prepare* function
        // is called.
        mRollbackBackupValid = false;
        CHIP_ERROR err       = addResponseFunction();
        if (err == CHIP_NO_ERROR)
        {
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(RollbackResponse());
        // If we failed to add a command due to lack of space in the
        // packet, we will make another attempt to add the response using
        // an additional InvokeResponseMessage.
        if (mState != State::AddedCommand || err != CHIP_ERROR_NO_MEMORY)
        {
            return err;
        }
        ReturnErrorOnFailure(FinalizeInvokeResponseMessageAndPrepareNext());
        err = addResponseFunction();
        if (err != CHIP_NO_ERROR)
        {
            // The return value of RollbackResponse is ignored, as we prioritize
            // conveying the error generated by addResponseFunction to the
            // caller.
            RollbackResponse();
        }
        return err;
    }

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
        return TryAddingResponse([&]() -> CHIP_ERROR { return TryAddResponseData(aRequestCommandPath, aData); });
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
        if (AddResponseData(aRequestCommandPath, aData) != CHIP_NO_ERROR)
        {
            AddStatus(aRequestCommandPath, Protocols::InteractionModel::Status::Failure);
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
     *          exchange being managed automatically by mResponseSender and
     *          if not done carefully, may end up with use-after-free errors.
     *
     * @return The inner exchange context, might be nullptr if no
     *         exchange context has been assigned or the context
     *         has been released.
     */
    Messaging::ExchangeContext * GetExchangeContext() const { return mResponseSender.GetExchangeContext(); }

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
    void FlushAcksRightAwayOnSlowCommand() { mResponseSender.FlushAcksRightNow(); }

    /**
     * GetSubjectDescriptor() may only be called during synchronous command
     * processing.  Anything that runs async (while holding a
     * CommandHandler::Handle or equivalent) must not call this method, because
     * it might not work right if the session we're using was evicted.
     */
    Access::SubjectDescriptor GetSubjectDescriptor() const
    {
        VerifyOrDie(!mGoneAsync);
        return mResponseSender.GetSubjectDescriptor();
    }

#if CHIP_WITH_NLFAULTINJECTION

    enum class NlFaultInjectionType : uint8_t
    {
        SeparateResponseMessages,
        SeparateResponseMessagesAndInvertedResponseOrder,
        SkipSecondResponse
    };

    /**
     * @brief Sends InvokeResponseMessages with injected faults for certification testing.
     *
     * The Test Harness (TH) uses this to simulate various server response behaviors,
     * ensuring the Device Under Test (DUT) handles responses per specification.
     *
     * This function strictly validates the DUT's InvokeRequestMessage against the test plan.
     * If deviations occur, the TH terminates with a detailed error message.
     *
     * @param ec Exchange context for sending InvokeResponseMessages to the client.
     * @param payload Payload of the incoming InvokeRequestMessage from the client.
     * @param isTimedInvoke Indicates whether the interaction is timed.
     * @param faultType The specific type of fault to inject into the response.
     */
    // TODO(#30453): After refactoring CommandHandler for better unit testability, create a
    // unit test specifically for the fault injection behavior.
    void TestOnlyInvokeCommandRequestWithFaultsInjected(Messaging::ExchangeContext * ec, System::PacketBufferHandle && payload,
                                                        bool isTimedInvoke, NlFaultInjectionType faultType);
#endif // CHIP_WITH_NLFAULTINJECTION

private:
    friend class TestCommandInteraction;
    friend class CommandHandler::Handle;

    enum class State : uint8_t
    {
        Idle,                ///< Default state that the object starts out in, where no work has commenced
        NewResponseMessage,  ///< mInvokeResponseBuilder is ready, with no responses added.
        Preparing,           ///< We are prepaing the command or status header.
        AddingCommand,       ///< In the process of adding a command.
        AddedCommand,        ///< A command has been completely encoded and is awaiting transmission.
        DispatchResponses,   ///< The command response(s) are being dispatched.
        AwaitingDestruction, ///< The object has completed its work and is awaiting destruction by the application.
    };

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    /**
     * Create a backup to enable rolling back to the state prior to ResponseData encoding in the event of failure.
     */
    void CreateBackupForResponseRollback();

    /**
     * Rollback the state to before encoding the current ResponseData (before calling PrepareInvokeResponseCommand / PrepareStatus)
     *
     * Requires CreateBackupForResponseRollback to be called at the start of PrepareInvokeResponseCommand / PrepareStatus
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

    /**
     * This will add a new CommandStatusIB element into InvokeResponses. It will put the
     * aCommandPath into the CommandPath element within CommandStatusIB.
     *
     * This call will fail if CommandHandler is already in the middle of building a
     * CommandStatusIB or CommandDataIB (i.e. something has called Prepare*, without
     * calling Finish*), or is already sending InvokeResponseMessage.
     *
     * Upon success, the caller is expected to call `FinishStatus` once they have encoded
     * StatusIB.
     *
     * @param [in] aCommandPath the concrete path of the command we are responding to.
     */
    CHIP_ERROR PrepareStatus(const ConcreteCommandPath & aCommandPath);

    /**
     * Finishes the CommandStatusIB element within the InvokeResponses.
     *
     * Caller must have first successfully called `PrepareStatus`.
     */
    CHIP_ERROR FinishStatus();

    CHIP_ERROR PrepareInvokeResponseCommand(const CommandPathRegistryEntry & apCommandPathRegistryEntry,
                                            const ConcreteCommandPath & aCommandPath, bool aStartDataStruct);

    CHIP_ERROR FinalizeLastInvokeResponseMessage() { return FinalizeInvokeResponseMessage(/* aHasMoreChunks = */ false); }

    CHIP_ERROR FinalizeInvokeResponseMessageAndPrepareNext();

    CHIP_ERROR FinalizeInvokeResponseMessage(bool aHasMoreChunks);

    /**
     * Called internally to signal the completion of all work on this object, gracefully close the
     * exchange (by calling into the base class) and finally, signal to a registerd callback that it's
     * safe to release this object.
     */
    void Close();

    /**
     * @brief Callback method invoked when CommandResponseSender has finished sending all messages.
     */
    static void HandleOnResponseSenderDone(void * context);

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
    CHIP_ERROR StartSendingCommandResponses();

    CHIP_ERROR TryAddStatusInternal(const ConcreteCommandPath & aCommandPath, const StatusIB & aStatus);

    CHIP_ERROR AddStatusInternal(const ConcreteCommandPath & aCommandPath, const StatusIB & aStatus);

    /**
     * Non-templated function called before DataModel::Encode when attempting to add a response,
     * which does all the work needed before encoding the actual type-dependent data into the buffer.
     *
     * **Important:** If this function fails, the TLV buffer may be left in an inconsistent state.
     * Callers should create snapshots as necessary before invoking this function and implement
     * rollback mechanisms if needed.
     *
     * **Usage:** This function is intended to be called exclusively by TryAddResponseData. It was
     * factored out to optimize code size.
     *
     * @param aRequestCommandPath  The concrete path of the command being responded to.
     * @param aResponseCommandPath The concrete path of the command response.
     */
    CHIP_ERROR TryAddResponseDataPreEncode(const ConcreteCommandPath & aRequestCommandPath,
                                           const ConcreteCommandPath & aResponseCommandPath)
    {
        // Return early in case of requests targeted to a group, since they should not add a response.
        VerifyOrReturnValue(!IsGroupRequest(), CHIP_NO_ERROR);

        InvokeResponseParameters prepareParams(aRequestCommandPath);
        prepareParams.SetStartOrEndDataStruct(false);

        ScopedChange<bool> internalCallToAddResponse(mInternalCallToAddResponseData, true);
        return PrepareInvokeResponseCommand(aResponseCommandPath, prepareParams);
    }

    // TODO(#31627): It would be awesome if we could remove this template all together.
    /**
     * If this function fails, it may leave our TLV buffer in an inconsistent state.
     * Callers should snapshot as needed before calling this function, and roll back
     * as needed afterward.
     *
     * @param [in] aRequestCommandPath the concrete path of the command we are
     *             responding to.
     * @param [in] aData the data for the response.
     */
    template <typename CommandData>
    CHIP_ERROR TryAddResponseData(const ConcreteCommandPath & aRequestCommandPath, const CommandData & aData)
    {
        // This method, templated with CommandData, captures all the components needs
        // from CommandData with as little code as possible.
        //
        // Previously, non-essential code was unnecessarily templated, leading to
        // compilation and duplication N times. By isolating only the code segments
        // that genuinely require templating, minimizes duplicate compiled code.
        ConcreteCommandPath responseCommandPath = { aRequestCommandPath.mEndpointId, aRequestCommandPath.mClusterId,
                                                    CommandData::GetCommandId() };
        ReturnErrorOnFailure(TryAddResponseDataPreEncode(aRequestCommandPath, responseCommandPath));
        TLV::TLVWriter * writer = GetCommandDataIBTLVWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, TLV::ContextTag(CommandDataIB::Tag::kFields), aData));

        // FinishCommand technically should be refactored out as it is not a command that needs templating.
        // But, because there is only a single function call, keeping it here takes less code. If there is
        // ever more code between DataModel::Encode and the end of this function, it should be broken out into
        // TryAddResponseDataPostEncode.
        return FinishCommand(/* aEndDataStruct = */ false);
    }

    /**
     * Check whether the InvokeRequest we are handling is targeted to a group.
     */
    bool IsGroupRequest() { return mGroupRequest; }

    /**
     * Sets the state flag to keep the information that request we are handling is targeted to a group.
     */
    void SetGroupRequest(bool isGroupRequest) { mGroupRequest = isGroupRequest; }

    CommandPathRegistry & GetCommandPathRegistry() const { return *mCommandPathRegistry; }

    size_t MaxPathsPerInvoke() const { return mMaxPathsPerInvoke; }

    Callback * mpCallback = nullptr;
    InvokeResponseMessage::Builder mInvokeResponseBuilder;
    TLV::TLVType mDataElementContainerType = TLV::kTLVType_NotSpecified;
    size_t mPendingWork                    = 0;

    chip::System::PacketBufferTLVWriter mCommandMessageWriter;
    TLV::TLVWriter mBackupWriter;
    size_t mMaxPathsPerInvoke = CHIP_CONFIG_MAX_PATHS_PER_INVOKE;
    // TODO(#30453): See if we can reduce this size for the default cases
    // TODO Allow flexibility in registration.
    BasicCommandPathRegistry<CHIP_CONFIG_MAX_PATHS_PER_INVOKE> mBasicCommandPathRegistry;
    CommandPathRegistry * mCommandPathRegistry = &mBasicCommandPathRegistry;
    Optional<uint16_t> mRefForResponse;

    chip::Callback::Callback<OnResponseSenderDone> mResponseSenderDone;
    CommandResponseSender mResponseSender;

    State mState = State::Idle;
    State mBackupState;
    ScopedChangeOnly<bool> mInternalCallToAddResponseData{ false };
    bool mSuppressResponse                 = false;
    bool mTimedRequest                     = false;
    bool mSentStatusResponse               = false;
    bool mGroupRequest                     = false;
    bool mBufferAllocated                  = false;
    bool mReserveSpaceForMoreChunkMessages = false;
    // TODO(#30453): We should introduce breaking change where calls to add CommandData
    // need to use AddResponse, and not CommandHandler primitives directly using
    // GetCommandDataIBTLVWriter.
    bool mRollbackBackupValid = false;
    // If mGoneAsync is true, we have finished out initial processing of the
    // incoming invoke.  After this point, our session could go away at any
    // time.
    bool mGoneAsync = false;
};

} // namespace app
} // namespace chip
