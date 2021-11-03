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
 *      This file defines object for a CHIP IM Invoke Command Handler
 *
 */

#pragma once

#include <app/Command.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Encode.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>

namespace chip {
namespace app {

class CommandHandler : public Command
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
        virtual void OnDone(CommandHandler * apCommandObj) = 0;
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
     */
    CHIP_ERROR OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                      System::PacketBufferHandle && payload);
    CHIP_ERROR AddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus) override;

    CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus) override;

    CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus) override;

    CHIP_ERROR ProcessInvokeRequest(System::PacketBufferHandle && payload);
    CHIP_ERROR PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct = true);
    CHIP_ERROR FinishCommand(bool aStartDataStruct = true);
    CHIP_ERROR PrepareStatus(const CommandPathParams & aCommandPathParams);
    CHIP_ERROR FinishStatus();
    TLV::TLVWriter * GetCommandDataIBTLVWriter();

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
        ReturnErrorOnFailure(PrepareResponse(aRequestCommandPath, CommandData::GetCommandId()));
        TLV::TLVWriter * writer = GetCommandDataIBTLVWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, TLV::ContextTag(to_underlying(CommandDataIB::Tag::kData)), aData));

        return FinishCommand(/* aEndDataStruct = */ false);
    }

private:
    friend class TestCommandInteraction;
    /*
     * Allocates a packet buffer used for encoding an invoke response payload.
     *
     * This can be called multiple times safely, as it will only allocate the buffer once for the lifetime
     * of this object.
     */
    CHIP_ERROR AllocateBuffer();

    //
    // Called internally to signal the completion of all work on this object, gracefully close the
    // exchange (by calling into the base class) and finally, signal to a registerd callback that it's
    // safe to release this object.
    //
    void Close();

    CHIP_ERROR ProcessCommandDataIB(CommandDataIB::Parser & aCommandElement);
    CHIP_ERROR SendCommandResponse();
    CHIP_ERROR PrepareResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommand);
    CHIP_ERROR AddStatusInternal(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus,
                                 const Optional<ClusterStatus> & aClusterStatus);

    Callback * mpCallback = nullptr;
    InvokeResponseMessage::Builder mInvokeResponseBuilder;
    TLV::TLVType mDataElementContainerType = TLV::kTLVType_NotSpecified;
    bool mSuppressResponse                 = false;
    bool mTimedRequest                     = false;
};
} // namespace app
} // namespace chip
