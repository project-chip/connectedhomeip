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
 *      This file defines objects for a CHIP IM Invoke Command Sender
 *
 */

#pragma once

#include <type_traits>

#include "CommandSenderLegacyCallback.h"

#include <app/CommandPathParams.h>
#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>
#include <app/MessageDef/StatusIB.h>
#include <app/PendingResponseTrackerImpl.h>
#include <app/data-model/EncodableToTLV.h>
#include <app/data-model/Encode.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeHolder.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#define COMMON_STATUS_SUCCESS 0

namespace chip {
namespace app {

class CommandSender final : public Messaging::ExchangeDelegate
{
public:
    // CommandSender::ExtendableCallback::OnResponse is public SDK API, so we cannot break
    // source compatibility for it. To allow for additional values to be added at a future
    // time without constantly changing the function's declaration parameter list, we are
    // defining the struct ResponseData and adding that to the parameter list to allow for
    // future extendability.
    struct ResponseData
    {
        // The command path field in invoke command response.
        const ConcreteCommandPath & path;
        // The status of the command. It can be any success status, including possibly a cluster-specific one.
        // If `data` is not null, statusIB will always be a generic SUCCESS status with no-cluster specific
        // information.
        const StatusIB & statusIB;
        // The command data, will be nullptr if the server returns a StatusIB.
        TLV::TLVReader * data;
        // Reference for the command. This should be associated with the reference value sent out in the initial
        // invoke request.
        Optional<uint16_t> commandRef;
    };

    // CommandSender::ExtendableCallback::OnNoResponse is public SDK API, so we cannot break
    // source compatibility for it. To allow for additional values to be added at a future
    // time without constantly changing the function's declaration parameter list, we are
    // defining the struct NoResponseData and adding that to the parameter list to allow for
    // future extendability.
    struct NoResponseData
    {
        uint16_t commandRef;
    };

    // CommandSender::ExtendableCallback::OnError is public SDK API, so we cannot break source
    // compatibility for it. To allow for additional values to be added at a future time
    // without constantly changing the function's declaration parameter list, we are
    // defining the struct ErrorData and adding that to the parameter list
    // to allow for future extendability.
    struct ErrorData
    {
        /**
         * The following errors will be delivered through `error`
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR encapsulating a StatusIB: If we got a non-path-specific
         *   status response from the server.  In that case, constructing
         *   a StatusIB from the error can be used to extract the status.
         * - CHIP_ERROR*: All other cases.
         */
        CHIP_ERROR error;
    };

    /**
     * @brief Callback that is extendable for future features, starting with batch commands
     *
     * The two major differences between ExtendableCallback and Callback are:
     * 1. Path-specific errors go to OnResponse instead of OnError
     *       - Note: Non-path-specific errors still go to OnError.
     * 2. Instead of having new parameters at the end of the arguments list, with defaults,
     *    as functionality expands, a parameter whose type is defined in this header is used
     *    as the argument to the callbacks
     *
     * To support batch commands client must use ExtendableCallback.
     */
    class ExtendableCallback
    {
    public:
        virtual ~ExtendableCallback() = default;

        /**
         * OnResponse will be called for all path specific responses from the server that have been received
         * and processed. Specifically:
         *  - When a status code is received and it is IM::Success, aData will be nullptr.
         *  - When a status code is received and it is IM and/or cluster error, aData will be nullptr.
         *      - These kinds of errors are referred to as path-specific errors.
         *  - When a data response is received, aData will point to a valid TLVReader initialized to point at the struct container
         *    that contains the data payload (callee will still need to open and process the container).
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param[in] commandSender   The command sender object that initiated the command transaction.
         * @param[in] aResponseData   Information pertaining to the response.
         */
        virtual void OnResponse(CommandSender * commandSender, const ResponseData & aResponseData) {}

        /**
         * Called for each request that failed to receive a response after the server indicates completion of all requests.
         *
         * This callback may be omitted if clients have alternative ways to track non-responses.
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * @param commandSender   The CommandSender object that initiated the transaction.
         * @param aNoResponseData Details about the request without a response.
         */
        virtual void OnNoResponse(CommandSender * commandSender, const NoResponseData & aNoResponseData) {}

        /**
         * OnError will be called when a non-path-specific error occurs *after* a successful call to SendCommandRequest().
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy and free the object.
         *
         * NOTE: Path specific errors do NOT come to OnError, but instead go to OnResponse.
         *
         * @param[in] apCommandSender The command sender object that initiated the command transaction.
         * @param[in] aErrorData      A error data regarding error that occurred.
         */
        virtual void OnError(const CommandSender * apCommandSender, const ErrorData & aErrorData) {}

        /**
         * OnDone will be called when CommandSender has finished all work and is safe to destroy and free the
         * allocated CommandSender object.
         *
         * This function will:
         *      - Always be called exactly *once* for a given CommandSender instance.
         *      - Be called even in error circumstances.
         *      - Only be called after a successful call to SendCommandRequest returns, if SendCommandRequest is used.
         *      - Always be called before a successful return from SendGroupCommandRequest, if SendGroupCommandRequest is used.
         *
         * This function must be implemented to destroy the CommandSender object.
         *
         * @param[in] apCommandSender   The command sender object of the terminated invoke command transaction.
         */
        virtual void OnDone(CommandSender * apCommandSender) = 0;
    };

    // `Callback` exists for legacy purposes. If you are developing a new callback implementation,
    // please use `ExtendableCallback`.
    using Callback = CommandSenderLegacyCallback;

    // SetCommandSenderConfig is a public SDK API, so we cannot break source compatibility
    // for it. By having parameters to that API use this struct instead of individual
    // function arguments, we centralize required changes to one file when adding new
    // funtionality.
    struct ConfigParameters
    {
        ConfigParameters & SetRemoteMaxPathsPerInvoke(uint16_t aRemoteMaxPathsPerInvoke)
        {
            remoteMaxPathsPerInvoke = aRemoteMaxPathsPerInvoke;
            return *this;
        }

        // If remoteMaxPathsPerInvoke is 1, this will allow the CommandSender client to contain only one command and
        // doesn't enforce other batch commands requirements.
        uint16_t remoteMaxPathsPerInvoke = 1;
    };

    // AddRequestData is a public SDK API, so we must maintain source compatibility.
    // Using this struct for API parameters instead of individual parameters allows us
    // to make necessary changes for new functionality in a single location.
    struct AddRequestDataParameters
    {
        // gcc bug requires us to have the constructor below
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96645
        AddRequestDataParameters() {}

        AddRequestDataParameters(const Optional<uint16_t> & aTimedInvokeTimeoutMs) : timedInvokeTimeoutMs(aTimedInvokeTimeoutMs) {}

        AddRequestDataParameters & SetCommandRef(uint16_t aCommandRef)
        {
            commandRef.SetValue(aCommandRef);
            return *this;
        }

        // When a value is provided for timedInvokeTimeoutMs, this invoke becomes a timed
        // invoke. CommandSender will use the minimum of all provided timeouts for execution.
        const Optional<uint16_t> timedInvokeTimeoutMs;
        // The command reference is required when sending multiple commands. It allows the caller
        // to associate this request with its corresponding response.
        Optional<uint16_t> commandRef;
    };

    // PrepareCommand is a public SDK API, so we must maintain source compatibility.
    // Using this struct for API parameters instead of individual parameters allows us
    // to make necessary changes for new functionality in a single location.
    struct PrepareCommandParameters
    {
        // gcc bug requires us to have the constructor below
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96645
        PrepareCommandParameters() {}

        PrepareCommandParameters(const AddRequestDataParameters & aAddRequestDataParam) :
            commandRef(aAddRequestDataParam.commandRef)
        {}

        PrepareCommandParameters & SetStartDataStruct(bool aStartDataStruct)
        {
            startDataStruct = aStartDataStruct;
            return *this;
        }

        PrepareCommandParameters & SetCommandRef(uint16_t aCommandRef)
        {
            commandRef.SetValue(aCommandRef);
            return *this;
        }
        // The command reference is required when sending multiple commands. It allows the caller
        // to associate this request with its corresponding response. We validate the reference
        // early in PrepareCommand, even though it's not used until FinishCommand. This proactive
        // validation helps prevent unnecessary writing an InvokeRequest into the packet that later
        // needs to be undone.
        Optional<uint16_t> commandRef;
        // If the InvokeRequest needs to be in a state with a started data TLV struct container
        bool startDataStruct = false;
    };

    // FinishCommand is a public SDK API, so we must maintain source compatibility.
    // Using this struct for API parameters instead of individual parameters allows us
    // to make necessary changes for new functionality in a single location.
    struct FinishCommandParameters
    {
        // gcc bug requires us to have the constructor below
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96645
        FinishCommandParameters() {}

        FinishCommandParameters(const Optional<uint16_t> & aTimedInvokeTimeoutMs) : timedInvokeTimeoutMs(aTimedInvokeTimeoutMs) {}
        FinishCommandParameters(const AddRequestDataParameters & aAddRequestDataParam) :
            timedInvokeTimeoutMs(aAddRequestDataParam.timedInvokeTimeoutMs), commandRef(aAddRequestDataParam.commandRef)
        {}

        FinishCommandParameters & SetEndDataStruct(bool aEndDataStruct)
        {
            endDataStruct = aEndDataStruct;
            return *this;
        }

        FinishCommandParameters & SetCommandRef(uint16_t aCommandRef)
        {
            commandRef.SetValue(aCommandRef);
            return *this;
        }

        // When a value is provided for timedInvokeTimeoutMs, this invoke becomes a timed
        // invoke. CommandSender will use the minimum of all provided timeouts for execution.
        const Optional<uint16_t> timedInvokeTimeoutMs;
        // The command reference is required when sending multiple commands. It allows the caller
        // to associate this request with its corresponding response. This value must be
        // the same as the one provided in PrepareCommandParameters when calling PrepareCommand.
        Optional<uint16_t> commandRef;
        // If InvokeRequest is in a state where the data TLV struct container is currently open
        // and FinishCommand should close it.
        bool endDataStruct = false;
    };

    class TestOnlyMarker
    {
    };

    /*
     * Constructor.
     *
     * The callback passed in has to outlive this CommandSender object.
     * If used in a groups setting, callbacks do not need to be passed.
     * If callbacks are passed the only one that will be called in a group sesttings is the onDone
     */
    CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr, bool aIsTimedRequest = false,
                  bool aSuppressResponse = false, bool aAllowLargePayload = false);
    CommandSender(std::nullptr_t, Messaging::ExchangeManager * apExchangeMgr, bool aIsTimedRequest = false,
                  bool aSuppressResponse = false, bool aAllowLargePayload = false) :
        CommandSender(static_cast<Callback *>(nullptr), apExchangeMgr, aIsTimedRequest, aSuppressResponse, aAllowLargePayload)
    {}
    CommandSender(ExtendableCallback * apCallback, Messaging::ExchangeManager * apExchangeMgr, bool aIsTimedRequest = false,
                  bool aSuppressResponse = false, bool aAllowLargePayload = false);
    // TODO(#32138): After there is a macro that is always defined for all unit tests, the constructor with
    // TestOnlyMarker should only be compiled if that macro is defined.
    CommandSender(TestOnlyMarker aTestMarker, ExtendableCallback * apCallback, Messaging::ExchangeManager * apExchangeMgr,
                  PendingResponseTracker * apPendingResponseTracker, bool aIsTimedRequest = false, bool aSuppressResponse = false,
                  bool aAllowLargePayload = false) :
        CommandSender(apCallback, apExchangeMgr, aIsTimedRequest, aSuppressResponse, aAllowLargePayload)
    {
        mpPendingResponseTracker = apPendingResponseTracker;
    }
    ~CommandSender();

    /**
     * Enables additional features of CommandSender, for example sending batch commands.
     *
     * In the case of enabling batch commands, once set it ensures that commands contain all
     * required data elements while building the InvokeRequestMessage. This must be called
     * before PrepareCommand.
     *
     * @param [in] aConfigParams contains information to configure CommandSender behavior,
     *                      such as such as allowing a max number of paths per invoke greater than one,
     *                      based on how many paths the remote peer claims to support.
     *
     * @return CHIP_ERROR_INCORRECT_STATE
     *             If device has previously called `PrepareCommand`.
     * @return CHIP_ERROR_INVALID_ARGUMENT
     *             Invalid argument value.
     * @return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
     *             Device has not enabled batch command support. To enable:
     *               1. Enable the CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS
     *                  configuration option.
     *               2. Ensure you provide ExtendableCallback.
     */
    CHIP_ERROR SetCommandSenderConfig(ConfigParameters & aConfigParams);

    CHIP_ERROR PrepareCommand(const CommandPathParams & aCommandPathParams, PrepareCommandParameters & aPrepareCommandParams);

    [[deprecated("PrepareCommand should migrate to calling PrepareCommand with PrepareCommandParameters")]] CHIP_ERROR
    PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct = true)
    {
        PrepareCommandParameters prepareCommandParams;
        prepareCommandParams.SetStartDataStruct(aStartDataStruct);
        return PrepareCommand(aCommandPathParams, prepareCommandParams);
    }

    CHIP_ERROR FinishCommand(FinishCommandParameters & aFinishCommandParams);

    [[deprecated("FinishCommand should migrate to calling FinishCommand with FinishCommandParameters")]] CHIP_ERROR
    FinishCommand(bool aEndDataStruct = true)
    {
        FinishCommandParameters finishCommandParams;
        finishCommandParams.SetEndDataStruct(aEndDataStruct);
        return FinishCommand(finishCommandParams);
    }
    [[deprecated("FinishCommand should migrate to calling FinishCommand with FinishCommandParameters")]] CHIP_ERROR
    FinishCommand(const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        FinishCommandParameters finishCommandParams(aTimedInvokeTimeoutMs);
        return FinishCommand(finishCommandParams);
    }

    TLV::TLVWriter * GetCommandDataIBTLVWriter();

    /**
     * API for adding request data using DataModel::EncodableToTLV.
     *
     * @param [in] aCommandPath The path of the command being requested.
     * @param [in] aEncodable The request data to encode into the
     *             `CommandFields` member of `CommandDataIB`.
     * @param [in] aAddRequestDataParams parameters associated with building the
     *             InvokeRequestMessage that are associated with this request.
     *
     * This API will not fail if this is an untimed invoke but the command provided requires a timed
     * invoke interaction. If the caller wants that to fail before sending the command, they should call
     * the templated version of AddRequestData.
     */
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const DataModel::EncodableToTLV & aEncodable,
                              AddRequestDataParameters & aAddRequestDataParams);

    /**
     * API for adding a data request.  The template parameter T is generally
     * expected to be a ClusterName::Commands::CommandName::Type struct, but any
     * object that can be encoded using the DataModel::Encode machinery and
     * exposes the right command id will work.
     *
     * @param [in] aCommandPath  The path of the command being requested.
     * @param [in] aData         The data for the request.
     */
    template <typename CommandDataT, typename std::enable_if_t<!CommandDataT::MustUseTimedInvoke(), int> = 0>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData)
    {
        AddRequestDataParameters addRequestDataParams;
        return AddRequestData(aCommandPath, aData, addRequestDataParams);
    }

    template <typename CommandDataT,
              typename std::enable_if_t<!std::is_base_of_v<DataModel::EncodableToTLV, CommandDataT>, int> = 0>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                              AddRequestDataParameters & aAddRequestDataParams)
    {
        VerifyOrReturnError(!CommandDataT::MustUseTimedInvoke() || aAddRequestDataParams.timedInvokeTimeoutMs.HasValue(),
                            CHIP_ERROR_INVALID_ARGUMENT);

        DataModel::EncodableType<CommandDataT> encodable(aData);
        return AddRequestData(aCommandPath, encodable, aAddRequestDataParams);
    }

    template <typename CommandDataT>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                              const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        AddRequestDataParameters addRequestDataParams(aTimedInvokeTimeoutMs);
        return AddRequestData(aCommandPath, aData, addRequestDataParams);
    }

    /**
     * @brief Returns the number of InvokeResponseMessages received.
     *
     * Responses to multiple requests might be split across several InvokeResponseMessages.
     * This function helps track the total count. Primarily for test validation purposes.
     */
    size_t GetInvokeResponseMessageCount();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    /**
     * Version of AddRequestData that allows sending a message that is
     * guaranteed to fail due to requiring a timed invoke but not providing a
     * timeout parameter.  For use in tests only.
     */
    template <typename CommandDataT>
    CHIP_ERROR TestOnlyAddRequestDataNoTimedCheck(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                                                  AddRequestDataParameters & aAddRequestDataParams)
    {
        DataModel::EncodableType<CommandDataT> encodable(aData);
        return AddRequestData(aCommandPath, encodable, aAddRequestDataParams);
    }

    CHIP_ERROR TestOnlyFinishCommand(FinishCommandParameters & aFinishCommandParams)
    {
        if (mBatchCommandsEnabled)
        {
            VerifyOrReturnError(aFinishCommandParams.commandRef.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        }
        return FinishCommandInternal(aFinishCommandParams);
    }

    /**
     * Version of SendCommandRequest that sets the TimedRequest flag but does not send the TimedInvoke
     * action. For use in tests only.
     */
    CHIP_ERROR TestOnlyCommandSenderTimedRequestFlagWithNoTimedInvoke(const SessionHandle & session,
                                                                      Optional<System::Clock::Timeout> timeout = NullOptional);

#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

private:
    CHIP_ERROR FinishCommandInternal(FinishCommandParameters & aFinishCommandParams);

public:
    // Sends a queued up command request to the target encapsulated by the secureSession handle.
    //
    // Upon successful return from this call, all subsequent errors that occur during this interaction
    // will be conveyed through the OnError callback above. In addition, upon completion of work regardless of
    // whether it was successful or not, the OnDone callback will be invoked to indicate completion of work on this
    // object and to indicate to the application that it can destroy and free this object.
    //
    // Applications can, however, destroy this object at any time after this call, except while handling
    // an OnResponse or OnError callback, and it will safely clean-up.
    //
    // If this call returns failure, the callback's OnDone will never be called; the client is responsible
    // for destroying this object on failure.
    //
    // Client can specify the maximum time to wait for response (in milliseconds) via timeout parameter.
    // Default timeout value will be used otherwise.
    //
    CHIP_ERROR SendCommandRequest(const SessionHandle & session, Optional<System::Clock::Timeout> timeout = NullOptional);

    // Sends a queued up group command request to the target encapsulated by the secureSession handle.
    //
    // If this function is successful, it will invoke the OnDone callback before returning to indicate
    // to the application that it can destroy and free this object.
    //
    CHIP_ERROR SendGroupCommandRequest(const SessionHandle & session);

private:
    friend class TestCommandInteraction;

    enum class State : uint8_t
    {
        Idle,                ///< Default state that the object starts out in, where no work has commenced
        AddingCommand,       ///< In the process of adding a command.
        AddedCommand,        ///< A command has been completely encoded and is awaiting transmission.
        AwaitingTimedStatus, ///< Sent a Timed Request and waiting for response.
        AwaitingResponse,    ///< The command has been sent successfully, and we are awaiting invoke response.
        ResponseReceived,    ///< Received a response to our invoke and request and processing the response.
        AwaitingDestruction, ///< The object has completed its work and is awaiting destruction by the application.
    };

    /**
     * Class to help backup CommandSender's buffer containing InvokeRequestMessage when adding InvokeRequest
     * in case there is a failure to add InvokeRequest. Intended usage is as follows:
     *  - Allocate RollbackInvokeRequest on the stack.
     *  - Attempt adding InvokeRequest into InvokeRequestMessage buffer.
     *  - If modification is added successfully, call DisableAutomaticRollback() to prevent destructor from
     *    rolling back InvokeReqestMessage.
     *  - If there is an issue adding InvokeRequest, destructor will take care of rolling back
     *    InvokeRequestMessage to previously saved state.
     */
    class RollbackInvokeRequest
    {
    public:
        explicit RollbackInvokeRequest(CommandSender & aCommandSender);
        ~RollbackInvokeRequest();

        /**
         * Disables rolling back to previously saved state for InvokeRequestMessage.
         */
        void DisableAutomaticRollback();

    private:
        CommandSender & mCommandSender;
        TLV::TLVWriter mBackupWriter;
        State mBackupState;
        bool mRollbackInDestructor = false;
    };

    union CallbackHandle
    {
        CallbackHandle(Callback * apCallback) : legacyCallback(apCallback) {}
        CallbackHandle(ExtendableCallback * apExtendableCallback) : extendableCallback(apExtendableCallback) {}
        Callback * legacyCallback;
        ExtendableCallback * extendableCallback;
    };

    void MoveToState(const State aTargetState);
    const char * GetStateStr() const;

    /*
     * Allocates a packet buffer used for encoding an invoke request payload.
     *
     * This can be called multiple times safely, as it will only allocate the buffer once for the lifetime
     * of this object.
     */
    CHIP_ERROR AllocateBuffer();

    // ExchangeDelegate interface implementation.  Private so people won't
    // accidentally call it on us when we're not being treated as an actual
    // ExchangeDelegate.
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    void FlushNoCommandResponse();
    //
    // Called internally to signal the completion of all work on this object, gracefully close the
    // exchange (by calling into the base class) and finally, signal to the application that it's
    // safe to release this object.
    //
    void Close();

    /*
     * This forcibly closes the exchange context if a valid one is pointed to. Such a situation does
     * not arise during normal message processing flows that all normally call Close() above. This can only
     * arise due to application-initiated destruction of the object when this object is handling receiving/sending
     * message payloads.
     */
    void Abort();

    CHIP_ERROR ProcessInvokeResponse(System::PacketBufferHandle && payload, bool & moreChunkedMessages);
    CHIP_ERROR ProcessInvokeResponseIB(InvokeResponseIB::Parser & aInvokeResponse);

    void SetTimedInvokeTimeoutMs(const Optional<uint16_t> & aTimedInvokeTimeoutMs);

    // Send our queued-up Invoke Request message.  Assumes the exchange is ready
    // and mPendingInvokeData is populated.
    CHIP_ERROR SendInvokeRequest();

    CHIP_ERROR Finalize(System::PacketBufferHandle & commandPacket);

    CHIP_ERROR SendCommandRequestInternal(const SessionHandle & session, Optional<System::Clock::Timeout> timeout);

    void OnResponseCallback(const ResponseData & aResponseData)
    {
        // mpExtendableCallback and mpCallback are mutually exclusive.
        if (mUseExtendableCallback && mCallbackHandle.extendableCallback)
        {
            mCallbackHandle.extendableCallback->OnResponse(this, aResponseData);
        }
        else if (mCallbackHandle.legacyCallback)
        {
            mCallbackHandle.legacyCallback->OnResponse(this, aResponseData.path, aResponseData.statusIB, aResponseData.data);
        }
    }

    void OnErrorCallback(CHIP_ERROR aError)
    {
        // mpExtendableCallback and mpCallback are mutually exclusive.
        if (mUseExtendableCallback && mCallbackHandle.extendableCallback)
        {
            ErrorData errorData = { aError };
            mCallbackHandle.extendableCallback->OnError(this, errorData);
        }
        else if (mCallbackHandle.legacyCallback)
        {
            mCallbackHandle.legacyCallback->OnError(this, aError);
        }
    }

    void OnDoneCallback()
    {
        // mpExtendableCallback and mpCallback are mutually exclusive.
        if (mUseExtendableCallback && mCallbackHandle.extendableCallback)
        {
            mCallbackHandle.extendableCallback->OnDone(this);
        }
        else if (mCallbackHandle.legacyCallback)
        {
            mCallbackHandle.legacyCallback->OnDone(this);
        }
    }

    Messaging::ExchangeHolder mExchangeCtx;
    CallbackHandle mCallbackHandle;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    InvokeRequestMessage::Builder mInvokeRequestBuilder;
    // TODO Maybe we should change PacketBufferTLVWriter so we can finalize it
    // but have it hold on to the buffer, and get the buffer from it later.
    // Then we could avoid this extra pointer-sized member.
    System::PacketBufferHandle mPendingInvokeData;
    // If mTimedInvokeTimeoutMs has a value, we are expected to do a timed
    // invoke.
    Optional<uint16_t> mTimedInvokeTimeoutMs;
    TLV::TLVType mDataElementContainerType = TLV::kTLVType_NotSpecified;

    chip::System::PacketBufferTLVWriter mCommandMessageWriter;

#if CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS
    PendingResponseTrackerImpl mNonTestPendingResponseTracker;
#endif // CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS
    PendingResponseTracker * mpPendingResponseTracker = nullptr;

    uint16_t mInvokeResponseMessageCount = 0;
    uint16_t mFinishedCommandCount       = 0;
    uint16_t mRemoteMaxPathsPerInvoke    = 1;

    State mState                = State::Idle;
    bool mSuppressResponse      = false;
    bool mTimedRequest          = false;
    bool mBufferAllocated       = false;
    bool mBatchCommandsEnabled  = false;
    bool mUseExtendableCallback = false;
    bool mAllowLargePayload     = false;
};

} // namespace app
} // namespace chip
