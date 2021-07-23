/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines a set of classes that applications can use to initiate and respond to
 *      IM commands.
 */

#pragma once

#include "IEncodableElement.h"
#include "messaging/ExchangeDelegate.h"
#include "protocols/secure_channel/Constants.h"
#include <app/MessageDef/CommandDataElement.h>
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>
#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <functional>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/BitFlags.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace app {

/*
 * @brief
 *
 * This class encapsulates the data that goes into a status response message.
 */
struct StatusResponse
{
public:
    bool IsError() { return generalCode != Protocols::SecureChannel::GeneralStatusCode::kSuccess; }
    Protocols::SecureChannel::GeneralStatusCode generalCode = Protocols::SecureChannel::GeneralStatusCode::kSuccess;
    Protocols::Id protocolId                                = Protocols::Id(Common, 0);
    uint16_t protocolCode                                   = 0;
};

/**
 * @brief
 *
 * Encapsulates parameters that go into the header of an IM Invoke Request or Response message.
 */
struct CommandParams
{
    enum class TargetType : uint8_t
    {
        kTargetEndpoint = 0x01, /**< Set when the EndpointId field is valid */
        kTargetGroup    = 0x02, /**< Set when the GroupId field is valid */
    };

    template <class CodeGenType>
    CommandParams(CodeGenType & t, chip::EndpointId endpointId, bool expectsResponses = true)
    {
        ClusterId        = (uint16_t) CodeGenType::GetClusterId();
        CommandId        = (uint8_t) CodeGenType::GetCommandId();
        EndpointId       = endpointId;
        ExpectsResponses = expectsResponses;
    }

    CommandParams() {}

    chip::EndpointId EndpointId = 0;
    chip::GroupId GroupId       = 0;
    chip::ClusterId ClusterId   = 0;
    chip::CommandId CommandId   = 0;
    BitFlags<TargetType> Flags  = TargetType::kTargetEndpoint;
    bool ExpectsResponses       = false;
};

/*
 * @brief
 *
 * This class represents an invoke interaction from the initiating side (i.e on the client). An instance of this class maps to a
 * specific invoke interaction over the wire. Consequently, the lifetime of this object is limited to a SINGLE interaction backed by
 * a SINGLE exchange context.
 *
 * This object permits a single request + response. It does not permit multiple responses to be sequenced in order.
 *
 * It provides a number of overloaded methods that permit adding multiple command payloads into the message.
 *
 * It also provides a means to register callbacks on receipt of responses or errors to the originating invoke request.
 */
class InvokeInitiator : chip::Messaging::ExchangeDelegate
{
public:
    /*
     * @brief
     *
     * An interface that applications implement to handle events that occur during the lifetime of an invoke interaction, including
     * responses and errors.
     */
    class ICommandHandler
    {
    public:
        /*
         * @brief
         *
         * Called to handle a data element present within an Invoke Response message.
         *
         * @param invokeInteraction         A reference to the invoke interaction instance that corresponds to the invoke response
         * being processed.
         * @param commandParams             A reference to an object containing the parameters of the command response.
         * @param payload                   If a data payload is present, a TLV reader positioned at that payload is provided. The
         * reader shall not be assumed to exist past this call, so the application should save off the data if they desire to.
         */
        virtual void OnResponse(InvokeInitiator & invokeInteraction, CommandParams & commandParams, TLV::TLVReader * payload) = 0;

        /*
         * @brief
         *
         * Called to handle any asynchronously generated errors (exchange timeout, non-success status reports, etc) on the invoke
         * interaction.
         *
         * @param invokeInteraction         A reference to the invoke interaction instance that corresponds to the invoke response
         * being processed.
         * @param commandParams             If the error is specific to a particular command, a pointer to a command parameter
         * object shall be provided. Otherwise, this SHALL be null.
         * @param error                     A CHIP_ERROR containing the high-level error that occurred.
         * @param statusResponse            If a status response was received, CHIP_ERROR shall be set to
         * CHIP_ERROR_STATUS_RESPONSE_RECEIVED and this argument SHALL be valid and non-null.
         */
        virtual void OnError(InvokeInitiator & invokeInteration, CommandParams * aPath, CHIP_ERROR error,
                             StatusResponse * statusResponse) = 0;

        /*
         * @brief
         *
         * When the interaction has completed, this method is invoked to give the application the chance to reclaim this object if
         * needed. This is invoked regardless of whether it was a successful completion or an erroneous one. In the latter case, the
         * `OnError` method above will be invoked first before the below is invoked.
         *
         * This method will be invoked anytime after the Init() method has been called on this object.
         *
         *
         * @param invokeInteraction         A reference to the invoke interaction instance that correponds to the invoke response
         * being processed.
         * @param commandParams             If the error is specific to a particular command, a pointer to a command parameter
         * object shall be provided. Otherwise, this SHALL be null.
         * @param error                     A CHIP_ERROR containing the high-level error that occurred.
         * @param statusResponse            If a status response was received, CHIP_ERROR shall be set to
         * CHIP_ERROR_STATUS_RESPONSE_RECEIVED and this argument SHALL be valid and non-null.
         */
        virtual void OnEnd(InvokeInitiator & invokeInteration) = 0;
        virtual ~ICommandHandler() {}
    };

    /*
     * @brief
     *
     * Initializes the object with a destination NodeId + AdminId pair OR a secure session handle.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, ICommandHandler * aHandler, NodeId aNodeId,
                    Transport::AdminId aAdminId, SecureSessionHandle * secureSession);

    /*
     * @brief
     *
     * Encodes an invoke request into the larger invoke message. This method takes an encodable object and command parameters and
     * writes that into an opened packet buffer.
     *
     */
    CHIP_ERROR AddRequest(CommandParams aParams, IEncodableElement * serializable);

    /*
     * @brief
     *
     * Encodes an invoke request into the larger invoke message. This method does so by accepting a generic lambda that is then
     * invoked immediately with a TLVWriter + TLV tag passed in. The writer is positioned within the command's payload.
     *
     */
    template <class F>
    CHIP_ERROR AddRequest(CommandParams aParams, F f)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        //
        // Update our accumulated flag that tracks if any command going into this invoke
        // expects responses
        //
        mExpectsResponses |= aParams.ExpectsResponses;

        err = StartCommandHeader(aParams);
        SuccessOrExit(err);

        //
        // Invoke the passed in closure that will actually write out the command payload if any
        //
        err = f(*mInvokeCommandBuilder.GetWriter(), TLV::ContextTag(CommandDataElement::kCsTag_Data));
        SuccessOrExit(err);

        mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().EndOfCommandDataElement();
        SuccessOrExit((err = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().GetError()));

    exit:
        return err;
    }

    /*
     * @brief
     *
     * In addition to encoding in the encodable object, this also immediately sends the message.
     *
     * This is equivalent to calling AddRequest and Send in succession.
     *
     */
    CHIP_ERROR AddRequestAndSend(CommandParams aParams, IEncodableElement * serializable);

    /*
     * @brief
     *
     * In addition to encoding in the invoke request by calling the provided lambda, this also immediately sends the message.
     *
     * This is equivalent to calling AddRequest and Send in succession.
     *
     */
    template <class F>
    CHIP_ERROR AddRequestAndSend(CommandParams aParams, F f)
    {
        ReturnErrorOnFailure(AddRequest(aParams, f));
        return Send();
    }

    /*
     * @brief
     *
     * If any invokes have been written into the message, this closes up the packet buffer and sends the message.
     * There-after, AddRequest and Send cannot be called again on this object.
     *
     */
    CHIP_ERROR Send();

    Messaging::ExchangeContext * GetExchange() { return mpExchangeCtx; }

private:
    enum State
    {
        kStateReady            = 0, //< The invoke command message has been initialized and is ready
        kStateAwaitingResponse = 1,
    };

    CHIP_ERROR StartCommandHeader(const CommandParams & aParams);
    void CloseExchange();

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                 const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override;

    Messaging::ExchangeContext * GetExchangeContext() { return mpExchangeCtx; }
    friend class InteractionModelEngine;

    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle & aBuf);
    CHIP_ERROR SendMessage(System::PacketBufferHandle aBuf);

private:
    enum Mode
    {
        kModeUnset           = 0,
        kModeClientInitiator = 1,
        kModeServerResponder = 2
    };

    friend class TestInvokeInteraction;
    bool mExpectsResponses = false;
    State mState           = kStateReady;
    chip::System::PacketBufferTLVWriter mWriter;
    app::InvokeCommand::Builder mInvokeCommandBuilder;
    ICommandHandler * mHandler;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
};

/*
 * @brief
 *
 * This class represents an invoke interaction from the responding side (i.e on the server). An instance of this class maps to a
 * specific invoke interaction over the wire. Consequently, the lifetime of this object is limited to a SINGLE interaction backed by
 * a SINGLE exchange context.
 *
 * This object permits a single request + response. It does not permit multiple responses to be sequenced in order.
 *
 * This object comes into existence at the point that an invoke request has to be handled. Upon initialization of the object with
 * the packet buffer, this object parses the various command data elements embedded in the message and dispatches them to
 * ClusterServer objects registered with the InteractionModel engine.
 *
 * Upon dispatch, the ClusterServer instance's OnInvokeRequest method is invoked. The cluster's OnInvokeRequest implementation has
 * to follow one of the following calling conventions:
 *      1. Handle the request and if a response needs to be generated, call AddResponse on the passed in InvokeResponder object.
 *      2. Handle the request and if no responses needs to to be generated, do nothing.
 *      3. Handle the request if a response needs to be generated asynchronously, call IncrementHoldOffRef() on the passed in
 * InvokeResponder object. This is a signal to the responder object to hold-off sending any queued up responoses till a matching
 * DecrementHoldOffRef() call is made by that handler.
 *
 */
class InvokeResponder
{
public:
    /*
     * @brief
     *
     * This interface defines the expectations for a handler of command requests.
     *
     */
    class CommandHandler
    {
    public:
        /*
         * @brief
         *
         * Called to handle a data element present within an Invoke Request message.
         *
         * @param invokeInteraction         A reference to the invoke interaction instance that corresponds to the invoke request
         * being processed.
         * @param commandParams             A reference to an object containing the parameters of the command request.
         * @param payload                   If a data payload is present, a TLV reader positioned at that payload is provided. The
         * reader shall not be assumed to exist past this call, so the application should save off the data if they desire to.
         */
        virtual CHIP_ERROR OnInvokeRequest(CommandParams & commandParams, InvokeResponder & invokeInteraction,
                                           TLV::TLVReader * payload) = 0;

        virtual ~CommandHandler() {}
    };

    /*
     * @brief
     *
     * Initializes the interaction object with the exchange context associated with the received message
     * as well as a rvalue reference to the packet buffer containing the message.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeContext * apContext, System::PacketBufferHandle && aBufferHandle);

    /*
     * @brief
     *
     * Encode a response to a request by providing command parameters as well as a pointer to an encodable object.
     * The object's contents are immediately serialized into a response packet buffer.
     *
     */
    CHIP_ERROR AddResponse(CommandParams aParams, IEncodableElement * serializable);

    /*
     * @brief
     *
     * Encode a response to a request by providing command parameters as well as a lambda that is in turn
     * invoked with a TLVWriter passed in positioned at the appropriate offset into the packet buffer.
     *
     * This uses SFINAE to ensure selection of the right overload when a closure is passed in vs. a pointer to
     * a serializable object.
     *
     */
    template <typename F, typename std::enable_if<std::is_class<F>::value, int>::type = 0>
    CHIP_ERROR AddResponse(CommandParams aParams, F f)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        err = StartCommandHeader(aParams);
        SuccessOrExit(err);

        //
        // Invoke the passed in closure that will actually write out the command payload if any
        //
        err = f(*mInvokeCommandBuilder.GetWriter(), TLV::ContextTag(CommandDataElement::kCsTag_Data));
        SuccessOrExit(err);

        mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().EndOfCommandDataElement();
        SuccessOrExit((err = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().GetError()));

    exit:
        return err;
    }

    /*
     * @brief
     *
     * Encode a status response.
     *
     */
    CHIP_ERROR AddStatusCode(const CommandParams & aParams, const StatusResponse & statusResponse);

    /*
     * @brief
     *
     * Increments the hold-off ref, signalling to the responder object that it should not dispatch any queued up responses till a
     * matching call to DecrementHoldOffRef() has been made. This also prevents the de-allocation of this object till the matching
     * call has been made.
     *
     * If the caller of this method encounters any errors, it should call DecrementHoldOffRef() to safely permit the dispach of
     * other responses on this object despite its own failure. Failure to do so will prevent futher work on this object.
     */
    void IncrementHoldOffRef();

    /*
     * @brief
     *
     * Decrements the hold-off ref. If it hits 0, any queued up responses that have been encoded into a response packet buffer are
     * immediately transmitted, before this object is released/destructed.
     */
    void DecrementHoldOffRef();

private:
    enum State
    {
        kStateReady    = 0,
        kStateReleased = 1
    };

    CHIP_ERROR Shutdown();

    Messaging::ExchangeContext * GetExchangeContext() { return mpExchangeCtx; }
    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle & aBuf);
    CHIP_ERROR SendMessage(System::PacketBufferHandle aBuf);
    friend class InteractionModelEngine;

private:
    int mHoldOffCount = 0;
    State mState;
    friend class TestInvokeInteraction;
    CHIP_ERROR StartCommandHeader(const CommandParams & aParams);
    app::InvokeCommand::Builder mInvokeCommandBuilder;
    chip::System::PacketBufferTLVWriter mWriter;
    Messaging::ExchangeContext * mpExchangeCtx = nullptr;
};

} // namespace app
} // namespace chip
