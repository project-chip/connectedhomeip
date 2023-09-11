/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "CloseSessionCommand.h"
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>

using namespace chip;
using namespace chip::Protocols;

CHIP_ERROR SendCloseSessionCommand::RunCommand()
{
    CommissioneeDeviceProxy * commissioneeDeviceProxy = nullptr;
    if (CHIP_NO_ERROR == CurrentCommissioner().GetDeviceBeingCommissioned(mDestinationNodeId, &commissioneeDeviceProxy))
    {
        VerifyOrReturnError(commissioneeDeviceProxy->GetSecureSession().HasValue(), CHIP_ERROR_INCORRECT_STATE);
        return CloseSession(*commissioneeDeviceProxy->GetExchangeManager(), commissioneeDeviceProxy->GetSecureSession().Value());
    }

    return CurrentCommissioner().GetConnectedDevice(mDestinationNodeId, &mOnDeviceConnectedCallback,
                                                    &mOnDeviceConnectionFailureCallback);
}

CHIP_ERROR SendCloseSessionCommand::CloseSession(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    // TODO perhaps factor out this code into something on StatusReport that
    // takes an exchange and maybe a SendMessageFlags?
    SecureChannel::StatusReport statusReport(SecureChannel::GeneralStatusCode::kSuccess, SecureChannel::Id,
                                             SecureChannel::kProtocolCodeCloseSession);

    size_t reportSize = statusReport.Size();
    auto packetBuffer = MessagePacketBuffer::New(reportSize);
    VerifyOrReturnError(!packetBuffer.IsNull(), CHIP_ERROR_NO_MEMORY);
    Encoding::LittleEndian::PacketBufferWriter bbuf(std::move(packetBuffer), reportSize);
    statusReport.WriteToBuffer(bbuf);

    System::PacketBufferHandle msg = bbuf.Finalize();
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

    auto * exchange = exchangeMgr.NewContext(sessionHandle, nullptr);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);

    // Per spec, CloseSession reports are always sent with MRP disabled.
    CHIP_ERROR err =
        exchange->SendMessage(SecureChannel::MsgType::StatusReport, std::move(msg), Messaging::SendMessageFlags::kNoAutoRequestAck);
    if (err == CHIP_NO_ERROR)
    {
        if (mEvictLocalSession.ValueOr(true))
        {
            sessionHandle->AsSecureSession()->MarkForEviction();
        }
        SetCommandExitStatus(CHIP_NO_ERROR);
    }
    else
    {
        exchange->Close();
    }

    return err;
}

void SendCloseSessionCommand::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                  const SessionHandle & sessionHandle)
{
    auto * command = reinterpret_cast<SendCloseSessionCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));

    CHIP_ERROR err = command->CloseSession(exchangeMgr, sessionHandle);
    VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
}

void SendCloseSessionCommand::OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    auto * command = reinterpret_cast<SendCloseSessionCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    command->SetCommandExitStatus(err);
}

CHIP_ERROR EvictLocalCASESessionsCommand::RunCommand()
{
    auto & controller   = CurrentCommissioner();
    auto sessionManager = controller.SessionMgr();

    sessionManager->ExpireAllSessions(ScopedNodeId(mDestinationNodeId, controller.GetFabricIndex()));

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}
