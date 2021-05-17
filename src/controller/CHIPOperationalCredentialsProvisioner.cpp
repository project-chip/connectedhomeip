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

#include <app/chip-zcl-zpro-codec-api.h>
#include <controller/CHIPOperationalCredentialsProvisioner.h>

namespace chip {
namespace Controller {

CHIP_ERROR OperationalCredentialsProvisioner::AddOpCert(Callback::Cancelable * onSuccessCallback,
                                                        Callback::Cancelable * onFailureCallback, chip::ByteSpan noc,
                                                        chip::ByteSpan iCACertificate, chip::ByteSpan iPKValue,
                                                        chip::NodeId caseAdminNode, uint16_t adminVendorId)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kAddOpCertCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = mpCommandSender->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // noc: octetString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), noc));
    // iCACertificate: octetString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), iCACertificate));
    // iPKValue: octetString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), iPKValue));
    // caseAdminNode: nodeId
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), caseAdminNode));
    // adminVendorId: int16u
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), adminVendorId));

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
}

CHIP_ERROR OperationalCredentialsProvisioner::OpCSRRequest(Callback::Cancelable * onSuccessCallback,
                                                           Callback::Cancelable * onFailureCallback, chip::ByteSpan CSRNonce)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kOpCSRRequestCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = mpCommandSender->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // CSRNonce: octetString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), CSRNonce));

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
}

CHIP_ERROR OperationalCredentialsProvisioner::RemoveAllFabrics(Callback::Cancelable * onSuccessCallback,
                                                               Callback::Cancelable * onFailureCallback)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveAllFabricsCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    // Command takes no arguments.

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
}

CHIP_ERROR OperationalCredentialsProvisioner::RemoveFabric(Callback::Cancelable * onSuccessCallback,
                                                           Callback::Cancelable * onFailureCallback, chip::FabricId fabricId,
                                                           chip::NodeId nodeId, uint16_t vendorId)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveFabricCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = mpCommandSender->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // fabricId: fabricId
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), fabricId));
    // nodeId: nodeId
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), nodeId));
    // vendorId: int16u
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), vendorId));

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
}

CHIP_ERROR OperationalCredentialsProvisioner::SetFabric(Callback::Cancelable * onSuccessCallback,
                                                        Callback::Cancelable * onFailureCallback, uint16_t vendorId)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kSetFabricCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = mpCommandSender->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // vendorId: int16u
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), vendorId));

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
}

CHIP_ERROR OperationalCredentialsProvisioner::UpdateFabricLabel(Callback::Cancelable * onSuccessCallback,
                                                                Callback::Cancelable * onFailureCallback, chip::ByteSpan label)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kUpdateFabricLabelCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = mpCommandSender->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // label: charString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), label));

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
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
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kAddTrustedRootCertificateCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = mpCommandSender->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // rootCertificate: octetString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), rootCertificate));

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
}

CHIP_ERROR TrustedRootCertificatesProvisioner::RemoveTrustedRootCertificate(Callback::Cancelable * onSuccessCallback,
                                                                            Callback::Cancelable * onFailureCallback,
                                                                            chip::ByteSpan trustedRootIdentifier)
{
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveTrustedRootCertificateCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    if (mpCommandSender == nullptr)
    {
        ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&mpCommandSender));
    }

    ReturnErrorOnFailure(mpCommandSender->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = mpCommandSender->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // trustedRootIdentifier: octetString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), trustedRootIdentifier));

    ReturnErrorOnFailure(mpCommandSender->FinishCommand());

    // #6308: This is a temporary solution before we fully support IM on application side and should be replaced by IMDelegate.
    mDevice->AddIMResponseHandler(mpCommandSender, onSuccessCallback, onFailureCallback);

    return mDevice->SendCommands(mpCommandSender);
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
