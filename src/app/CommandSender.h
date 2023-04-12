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
    class Callback
    {
    public:
        virtual ~Callback() = default;

        /**
         * OnResponse will be called when a successful response from server has been received and processed. Specifically:
         *  - When a status code is received and it is IM::Success, aData will be nullptr.
         *  - When a data response is received, aData will point to a valid TLVReader initialized to point at the struct container
         *    that contains the data payload (callee will still need to open and process the container).
         *
         * The CommandSender object MUST continue to exist after this call is completed. The application shall wait until it
         * receives an OnDone call to destroy the object.
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

    /*
     * Constructor.
     *
     * The callback passed in has to outlive this CommandSender object.
     * If used in a groups setting, callbacks do not need to be passed.
     * If callbacks are passed the only one that will be called in a group sesttings is the onDone
     */
    CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr, bool aIsTimedRequest = false);
    CHIP_ERROR PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct = true);
    CHIP_ERROR FinishCommand(bool aEndDataStruct = true);
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
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData)
    {
        return AddRequestData(aCommandPath, aData, NullOptional);
    }

    /**
     * API for adding a data request that allows caller to provide a timed
     * invoke timeout.  If provided, this invoke will be a timed invoke, using
     * the minimum of the provided timeouts.
     */
    template <typename CommandDataT>
    CHIP_ERROR AddRequestData(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                              const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        VerifyOrReturnError(!CommandDataT::MustUseTimedInvoke() || aTimedInvokeTimeoutMs.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

        return AddRequestDataInternal(aCommandPath, aData, aTimedInvokeTimeoutMs);
    }

    CHIP_ERROR FinishCommand(const Optional<uint16_t> & aTimedInvokeTimeoutMs);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    /**
     * Version of AddRequestData that allows sending a message that is
     * guaranteed to fail due to requiring a timed invoke but not providing a
     * timeout parameter.  For use in tests only.
     */
    template <typename CommandDataT>
    CHIP_ERROR AddRequestDataNoTimedCheck(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                                          const Optional<uint16_t> & aTimedInvokeTimeoutMs, bool aSuppressResponse = false)
    {
        mSuppressResponse = aSuppressResponse;
        return AddRequestDataInternal(aCommandPath, aData, aTimedInvokeTimeoutMs);
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

private:
    template <typename CommandDataT>
    CHIP_ERROR AddRequestDataInternal(const CommandPathParams & aCommandPath, const CommandDataT & aData,
                                      const Optional<uint16_t> & aTimedInvokeTimeoutMs)
    {
        ReturnErrorOnFailure(PrepareCommand(aCommandPath, /* aStartDataStruct = */ false));
        TLV::TLVWriter * writer = GetCommandDataIBTLVWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, TLV::ContextTag(CommandDataIB::Tag::kFields), aData));
        return FinishCommand(aTimedInvokeTimeoutMs);
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
    bool mSuppressResponse                 = false;
    bool mTimedRequest                     = false;

    State mState = State::Idle;
    chip::System::PacketBufferTLVWriter mCommandMessageWriter;
    bool mBufferAllocated = false;
};

} // namespace app
} // namespace chip
