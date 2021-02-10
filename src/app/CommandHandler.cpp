/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *      This file defines object for a CHIP IM Invoke Command Handler
 *
 */

#include "CommandHandler.h"
#include "Command.h"
#include "CommandSender.h"
#include "InteractionModelEngine.h"

#include <support/ReturnMacros.h>

namespace chip {
namespace app {
void CommandHandler::OnMessageReceived(Messaging::ExchangeContext * ec, const PacketHeader & packetHeader,
                                       const PayloadHeader & payloadHeader, System::PacketBufferHandle payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle response;

    // NOTE: we already know this is an InvokeCommand Request message because we explicitly registered with the
    // Exchange Manager for unsolicited InvokeCommand Requests.

    mpExchangeCtx = ec;

    err = ProcessCommandMessage(std::move(payload), kCommandHandlerId);
    SuccessOrExit(err);

    SendCommandResponse();

exit:
    ChipLogFunctError(err);
}

CHIP_ERROR CommandHandler::SendCommandResponse()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = FinalizeCommandsMessage();
    SuccessOrExit(err);

    VerifyOrExit(mpExchangeCtx != NULL, err = CHIP_ERROR_INCORRECT_STATE);
    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandResponse, std::move(mCommandMessageBuf),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kNone));
    SuccessOrExit(err);

    MoveToState(kState_Sending);

exit:
    Shutdown();
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR CommandHandler::ProcessCommandDataElement(CommandDataElement::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPath::Parser commandPath;
    chip::TLV::TLVReader commandDataReader;
    chip::ClusterId clusterId;
    chip::CommandId commandId;
    chip::EndpointId endpointId;

    ReturnErrorOnFailure(aCommandElement.GetCommandPath(&commandPath));
    ReturnErrorOnFailure(commandPath.GetNamespacedClusterId(&clusterId));
    ReturnErrorOnFailure(commandPath.GetCommandId(&commandId));
    ReturnErrorOnFailure(commandPath.GetEndpointId(&endpointId));

    err = aCommandElement.GetData(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        // Empty Command, Add status code in invoke command response, notify cluster handler to hand it further.
        err = CHIP_NO_ERROR;
        ChipLogDetail(DataManagement, "Add Status code for empty command, cluster Id is %d", clusterId);
        // Todo: Define ProtocolCode for StatusCode.
        AddStatusCode(COMMON_STATUS_SUCCESS, chip::Protocols::kProtocol_Protocol_Common, 0, clusterId);
    }
    else if (CHIP_NO_ERROR == err)
    {
        DispatchSingleClusterCommand(clusterId, commandId, endpointId, commandDataReader, this);
    }

    return err;
}
} // namespace app
} // namespace chip
