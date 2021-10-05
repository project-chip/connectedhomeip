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
#include <app/MessageDef/CommandDataElement.h>
#include <app/data-model/Encode.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
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

class CommandHandler : public Command
{
public:
    CHIP_ERROR OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                      System::PacketBufferHandle && payload);
    CHIP_ERROR AddStatusCode(const CommandPathParams & aCommandPathParams,
                             const Protocols::SecureChannel::GeneralStatusCode aGeneralCode, const Protocols::Id aProtocolId,
                             const Protocols::InteractionModel::Status aStatus) override;

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
        ReturnErrorOnFailure(PrepareResponse(aRequestCommandPath, CommandData::CommandId));
        TLV::TLVWriter * writer = GetCommandDataElementTLVWriter();
        VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(DataModel::Encode(*writer, TLV::ContextTag(CommandDataElement::kCsTag_Data), aData));

        return FinishCommand(/* aEndDataStruct = */ false);
    }

private:
    friend class TestCommandInteraction;
    CHIP_ERROR SendCommandResponse();
    CHIP_ERROR ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement) override;
    CHIP_ERROR PrepareResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommand);
};
} // namespace app
} // namespace chip
