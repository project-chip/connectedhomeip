/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *      recipient (server).
 *
 */

#include "UserDirectedCommissioning.h"
#include <lib/core/CHIPSafeCasts.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <unistd.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

void UserDirectedCommissioningServer::OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msg)
{
    char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];
    source.ToString(addrBuffer);
    ChipLogProgress(AppServer, "UserDirectedCommissioningServer::OnMessageReceived from %s", addrBuffer);

    PacketHeader packetHeader;

    ReturnOnFailure(packetHeader.DecodeAndConsume(msg));

    if (packetHeader.IsEncrypted())
    {
        ChipLogError(AppServer, "UDC encryption flag set - ignoring");
        return;
    }

    PayloadHeader payloadHeader;
    ReturnOnFailure(payloadHeader.DecodeAndConsume(msg));

    ChipLogProgress(AppServer, "IdentityDeclaration DataLength()=%d", msg->DataLength());

    uint8_t udcPayload[IdentificationDeclaration::kUdcTLVDataMaxBytes];
    size_t udcPayloadLength = std::min<size_t>(msg->DataLength(), sizeof(udcPayload));
    msg->Read(udcPayload, udcPayloadLength);

    IdentificationDeclaration id;
    id.ReadPayload(udcPayload, sizeof(udcPayload));

    char * instanceName = (char *) id.GetInstanceName();

    ChipLogProgress(AppServer, "UDC instance=%s ", id.GetInstanceName());

    UDCClientState * client = mUdcClients.FindUDCClientState(instanceName);
    if (client == nullptr)
    {
        ChipLogProgress(AppServer, "UDC new instance state received");

        id.DebugLog();

        CHIP_ERROR err;
        err = mUdcClients.CreateNewUDCClientState(instanceName, &client);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "UDC error creating new connection state");
            return;
        }

        if (id.HasDiscoveryInfo())
        {
            // if we received mDNS info, skip the commissionable lookup
            ChipLogDetail(AppServer, "UDC discovery info provided");
            mUdcClients.MarkUDCClientActive(client);

            client->SetUDCClientProcessingState(UDCClientProcessingState::kPromptingUser);
            client->SetPeerAddress(source);

            id.UpdateClientState(client);

            // Call the registered mUserConfirmationProvider, if any.
            if (mUserConfirmationProvider != nullptr)
            {
                mUserConfirmationProvider->OnUserDirectedCommissioningRequest(*client);
            }
            return;
        }

        // Call the registered InstanceNameResolver, if any.
        if (mInstanceNameResolver != nullptr)
        {
            mInstanceNameResolver->FindCommissionableNode(instanceName);
        }
        else
        {
            ChipLogError(AppServer, "UserDirectedCommissioningServer::OnMessageReceived no mInstanceNameResolver registered");
        }
    }

    mUdcClients.MarkUDCClientActive(client);
}

CHIP_ERROR UserDirectedCommissioningServer::SendCDCMessage(CommissionerDeclaration cd, chip::Transport::PeerAddress peerAddress)
{
    if (mTransportMgr == nullptr)
    {
        ChipLogError(AppServer, "CDC: No transport manager\n");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    uint8_t idBuffer[IdentificationDeclaration::kUdcTLVDataMaxBytes];
    uint32_t length = cd.WritePayload(idBuffer, sizeof(idBuffer));
    if (length == 0)
    {
        ChipLogError(AppServer, "CDC: error writing payload\n");
        return CHIP_ERROR_INTERNAL;
    }

    chip::System::PacketBufferHandle payload = chip::MessagePacketBuffer::NewWithData(idBuffer, length);
    if (payload.IsNull())
    {
        ChipLogError(AppServer, "Unable to allocate packet buffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }
    ReturnErrorOnFailure(EncodeUDCMessage(payload));

    cd.DebugLog();
    ChipLogProgress(Inet, "Sending CDC msg");

    auto err = mTransportMgr->SendMessage(peerAddress, std::move(payload));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CDC SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    ChipLogProgress(Inet, "CDC msg sent");
    return CHIP_NO_ERROR;
}

CHIP_ERROR UserDirectedCommissioningServer::EncodeUDCMessage(const System::PacketBufferHandle & payload)
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

CHIP_ERROR IdentificationDeclaration::ReadPayload(uint8_t * udcPayload, size_t payloadBufferSize)
{
    size_t i = 0;
    while (i < std::min<size_t>(sizeof(mInstanceName), payloadBufferSize) && udcPayload[i] != '\0')
    {
        mInstanceName[i] = (char) udcPayload[i];
        i++;
    }
    mInstanceName[i] = '\0';

    if (payloadBufferSize <= sizeof(mInstanceName))
    {
        ChipLogProgress(AppServer, "UDC - No TLV information in Identification Declaration");
        return CHIP_NO_ERROR;
    }
    // advance i to the end of the fixed length block containing instance name
    i = sizeof(mInstanceName);

    CHIP_ERROR err;

    TLV::TLVReader reader;
    reader.Init(udcPayload + i, payloadBufferSize - i);

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
        case kVendorIdTag:
            // vendorId
            err = reader.Get(mVendorId);
            break;
        case kProductIdTag:
            // productId
            err = reader.Get(mProductId);
            break;
        case kCdPortTag:
            // port
            err = reader.Get(mCdPort);
            break;
        case kDeviceNameTag:
            // deviceName
            err = reader.GetString(mDeviceName, sizeof(mDeviceName));
            break;
        case kPairingInstTag:
            // pairingInst
            err = reader.GetString(mPairingInst, sizeof(mPairingInst));
            break;
        case kPairingHintTag:
            // pairingHint
            err = reader.Get(mPairingHint);
            break;
        case kRotatingIdTag:
            // rotatingId
            mRotatingIdLen = reader.GetLength();
            err            = reader.GetBytes(mRotatingId, sizeof(mRotatingId));
            break;
        case kTargetAppListTag:
            // app vendor list
            {
                ChipLogProgress(AppServer, "TLV found an applist");
                chip::TLV::TLVType listContainerType = chip::TLV::kTLVType_List;
                ReturnErrorOnFailure(reader.EnterContainer(listContainerType));

                while ((err = reader.Next()) == CHIP_NO_ERROR && mNumTargetAppInfos < sizeof(mTargetAppInfos))
                {
                    containerTag = reader.GetTag();
                    if (!TLV::IsContextTag(containerTag))
                    {
                        ChipLogError(AppServer, "Unexpected non-context TLV tag.");
                        return CHIP_ERROR_INVALID_TLV_TAG;
                    }
                    tagNum = static_cast<uint8_t>(chip::TLV::TagNumFromTag(containerTag));
                    if (tagNum == kTargetAppTag)
                    {
                        ReturnErrorOnFailure(reader.EnterContainer(outerContainerType));
                        uint16_t appVendorId  = 0;
                        uint16_t appProductId = 0;

                        while ((err = reader.Next()) == CHIP_NO_ERROR)
                        {
                            containerTag = reader.GetTag();
                            if (!TLV::IsContextTag(containerTag))
                            {
                                ChipLogError(AppServer, "Unexpected non-context TLV tag.");
                                return CHIP_ERROR_INVALID_TLV_TAG;
                            }
                            tagNum = static_cast<uint8_t>(chip::TLV::TagNumFromTag(containerTag));
                            if (tagNum == kAppVendorIdTag)
                            {
                                err = reader.Get(appVendorId);
                            }
                            else if (tagNum == kAppProductIdTag)
                            {
                                err = reader.Get(appProductId);
                            }
                        }
                        if (err == CHIP_END_OF_TLV)
                        {
                            ChipLogProgress(AppServer, "TLV end of struct TLV");
                            ReturnErrorOnFailure(reader.ExitContainer(outerContainerType));
                        }
                        if (appVendorId != 0)
                        {
                            mTargetAppInfos[mNumTargetAppInfos].vendorId  = appVendorId;
                            mTargetAppInfos[mNumTargetAppInfos].productId = appProductId;
                            mNumTargetAppInfos++;
                        }
                    }
                    else
                    {
                        ChipLogError(AppServer, "unrecognized tag %d", tagNum);
                    }
                }
                if (err == CHIP_END_OF_TLV)
                {
                    ChipLogProgress(AppServer, "TLV end of array");
                    ReturnErrorOnFailure(reader.ExitContainer(listContainerType));
                }
            }
            break;
        case kNoPasscodeTag:
            err = reader.Get(mNoPasscode);
            break;
        case kCdUponPasscodeDialogTag:
            err = reader.Get(mCdUponPasscodeDialog);
            break;
        case kCommissionerPasscodeTag:
            err = reader.Get(mCommissionerPasscode);
            break;
        case kCommissionerPasscodeReadyTag:
            err = reader.Get(mCommissionerPasscodeReady);
            break;
        case kCancelPasscodeTag:
            err = reader.Get(mCancelPasscode);
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "IdentificationDeclaration::ReadPayload read error %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    if (err == CHIP_END_OF_TLV)
    {
        // Exiting container
        ReturnErrorOnFailure(reader.ExitContainer(outerContainerType));
    }
    else
    {
        ChipLogError(AppServer, "IdentificationDeclaration::ReadPayload exiting early error %" CHIP_ERROR_FORMAT, err.Format());
    }

    ChipLogProgress(AppServer, "UDC TLV parse complete");
    return CHIP_NO_ERROR;
}

/**
 *  Reset the connection state to a completely uninitialized status.
 */
uint32_t CommissionerDeclaration::WritePayload(uint8_t * payloadBuffer, size_t payloadBufferSize)
{
    CHIP_ERROR err;

    chip::TLV::TLVWriter writer;

    writer.Init(payloadBuffer, payloadBufferSize);

    chip::TLV::TLVType outerContainerType = chip::TLV::kTLVType_Structure;
    VerifyOrExit(CHIP_NO_ERROR ==
                     (err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, outerContainerType)),
                 LogErrorOnFailure(err));

    VerifyOrExit(CHIP_NO_ERROR == (err = writer.Put(chip::TLV::ContextTag(kErrorCodeTag), GetErrorCode())), LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kNeedsPasscodeTag), mNeedsPasscode)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kNoAppsFoundTag), mNoAppsFound)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR ==
                     (err = writer.PutBoolean(chip::TLV::ContextTag(kPasscodeDialogDisplayedTag), mPasscodeDialogDisplayed)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kCommissionerPasscodeTag), mCommissionerPasscode)),
                 LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.PutBoolean(chip::TLV::ContextTag(kQRCodeDisplayedTag), mQRCodeDisplayed)),
                 LogErrorOnFailure(err));

    VerifyOrExit(CHIP_NO_ERROR == (err = writer.EndContainer(outerContainerType)), LogErrorOnFailure(err));
    VerifyOrExit(CHIP_NO_ERROR == (err = writer.Finalize()), LogErrorOnFailure(err));

    ChipLogProgress(AppServer, "TLV write done");

    return writer.GetLengthWritten();

exit:
    return 0;
}

void UserDirectedCommissioningServer::SetUDCClientProcessingState(char * instanceName, UDCClientProcessingState state)
{
    UDCClientState * client = mUdcClients.FindUDCClientState(instanceName);
    if (client == nullptr)
    {
        CHIP_ERROR err;
        err = mUdcClients.CreateNewUDCClientState(instanceName, &client);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer,
                         "UserDirectedCommissioningServer::SetUDCClientProcessingState error creating new connection state");
            return;
        }
    }

    ChipLogDetail(AppServer, "SetUDCClientProcessingState instance=%s new state=%d", StringOrNullMarker(instanceName), (int) state);

    client->SetUDCClientProcessingState(state);

    mUdcClients.MarkUDCClientActive(client);
}

void UserDirectedCommissioningServer::OnCommissionableNodeFound(const Dnssd::DiscoveredNodeData & nodeData)
{
    if (nodeData.resolutionData.numIPs == 0)
    {
        ChipLogError(AppServer, "OnCommissionableNodeFound no IP addresses returned for instance name=%s",
                     nodeData.commissionData.instanceName);
        return;
    }
    if (nodeData.resolutionData.port == 0)
    {
        ChipLogError(AppServer, "OnCommissionableNodeFound no port returned for instance name=%s",
                     nodeData.commissionData.instanceName);
        return;
    }

    UDCClientState * client = mUdcClients.FindUDCClientState(nodeData.commissionData.instanceName);
    if (client != nullptr && client->GetUDCClientProcessingState() == UDCClientProcessingState::kDiscoveringNode)
    {
        ChipLogDetail(AppServer, "OnCommissionableNodeFound instance: name=%s old_state=%d new_state=%d", client->GetInstanceName(),
                      (int) client->GetUDCClientProcessingState(), (int) UDCClientProcessingState::kPromptingUser);
        client->SetUDCClientProcessingState(UDCClientProcessingState::kPromptingUser);

#if INET_CONFIG_ENABLE_IPV4
        // prefer IPv4 if its an option
        bool foundV4 = false;
        for (unsigned i = 0; i < nodeData.resolutionData.numIPs; ++i)
        {
            if (nodeData.resolutionData.ipAddress[i].IsIPv4())
            {
                foundV4 = true;
                client->SetPeerAddress(
                    chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[i], nodeData.resolutionData.port));
                break;
            }
        }
        // use IPv6 as last resort
        if (!foundV4)
        {
            client->SetPeerAddress(
                chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[0], nodeData.resolutionData.port));
        }
#else  // INET_CONFIG_ENABLE_IPV4
       // if we only support V6, then try to find a v6 address
        bool foundV6 = false;
        for (unsigned i = 0; i < nodeData.resolutionData.numIPs; ++i)
        {
            if (nodeData.resolutionData.ipAddress[i].IsIPv6())
            {
                foundV6 = true;
                client->SetPeerAddress(
                    chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[i], nodeData.resolutionData.port));
                break;
            }
        }
        // last resort, try with what we have
        if (!foundV6)
        {
            ChipLogError(AppServer, "OnCommissionableNodeFound no v6 returned for instance name=%s",
                         nodeData.commissionData.instanceName);
            client->SetPeerAddress(
                chip::Transport::PeerAddress::UDP(nodeData.resolutionData.ipAddress[0], nodeData.resolutionData.port));
        }
#endif // INET_CONFIG_ENABLE_IPV4

        client->SetDeviceName(nodeData.commissionData.deviceName);
        client->SetLongDiscriminator(nodeData.commissionData.longDiscriminator);
        client->SetVendorId(nodeData.commissionData.vendorId);
        client->SetProductId(nodeData.commissionData.productId);
        client->SetRotatingId(nodeData.commissionData.rotatingId, nodeData.commissionData.rotatingIdLen);

        // Call the registered mUserConfirmationProvider, if any.
        if (mUserConfirmationProvider != nullptr)
        {
            mUserConfirmationProvider->OnUserDirectedCommissioningRequest(*client);
        }
    }
}

void UserDirectedCommissioningServer::PrintUDCClients()
{
    for (uint8_t i = 0; i < kMaxUDCClients; i++)
    {
        UDCClientState * state = GetUDCClients().GetUDCClientState(i);
        if (state == nullptr)
        {
            ChipLogProgress(AppServer, "UDC Client[%d] null", i);
        }
        else
        {
            char addrBuffer[chip::Transport::PeerAddress::kMaxToStringSize];
            state->GetPeerAddress().ToString(addrBuffer);

            char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
            Encoding::BytesToUppercaseHexString(state->GetRotatingId(), chip::Dnssd::kMaxRotatingIdLen, rotatingIdString,
                                                sizeof(rotatingIdString));

            ChipLogProgress(AppServer, "UDC Client[%d] instance=%s deviceName=%s address=%s, vid/pid=%d/%d disc=%d rid=%s", i,
                            state->GetInstanceName(), state->GetDeviceName(), addrBuffer, state->GetVendorId(),
                            state->GetProductId(), state->GetLongDiscriminator(), rotatingIdString);
        }
    }
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
