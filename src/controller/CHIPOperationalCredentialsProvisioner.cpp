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
#if CHIP_ENABLE_INTERACTION_MODEL
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    (void) onSuccessCallback;
    (void) onFailureCallback;

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kAddOpCertCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::Command * ZCLcommand        = mDevice->GetCommandSender();

    ReturnErrorOnFailure(ZCLcommand->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = ZCLcommand->GetCommandDataElementTLVWriter();
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

    ReturnErrorOnFailure(ZCLcommand->FinishCommand());

    return mDevice->SendCommands();
#else
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeOperationalCredentialsClusterAddOpCertCommand(
        seqNum, mEndpoint, noc, iCACertificate, iPKValue, caseAdminNode, adminVendorId);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
#endif
}

CHIP_ERROR OperationalCredentialsProvisioner::OpCSRRequest(Callback::Cancelable * onSuccessCallback,
                                                           Callback::Cancelable * onFailureCallback, chip::ByteSpan cSRNonce)
{
#if CHIP_ENABLE_INTERACTION_MODEL
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    (void) onSuccessCallback;
    (void) onFailureCallback;

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kOpCSRRequestCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::Command * ZCLcommand        = mDevice->GetCommandSender();

    ReturnErrorOnFailure(ZCLcommand->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = ZCLcommand->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // cSRNonce: octetString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), cSRNonce));

    ReturnErrorOnFailure(ZCLcommand->FinishCommand());

    return mDevice->SendCommands();
#else
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeOperationalCredentialsClusterOpCSRRequestCommand(seqNum, mEndpoint, cSRNonce);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
#endif
}

CHIP_ERROR OperationalCredentialsProvisioner::RemoveAllFabrics(Callback::Cancelable * onSuccessCallback,
                                                               Callback::Cancelable * onFailureCallback)
{
#if CHIP_ENABLE_INTERACTION_MODEL
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    (void) onSuccessCallback;
    (void) onFailureCallback;

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveAllFabricsCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::Command * ZCLcommand        = mDevice->GetCommandSender();

    ReturnErrorOnFailure(ZCLcommand->PrepareCommand(&cmdParams));

    // Command takes no arguments.

    ReturnErrorOnFailure(ZCLcommand->FinishCommand());

    return mDevice->SendCommands();
#else
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeOperationalCredentialsClusterRemoveAllFabricsCommand(seqNum, mEndpoint);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
#endif
}

CHIP_ERROR OperationalCredentialsProvisioner::RemoveFabric(Callback::Cancelable * onSuccessCallback,
                                                           Callback::Cancelable * onFailureCallback, chip::FabricId fabricId,
                                                           chip::NodeId nodeId, uint16_t vendorId)
{
#if CHIP_ENABLE_INTERACTION_MODEL
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    (void) onSuccessCallback;
    (void) onFailureCallback;

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kRemoveFabricCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::Command * ZCLcommand        = mDevice->GetCommandSender();

    ReturnErrorOnFailure(ZCLcommand->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = ZCLcommand->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // fabricId: fabricId
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), fabricId));
    // nodeId: nodeId
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), nodeId));
    // vendorId: int16u
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), vendorId));

    ReturnErrorOnFailure(ZCLcommand->FinishCommand());

    return mDevice->SendCommands();
#else
    uint8_t seqNum = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand =
        encodeOperationalCredentialsClusterRemoveFabricCommand(seqNum, mEndpoint, fabricId, nodeId, vendorId);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
#endif
}

CHIP_ERROR OperationalCredentialsProvisioner::SetFabric(Callback::Cancelable * onSuccessCallback,
                                                        Callback::Cancelable * onFailureCallback, uint16_t vendorId)
{
#if CHIP_ENABLE_INTERACTION_MODEL
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    (void) onSuccessCallback;
    (void) onFailureCallback;

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kSetFabricCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::Command * ZCLcommand        = mDevice->GetCommandSender();

    ReturnErrorOnFailure(ZCLcommand->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = ZCLcommand->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // vendorId: int16u
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), vendorId));

    ReturnErrorOnFailure(ZCLcommand->FinishCommand());

    return mDevice->SendCommands();
#else
    uint8_t seqNum                            = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand = encodeOperationalCredentialsClusterSetFabricCommand(seqNum, mEndpoint, vendorId);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
#endif
}

CHIP_ERROR OperationalCredentialsProvisioner::UpdateFabricLabel(Callback::Cancelable * onSuccessCallback,
                                                                Callback::Cancelable * onFailureCallback, chip::ByteSpan label)
{
#if CHIP_ENABLE_INTERACTION_MODEL
    VerifyOrReturnError(mDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    (void) onSuccessCallback;
    (void) onFailureCallback;

    app::CommandPathParams cmdParams = { mEndpoint, /* group id */ 0, mClusterId, kUpdateFabricLabelCommandId,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    app::Command * ZCLcommand        = mDevice->GetCommandSender();

    ReturnErrorOnFailure(ZCLcommand->PrepareCommand(&cmdParams));

    TLV::TLVWriter * writer = ZCLcommand->GetCommandDataElementTLVWriter();
    uint8_t argSeqNumber    = 0;
    // label: charString
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(argSeqNumber++), label));

    ReturnErrorOnFailure(ZCLcommand->FinishCommand());

    return mDevice->SendCommands();
#else
    uint8_t seqNum = mDevice->GetNextSequenceNumber();
    System::PacketBufferHandle encodedCommand =
        encodeOperationalCredentialsClusterUpdateFabricLabelCommand(seqNum, mEndpoint, label);
    return SendCommand(seqNum, std::move(encodedCommand), onSuccessCallback, onFailureCallback);
#endif
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

} // namespace Controller
} // namespace chip
