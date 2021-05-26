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

#include <controller/CHIPOperationalCredentialsProvisioner.h>
#include <controller/data_model/gen/chip-zcl-zpro-codec-api.h>

namespace chip {
namespace Controller {

CHIP_ERROR OperationalCredentialsProvisioner::AddOpCert(Callback::Cancelable * onSuccessCallback,
                                                        Callback::Cancelable * onFailureCallback, chip::ByteSpan noc,
                                                        chip::ByteSpan iCACertificate, chip::ByteSpan iPKValue,
                                                        chip::NodeId caseAdminNode, uint16_t adminVendorId)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    TLV::TLVWriter * writer     = nullptr;
    uint8_t argSeqNumber        = 0;

    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kAddOpCertCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    VerifyOrExit((writer = sender->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // noc: octetString
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), noc));
    // iCACertificate: octetString
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), iCACertificate));
    // iPKValue: octetString
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), iPKValue));
    // caseAdminNode: nodeId
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), caseAdminNode));
    // adminVendorId: int16u
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), adminVendorId));

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

CHIP_ERROR OperationalCredentialsProvisioner::OpCSRRequest(Callback::Cancelable * onSuccessCallback,
                                                           Callback::Cancelable * onFailureCallback, chip::ByteSpan CSRNonce)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    TLV::TLVWriter * writer     = nullptr;
    uint8_t argSeqNumber        = 0;

    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kOpCSRRequestCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    VerifyOrExit((writer = sender->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // CSRNonce: octetString
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), CSRNonce));

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

CHIP_ERROR OperationalCredentialsProvisioner::RemoveAllFabrics(Callback::Cancelable * onSuccessCallback,
                                                               Callback::Cancelable * onFailureCallback)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveAllFabricsCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    // Command takes no arguments.

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

CHIP_ERROR OperationalCredentialsProvisioner::RemoveFabric(Callback::Cancelable * onSuccessCallback,
                                                           Callback::Cancelable * onFailureCallback, chip::FabricId fabricId,
                                                           chip::NodeId nodeId, uint16_t vendorId)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    TLV::TLVWriter * writer     = nullptr;
    uint8_t argSeqNumber        = 0;

    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveFabricCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    VerifyOrExit((writer = sender->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // fabricId: fabricId
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), fabricId));
    // nodeId: nodeId
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), nodeId));
    // vendorId: int16u
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), vendorId));

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

CHIP_ERROR OperationalCredentialsProvisioner::SetFabric(Callback::Cancelable * onSuccessCallback,
                                                        Callback::Cancelable * onFailureCallback, uint16_t vendorId)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    TLV::TLVWriter * writer     = nullptr;
    uint8_t argSeqNumber        = 0;

    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kSetFabricCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    VerifyOrExit((writer = sender->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // vendorId: int16u
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), vendorId));

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

CHIP_ERROR OperationalCredentialsProvisioner::UpdateFabricLabel(Callback::Cancelable * onSuccessCallback,
                                                                Callback::Cancelable * onFailureCallback, chip::ByteSpan label)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    TLV::TLVWriter * writer     = nullptr;
    uint8_t argSeqNumber        = 0;
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kUpdateFabricLabelCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    VerifyOrExit((writer = sender->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // label: charString
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), label));

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

// OperationalCredentials Cluster Attributes
CHIP_ERROR OperationalCredentialsProvisioner::DiscoverAttributes(Callback::Cancelable * onSuccessCallback,
                                                                 Callback::Cancelable * onFailureCallback)
{
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeOperationalCredentialsClusterDiscoverAttributes(seqNum, mEndpoint);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
}
CHIP_ERROR OperationalCredentialsProvisioner::ReadAttributeFabricsList(Callback::Cancelable * onSuccessCallback,
                                                                       Callback::Cancelable * onFailureCallback)
{
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeOperationalCredentialsClusterReadFabricsListAttribute(seqNum, mEndpoint);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
}

CHIP_ERROR OperationalCredentialsProvisioner::ReadAttributeClusterRevision(Callback::Cancelable * onSuccessCallback,
                                                                           Callback::Cancelable * onFailureCallback)
{
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeOperationalCredentialsClusterReadClusterRevisionAttribute(seqNum, mEndpoint);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
}

// TrustedRootCertificates Cluster Commands
CHIP_ERROR TrustedRootCertificatesProvisioner::AddTrustedRootCertificate(Callback::Cancelable * onSuccessCallback,
                                                                         Callback::Cancelable * onFailureCallback,
                                                                         chip::ByteSpan rootCertificate)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    TLV::TLVWriter * writer     = nullptr;
    uint8_t argSeqNumber        = 0;
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kAddTrustedRootCertificateCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    VerifyOrExit((writer = sender->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // rootCertificate: octetString
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), rootCertificate));

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

CHIP_ERROR TrustedRootCertificatesProvisioner::RemoveTrustedRootCertificate(Callback::Cancelable * onSuccessCallback,
                                                                            Callback::Cancelable * onFailureCallback,
                                                                            chip::ByteSpan trustedRootIdentifier)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    app::CommandSender * sender = nullptr;
    TLV::TLVWriter * writer     = nullptr;
    uint8_t argSeqNumber        = 0;

    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveTrustedRootCertificateCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(err = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&sender));

    SuccessOrExit(err = sender->PrepareCommand(&cmdParams));

    VerifyOrExit((writer = sender->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    // trustedRootIdentifier: octetString
    SuccessOrExit(err = writer->Put(TLV::ContextTag(argSeqNumber++), trustedRootIdentifier));

    SuccessOrExit(err = sender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(sender, onSuccessCallback, onFailureCallback);

    err = mDevice->SendCommands(sender);

exit:
    // On error, we are responsible to close the sender.
    if (err != CHIP_NO_ERROR)
    {
        sender->Shutdown();
    }
    return err;
}

// TrustedRootCertificates Cluster Attributes
CHIP_ERROR TrustedRootCertificatesProvisioner::DiscoverAttributes(Callback::Cancelable * onSuccessCallback,
                                                                  Callback::Cancelable * onFailureCallback)
{
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeTrustedRootCertificatesClusterDiscoverAttributes(seqNum, mEndpoint);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
}
CHIP_ERROR TrustedRootCertificatesProvisioner::ReadAttributeClusterRevision(Callback::Cancelable * onSuccessCallback,
                                                                            Callback::Cancelable * onFailureCallback)
{
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeTrustedRootCertificatesClusterReadClusterRevisionAttribute(seqNum, mEndpoint);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
}

} // namespace Controller
} // namespace chip
