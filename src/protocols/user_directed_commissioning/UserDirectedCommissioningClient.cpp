/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements an object for a Matter User Directed Commissioning unsolicited
 *      initiator (client).
 *
 */

#include "UserDirectedCommissioning.h"
#include <transport/raw/Base.h>

#ifdef __ZEPHYR__
#include <zephyr/kernel.h>
#endif // __ZEPHYR__

#include <unistd.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

CHIP_ERROR UserDirectedCommissioningClient::SendUDCMessage(TransportMgrBase * transportMgr, IdentificationDeclaration id,
                                                           chip::Transport::PeerAddress peerAddress)
{
    uint8_t idBuffer[IdentificationDeclaration::kUdcTLVDataMaxBytes];
    uint32_t length = id.WritePayload(idBuffer, sizeof(idBuffer));
    if (length == 0)
    {
        ChipLogError(AppServer, "UDC: error writing payload\n");
        return CHIP_ERROR_INTERNAL;
    }

    chip::System::PacketBufferHandle payload = chip::MessagePacketBuffer::NewWithData(idBuffer, length);
    if (payload.IsNull())
    {
        ChipLogError(AppServer, "Unable to allocate packet buffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }
    ReturnErrorOnFailure(EncodeUDCMessage(payload));

    id.DebugLog();
    ChipLogProgress(Inet, "Sending UDC msg");

    // send UDC message 5 times per spec (no ACK on this message)
    for (unsigned int i = 0; i < 5; i++)
    {
        auto msgCopy = payload.CloneData();
        VerifyOrReturnError(!msgCopy.IsNull(), CHIP_ERROR_NO_MEMORY);

        auto err = transportMgr->SendMessage(peerAddress, std::move(msgCopy));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "UDC SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
        // Zephyr doesn't provide usleep implementation.
#ifdef __ZEPHYR__
        k_usleep(100 * 1000); // 100ms
#else
        usleep(100 * 1000); // 100ms
#endif // __ZEPHYR__
    }

    ChipLogProgress(Inet, "UDC msg sent");
    return CHIP_NO_ERROR;
}

CHIP_ERROR UserDirectedCommissioningClient::EncodeUDCMessage(const System::PacketBufferHandle & payload)
{
    PayloadHeader payloadHeader;
    PacketHeader packetHeader;

    payloadHeader.SetMessageType(MsgType::IdentificationDeclaration).SetInitiator(true).SetNeedsAck(false);

    VerifyOrReturnError(!payload.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!payload->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrReturnError(payload->TotalLength() <= kMaxAppMessageLen, CHIP_ERROR_MESSAGE_TOO_LONG);

    ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(payload));

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(payload));

    return CHIP_NO_ERROR;
}

/**
 *  Reset the connection state to a completely uninitialized status.
 */
uint32_t IdentificationDeclaration::WritePayload(uint8_t * payloadBuffer, size_t payloadBufferSize)
{
    CHIP_ERROR err;

    chip::TLV::TLVWriter writer;
    chip::TLV::TLVType listContainerType = chip::TLV::kTLVType_List;

    memcpy(payloadBuffer, mInstanceName, sizeof(mInstanceName));

    writer.Init(payloadBuffer + sizeof(mInstanceName), payloadBufferSize - sizeof(mInstanceName));

    chip::TLV::TLVType outerContainerType = chip::TLV::kTLVType_Structure;
    VerifyOrExit(CHIP_NO_ERROR ==
                     (err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerContainerType)),
                 LogErrorOnFailure(err));

    VerifyOrExit(CHIP_NO_ERROR == (err = writer.Put(chip::TLV::ContextTag(kVendorIdTag), GetVendorId())), LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.Put(chip::TLV::ContextTag(kProductIdTag), GetProductId())), LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutString(chip::TLV::ContextTag(kDeviceNameTag), mDeviceName)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutString(chip::TLV::ContextTag(kPairingInstTag), mPairingInst)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.Put(chip::TLV::ContextTag(kPairingHintTag), mPairingHint)), LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.Put(chip::TLV::ContextTag(kCdPortTag), GetCdPort())), LogErrorOnFailure(err));

    VerifyOrExit(
        CHIP_NO_ERROR ==
            (err = writer.PutBytes(chip::TLV::ContextTag(kRotatingIdTag), mRotatingId, static_cast<uint8_t>(mRotatingIdLen))),
        LogErrorOnFailure(err));

    if (mNumTargetAppInfos > 0)
    {
        // AppVendorIdList
        VerifyOrExit(CHIP_NO_ERROR ==
                         (err = writer.StartContainer(chip::TLV::ContextTag(kTargetAppListTag), chip::TLV::kTLVType_List,
                                                      listContainerType)),
                     LogErrorOnFailure(err));
        for (size_t i = 0; i < mNumTargetAppInfos; i++)
        {
            // start the TargetApp structure
            VerifyOrExit(CHIP_NO_ERROR ==
                             (err = writer.StartContainer(chip::TLV::ContextTag(kTargetAppTag), chip::TLV::kTLVType_Structure,
                                                          outerContainerType)),
                         LogErrorOnFailure(err));
            // add the vendor Id
            VerifyOrExit(CHIP_NO_ERROR == (err = writer.Put(chip::TLV::ContextTag(kAppVendorIdTag), mTargetAppInfos[i].vendorId)),
                         LogErrorOnFailure(err));
            VerifyOrExit(CHIP_NO_ERROR == (err = writer.Put(chip::TLV::ContextTag(kAppProductIdTag), mTargetAppInfos[i].productId)),
                         LogErrorOnFailure(err));
            // end the TargetApp structure
            VerifyOrExit(CHIP_NO_ERROR == (err = writer.EndContainer(outerContainerType)), LogErrorOnFailure(err));
        }
        VerifyOrExit(CHIP_NO_ERROR == (err = writer.EndContainer(listContainerType)), LogErrorOnFailure(err));
    }

    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kNoPasscodeTag), mNoPasscode)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kCdUponPasscodeDialogTag), mCdUponPasscodeDialog)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kCommissionerPasscodeTag), mCommissionerPasscode)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR ==
                     (err = writer.PutBoolean(chip::TLV::ContextTag(kCommissionerPasscodeReadyTag), mCommissionerPasscodeReady)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kCancelPasscodeTag), mCancelPasscode)),
                 LogErrorOnFailure(err));

    VerifyOrExit(CHIP_NO_ERROR == (err = writer.EndContainer(outerContainerType)), LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.Finalize()), LogErrorOnFailure(err));

    return writer.GetLengthWritten() + static_cast<uint32_t>(sizeof(mInstanceName));

exit:
    ChipLogError(AppServer, "IdentificationDeclaration::WritePayload exiting early error %" CHIP_ERROR_FORMAT, err.Format());
    return 0;
}

CHIP_ERROR CommissionerDeclaration::ReadPayload(uint8_t * udcPayload, size_t payloadBufferSize)
{
    CHIP_ERROR err;

    TLV::TLVReader reader;
    reader.Init(udcPayload, payloadBufferSize);

    // read the envelope
    ReturnErrorOnFailure(reader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag()));

    chip::TLV::TLVType outerContainerType = chip::TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.EnterContainer(outerContainerType));

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        chip::TLV::Tag containerTag = reader.GetTag();
        if (!TLV::IsContextTag(containerTag))
        {
            ChipLogError(AppServer, "Unexpected non-context TLV tag.");
            return CHIP_ERROR_INVALID_TLV_TAG;
        }
        uint8_t tagNum = static_cast<uint8_t>(chip::TLV::TagNumFromTag(containerTag));

        switch (tagNum)
        {
        case kErrorCodeTag:
            err = reader.Get(mErrorCode);
            break;
        case kNeedsPasscodeTag:
            err = reader.Get(mNeedsPasscode);
            break;
        case kNoAppsFoundTag:
            err = reader.Get(mNoAppsFound);
            break;
        case kPasscodeDialogDisplayedTag:
            err = reader.Get(mPasscodeDialogDisplayed);
            break;
        case kCommissionerPasscodeTag:
            err = reader.Get(mCommissionerPasscode);
            break;
        case kQRCodeDisplayedTag:
            err = reader.Get(mQRCodeDisplayed);
            break;
        case kCancelPasscodeTag:
            err = reader.Get(mCancelPasscode);
            break;
        }
    }

    if (err == CHIP_END_OF_TLV)
    {
        // Exiting container
        ReturnErrorOnFailure(reader.ExitContainer(outerContainerType));
    }

    ChipLogProgress(AppServer, "UDC TLV parse complete");
    return CHIP_NO_ERROR;
}

void UserDirectedCommissioningClient::OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msg,
                                                        Transport::MessageTransportContext * ctxt)
{
    char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];
    source.ToString(addrBuffer);

    ChipLogProgress(AppServer, "UserDirectedCommissioningClient::OnMessageReceived() from %s", addrBuffer);

    PacketHeader packetHeader;

    ReturnOnFailure(packetHeader.DecodeAndConsume(msg));

    if (packetHeader.IsEncrypted())
    {
        ChipLogError(AppServer, "UserDirectedCommissioningClient::OnMessageReceived() UDC encryption flag set - ignoring");
        return;
    }

    PayloadHeader payloadHeader;
    ReturnOnFailure(payloadHeader.DecodeAndConsume(msg));

    ChipLogProgress(AppServer,
                    "UserDirectedCommissioningClient::OnMessageReceived() CommissionerDeclaration DataLength() = %" PRIu32,
                    static_cast<uint32_t>(msg->DataLength()));

    uint8_t udcPayload[IdentificationDeclaration::kUdcTLVDataMaxBytes];
    size_t udcPayloadLength = std::min<size_t>(msg->DataLength(), sizeof(udcPayload));
    msg->Read(udcPayload, udcPayloadLength);

    CommissionerDeclaration cd;
    cd.ReadPayload(udcPayload, sizeof(udcPayload));
    cd.DebugLog();

    // Call the registered mCommissionerDeclarationHandler, if any.
    if (mCommissionerDeclarationHandler != nullptr)
    {
        mCommissionerDeclarationHandler->OnCommissionerDeclarationMessage(source, cd);
    }
    else
    {
        ChipLogProgress(AppServer, "UserDirectedCommissioningClient::OnMessageReceived() No registered handler for UDC messages");
    }
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
