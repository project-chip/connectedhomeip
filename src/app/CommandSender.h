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

#include <app/CommandPathParams.h>
#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>
#include <app/MessageDef/StatusIB.h>
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
    // CommandSender::Callback::OnResponse is public SDK API, so we cannot break source
    // compatibility for it. To allow for additional values to be added at a future time
    // without constantly changing the function's declaration parameter list, we are
    // defining the struct AdditionalResponseData and adding that to the parameter list
    // to allow for future extendability.
    struct AdditionalResponseData
    {
        Optional<uint16_t> mCommandRef;
    };

    class Callback
    {
    public:
        virtual ~Callback() = default;

        /**
         * OnResponseWithAdditionalData will be called when a successful response from the server has been received and processed.
         * Specifically:
         *  - When a status code is received and it is IM::Success, aData will be nullptr.
         *  - When a data response is received, aData will point to a valid TLVReader initialized to point at the struct container
         *    that contains the data payload (callee will still need to open and process the container).
         *
         * This OnResponseWithAdditionalData is similar to OnResponse mentioned below, except it contains an additional parameter
         * `AdditionalResponseData`. This was added in Matter 1.3 to not break backward compatibility, but is extendable in the
         * future to provide additional response data by only making changes to `AdditionalResponseData`, and not all the potential
         * callees.
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * It is advised that subclass should only override this or `OnResponse`. But, it shouldn't actually matter if both are
         * overridden, just that `OnResponse` will never be called by CommandSender directly.
         *
         * @param[in] apCommandSender The command sender object that initiated the command transaction.
         * @param[in] aPath           The command path field in invoke command response.
         * @param[in] aStatusIB       It will always have a success status. If apData is null, it can be any success status,
         *                            including possibly a cluster-specific one. If apData is not null it aStatusIB will always
         *                            be a generic SUCCESS status with no-cluster specific information.
         * @param[in] apData          The command data, will be nullptr if the server returns a StatusIB.
         * @param[in] aAdditionalResponseData
         *                            Additional response data that comes within the InvokeResponseMessage.
         */
        virtual void OnResponseWithAdditionalData(CommandSender * apCommandSender, const ConcreteCommandPath & aPath,
                                                  const StatusIB & aStatusIB, TLV::TLVReader * apData,
                                                  const AdditionalResponseData & aAdditionalResponseData)
        {
            OnResponse(apCommandSender, aPath, aStatusIB, apData);
        }

        /**
         * OnResponse will be called when a successful response from server has been received and processed. Specifically:
         *  - When a status code is received and it is IM::Success, aData will be nullptr.
         *  - When a data response is received, aData will point to a valid TLVReader initialized to point at the struct container
         *    that contains the data payload (callee will still need to open and process the container).
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
         *
         * It is advised that subclasses should only override this or `OnResponseWithAdditionalData`. But, it shouldn't actually
         * matter if both are overridden, just that `OnResponse` will never be called by CommandSender directly.
         *
         * @param[in] apCommandSender The command sender object that initiated the command transaction.
         * @param[in] aPath           The command path field in invoke command response.
         * @param[in] aStatusIB       It will always have a success status. If apData is null, it can be any success status,
         *                            including possibly a cluster-specific one. If apData is not null it aStatusIB will always
         *                            be a generic SUCCESS status with no-cluster specific information.
         * @param[in] apData          The command data, will be nullptr if the server returns a StatusIB.
         */
        virtual void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, const StatusIB & aStatusIB,
                                TLV::TLVReader * apData)
        {}

        /**
         * OnError will be called when an error occur *after* a successful call to SendCommandRequest(). The following
         * errors will be delivered through this call in the aError field:
         *
         * - CHIP_ERROR_TIMEOUT: A response was not received within the expected response timeout.
         * - CHIP_ERROR_*TLV*: A malformed, non-compliant response was received from the server.
         * - CHIP_ERROR encapsulating a StatusIB: If we got a non-path-specific
         *   status response from the server.  In that case,
         *   StatusIB::InitFromChipError can be used to extract the status.
         * - CHIP_ERROR*: All other cases.
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy and free the object.
         *
         * @param[in] apCommandSender The command sender object that initiated the command transaction.
         * @param[in] aError          A system error code that conveys the overall error code.
         */
        virtual void OnError(const CommandSender * apCommandSender, CHIP_ERROR aError) {}

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

    // SetCommandSenderConfig is a public SDK API, so we cannot break source compatibility
    // for it. By having parameters to that API use this struct instead of individual
    // function arguments, we centralize required changes to one file when adding new
    // funtionality.
    struct ConfigParameters
    {
        ConfigParameters & SetRemoteMaxPathsPerInvoke(uint16_t aRemoteMaxPathsPerInvoke)
        {
            mRemoteMaxPathsPerInvoke = aRemoteMaxPathsPerInvoke;
            return *this;
        }

        // If mRemoteMaxPathsPerInvoke is 1, this will allow the CommandSender client to contain only one command and
        // doesn't enforce other batch commands requirements.
        uint16_t mRemoteMaxPathsPerInvoke = 1;
    };

    // PrepareCommand and FinishCommand are public SDK APIs, so we cannot break source
    // compatibility for them. By having parameters to those APIs use this struct instead
    // of individual function arguments, we centralize required changes to one file
    // when adding new functionality.
    struct AdditionalCommandParameters
    {
        // gcc bug requires us to have the constructor below
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96645
        AdditionalCommandParameters() {}

        AdditionalCommandParameters & SetStartOrEndDataStruct(bool aStartOrEndDataStruct)
        {
            mStartOrEndDataStruct = aStartOrEndDataStruct;
            return *this;
        }

        // From the perspective of `PrepareCommand`, this is an out parameter. This value will be
        // set by `PrepareCommand` and is expected to be unchanged by caller until it is provided
        // to `FinishCommand`.
        Optional<uint16_t> mCommandRef;
        // For both `PrepareCommand` and `FinishCommand` this is an in parameter. It must have
        // the same value when calling `PrepareCommand` and `FinishCommand` for a given command.
        bool mStartOrEndDataStruct = false;
    };

    /*
     * Constructor.
     *
     * The callback passed in has to outlive this CommandSender object.
     * If used in a groups setting, callbacks do not need to be passed.
     * If callbacks are passed the only one that will be called in a group sesttings is the onDone
     */
    CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr, bool aIsTimedRequest = false,
                  bool aSuppressResponse = false);
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
     *                      If device has previously called `PrepareCommand`.
     * @return CHIP_ERROR_INVALID_ARGUMENT
     *                      Invalid argument value.
     * @return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
     *                      Device has not enabled CHIP_CONFIG_SENDING_BATCH_COMMANDS_ENABLED.
     */
    CHIP_ERROR SetCommandSenderConfig(ConfigParameters & aConfigParams);

    CHIP_ERROR PrepareCommand(const CommandPathParams & aCommandPathParams, AdditionalCommandParameters & aOptionalArgs);

    [[deprecated("PrepareCommand should migrate to calling PrepareCommand with AdditionalCommandParameters")]] CHIP_ERROR
    PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct = true)
    {
        AdditionalCommandParameters optionalArgs;
        optionalArgs.SetStartOrEndDataStruct(aStartDataStruct);
        return PrepareCommand(aCommandPathParams, optionalArgs);
    }

    CHIP_ERROR FinishCommand(const AdditionalCommandParameters & aOptionalArgs);

    [[deprecated("FinishCommand should migrate to calling FinishCommand with AdditionalCommandParameters")]] CHIP_ERROR
    FinishCommand(bool aEndDataStruct = true)
    {
        AdditionalCommandParameters optionalArgs;
        optionalArgs.SetStartOrEndDataStruct(aEndDataStruct);
        return FinishCommand(optionalArgs);
    }

    TLV::TLVWriter * GetCommandDataIBTLVWriter();

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
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                              AdditionalCommandParameters & aOptionalArgs)
    {
        return AddRequestData(aCommandPath, aData, NullOptional, aOptionalArgs);
    }
    template <typename CommandDataT, typename std::enable_if_t<!CommandDataT::MustUseTimedInvoke(), int> = 0>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData)
    {
        AdditionalCommandParameters optionalArgs;
        return AddRequestData(aCommandPath, aData, optionalArgs);
    }

    /**
     * API for adding a data request that allows caller to provide a timed
     * invoke timeout.  If provided, this invoke will be a timed invoke, using
     * the minimum of the provided timeouts.
     */
    template <typename CommandDataT>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                              const Optional<uint16_t> & aTimedInvokeTimeoutMs, AdditionalCommandParameters & aOptionalArgs)
    {
        VerifyOrReturnError(!CommandDataT::MustUseTimedInvoke() || aTimedInvokeTimeoutMs.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        // AddRequestDataInternal encodes the data and requires mStartOrEndDataStruct to be false.
        VerifyOrReturnError(aOptionalArgs.mStartOrEndDataStruct == false, CHIP_ERROR_INVALID_ARGUMENT);

        return AddRequestDataInternal(aCommandPath, aData, aTimedInvokeTimeoutMs, aOptionalArgs);
    }
    template <typename CommandDataT>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                              const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        AdditionalCommandParameters optionalArgs;
        return AddRequestData(aCommandPath, aData, aTimedInvokeTimeoutMs, optionalArgs);
    }

    CHIP_ERROR FinishCommand(const Optional<uint16_t> & aTimedInvokeTimeoutMs, const AdditionalCommandParameters & aOptionalArgs);

    CHIP_ERROR FinishCommand(const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        AdditionalCommandParameters optionalArgs;
        return FinishCommand(aTimedInvokeTimeoutMs, optionalArgs);
    }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    /**
     * Version of AddRequestData that allows sending a message that is
     * guaranteed to fail due to requiring a timed invoke but not providing a
     * timeout parameter.  For use in tests only.
     */
    template <typename CommandDataT>
    CHIP_ERROR AddRequestDataNoTimedCheck(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                                          const Optional<uint16_t> & aTimedInvokeTimeoutMs,
                                          AdditionalCommandParameters & aOptionalArgs)
    {
        return AddRequestDataInternal(aCommandPath, aData, aTimedInvokeTimeoutMs, aOptionalArgs);
    }

    template <typename CommandDataT>
    CHIP_ERROR AddRequestDataNoTimedCheck(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                                          const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        AdditionalCommandParameters optionalArgs;
        return AddRequestDataNoTimedCheck(aCommandPath, aData, aTimedInvokeTimeoutMs, optionalArgs);
    }

    /**
     * Version of SendCommandRequest that sets the TimedRequest flag but does not send the TimedInvoke
     * action. For use in tests only.
     */
    CHIP_ERROR TestOnlyCommandSenderTimedRequestFlagWithNoTimedInvoke(const SessionHandle & session,
                                                                      Optional<System::Clock::Timeout> timeout = NullOptional);

#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

private:
    template <typename CommandDataT>
    CHIP_ERROR AddRequestDataInternal(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                                      const Optional<uint16_t> & aTimedInvokeTimeoutMs, AdditionalCommandParameters & aOptionalArgs)
    {
        ReturnErrorOnFailure(PrepareCommand(aCommandPath, aOptionalArgs));
        TLV::TLVWriter * writer = GetCommandDataIBTLVWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, TLV::ContextTag(CommandDataIB::Tag::kFields), aData));
        return FinishCommand(aTimedInvokeTimeoutMs, aOptionalArgs);
    }

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

    enum class State
    {
        Idle,                ///< Default state that the object starts out in, where no work has commenced
        AddingCommand,       ///< In the process of adding a command.
        AddedCommand,        ///< A command has been completely encoded and is awaiting transmission.
        AwaitingTimedStatus, ///< Sent a Timed Request and waiting for response.
        CommandSent,         ///< The command has been sent successfully.
        ResponseReceived,    ///< Received a response to our invoke and request and processing the response.
        AwaitingDestruction, ///< The object has completed its work and is awaiting destruction by the application.
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

    CHIP_ERROR ProcessInvokeResponse(System::PacketBufferHandle && payload);
    CHIP_ERROR ProcessInvokeResponseIB(InvokeResponseIB::Parser & aInvokeResponse);

    // Send our queued-up Invoke Request message.  Assumes the exchange is ready
    // and mPendingInvokeData is populated.
    CHIP_ERROR SendInvokeRequest();

    CHIP_ERROR Finalize(System::PacketBufferHandle & commandPacket);

    CHIP_ERROR SendCommandRequestInternal(const SessionHandle & session, Optional<System::Clock::Timeout> timeout);

    Messaging::ExchangeHolder mExchangeCtx;
    Callback * mpCallback                      = nullptr;
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

    State mState = State::Idle;
    chip::System::PacketBufferTLVWriter mCommandMessageWriter;
    uint16_t mFinishedCommandCount    = 0;
    uint16_t mRemoteMaxPathsPerInvoke = 1;

    bool mSuppressResponse     = false;
    bool mTimedRequest         = false;
    bool mBufferAllocated      = false;
    bool mBatchCommandsEnabled = false;
};

} // namespace app
} // namespace chip
