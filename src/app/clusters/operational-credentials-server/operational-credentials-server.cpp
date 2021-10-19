/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Operational Credentials Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/PeerId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <string.h>
#include <transport/FabricTable.h>

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Transport;
using namespace chip::app;
using namespace chip::app::Clusters::OperationalCredentials;

namespace {

constexpr uint8_t kDACCertificate = 1;
constexpr uint8_t kPAICertificate = 2;

class OperationalCredentialsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the OperationalCredentials cluster on all endpoints.
    OperationalCredentialsAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::OperationalCredentials::Id)
    {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadFabricsList(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

CHIP_ERROR OperationalCredentialsAttrAccess::ReadFabricsList(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
        {
            if (!fabricInfo.IsInitialized())
                continue;

            Clusters::OperationalCredentials::Structs::FabricDescriptor::Type fabricDescriptor;

            fabricDescriptor.fabricIndex = fabricInfo.GetFabricIndex();
            fabricDescriptor.nodeId      = fabricInfo.GetPeerId().GetNodeId();
            fabricDescriptor.vendorId    = fabricInfo.GetVendorId();
            fabricDescriptor.fabricId    = fabricInfo.GetFabricId();

            // TODO: The type of 'label' should be 'CharSpan', need to fix the XML definition for broken member type.
            fabricDescriptor.label =
                ByteSpan(Uint8::from_const_char(fabricInfo.GetFabricLabel().data()), fabricInfo.GetFabricLabel().size());
            fabricDescriptor.rootPublicKey = fabricInfo.GetRootPubkey();

            ReturnErrorOnFailure(encoder.Encode(fabricDescriptor));
        }

        return CHIP_NO_ERROR;
    });
}

OperationalCredentialsAttrAccess gAttrAccess;

CHIP_ERROR OperationalCredentialsAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::OperationalCredentials::Id);

    switch (aPath.mAttributeId)
    {
    case Attributes::FabricsList::Id: {
        return ReadFabricsList(aPath.mEndpointId, aEncoder);
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}
} // anonymous namespace

// As per specifications section 11.22.5.1. Constant RESP_MAX
constexpr uint16_t kMaxRspLen = 900;

/*
 * Temporary flow for fabric management until addOptCert + fabric index are implemented:
 * 1) When Commissioner pairs with CHIP device, store device nodeId in Fabric table as NodeId
 *    and store commissioner nodeId in Fabric table as FabricId (This is temporary until AddOptCert is implemented and
 * Fabrics are implemented correctely) 2) When pairing is complete, commissioner calls SetFabric to set the vendorId on the newly
 * created fabric. The corresponding fabric is found by looking in fabric table and finding a fabric that has the matching
 * commissioner node ID as fabricId + device nodeId as nodeId and an uninitialized vendorId. 3) RemoveFabric uses the passed in
 * fabricId, nodeId, vendorID to find matching entry and remove it from fabric table. Once fabricIndex is implemented, it
 * should use that instead.
 */

EmberAfStatus writeFabricAttribute(uint8_t * buffer, int32_t index = -1)
{
    EmberAfAttributeSearchRecord record;
    record.endpoint         = 0;
    record.clusterId        = ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID;
    record.clusterMask      = CLUSTER_MASK_SERVER;
    record.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
    record.attributeId      = ZCL_FABRICS_ATTRIBUTE_ID;

    // When reading or writing a List attribute the 'index' value could have 3 types of values:
    //  -1: Read/Write the whole list content, including the number of elements in the list
    //   0: Read/Write the number of elements in the list, represented as a uint16_t
    //   n: Read/Write the nth element of the list
    //
    // Since the first 2 bytes of the attribute are used to store the number of elements, elements indexing starts
    // at 1. In order to hide this to the rest of the code of this file, the element index is incremented by 1 here.
    // This also allows calling writeAttribute() with no index arg to mean "write the length".

    return emAfReadOrWriteAttribute(&record,
                                    NULL, // metadata
                                    buffer,
                                    0,    // read length
                                    true, // write ?
                                    index + 1);
}

EmberAfStatus writeFabric(FabricIndex fabricIndex, FabricId fabricId, NodeId nodeId, uint16_t vendorId, CharSpan & fabricLabel,
                          Credentials::P256PublicKeySpan rootPubkey, uint8_t index)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    auto * fabricDescriptor = chip::Platform::New<::FabricDescriptor>();
    VerifyOrReturnError(fabricDescriptor != nullptr, EMBER_ZCL_STATUS_FAILURE);

    fabricDescriptor->FabricIndex   = fabricIndex;
    fabricDescriptor->RootPublicKey = ByteSpan(rootPubkey.data(), rootPubkey.size());

    fabricDescriptor->VendorId = vendorId;
    fabricDescriptor->FabricId = fabricId;
    fabricDescriptor->NodeId   = nodeId;
    if (!fabricLabel.empty())
    {
        fabricDescriptor->Label = ByteSpan(Uint8::from_const_char(fabricLabel.data()), fabricLabel.size());
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                   "OpCreds: Writing fabric into attribute store at index %d: fabricId 0x" ChipLogFormatX64
                   ", nodeId 0x" ChipLogFormatX64 " vendorId 0x%04" PRIX16,
                   index, ChipLogValueX64(fabricId), ChipLogValueX64(nodeId), vendorId);
    status = writeFabricAttribute((uint8_t *) fabricDescriptor, static_cast<int32_t>(index));
    chip::Platform::Delete(fabricDescriptor);
    return status;
}

CHIP_ERROR writeFabricsIntoFabricsListAttribute()
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Call to writeFabricsIntoFabricsListAttribute");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Loop through fabrics
    uint8_t fabricIndex = 0;
    for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
    {
        NodeId nodeId        = fabricInfo.GetPeerId().GetNodeId();
        uint64_t fabricId    = fabricInfo.GetFabricId();
        uint16_t vendorId    = fabricInfo.GetVendorId();
        CharSpan fabricLabel = fabricInfo.GetFabricLabel();

        // Skip over uninitialized fabrics
        if (nodeId == kUndefinedNodeId)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                           "OpCreds: Skipping over uninitialized fabric with fabricId 0x" ChipLogFormatX64
                           ", nodeId 0x" ChipLogFormatX64 " vendorId 0x%04" PRIX16,
                           ChipLogValueX64(fabricId), ChipLogValueX64(nodeId), vendorId);
            continue;
        }
        else if (writeFabric(fabricInfo.GetFabricIndex(), fabricId, nodeId, vendorId, fabricLabel, fabricInfo.GetRootPubkey(),
                             fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                           "OpCreds: Failed to write fabric with fabricId 0x" ChipLogFormatX64 " in fabrics list",
                           ChipLogValueX64(fabricId));
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
            break;
        }
        fabricIndex++;
    }

    // Store the count of fabrics we just stored
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Stored %" PRIu8 " fabrics in fabrics list attribute.", fabricIndex);
    uint16_t u16Index = fabricIndex;
    if (writeFabricAttribute(reinterpret_cast<uint8_t *>(&u16Index)) != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write fabric count %" PRIu8 " in fabrics list", fabricIndex);
        err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    if (err == CHIP_NO_ERROR && Attributes::CommissionedFabrics::Set(0, fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write fabrics count %" PRIu8 " in commissioned fabrics",
                       fabricIndex);
        err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    if (err == CHIP_NO_ERROR && Attributes::SupportedFabrics::Set(0, CHIP_CONFIG_MAX_DEVICE_ADMINS) != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write %d in supported fabrics count attribute",
                       CHIP_CONFIG_MAX_DEVICE_ADMINS);
        err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    return err;
}

static FabricInfo * retrieveCurrentFabric()
{
    if (emberAfCurrentCommand()->source == nullptr)
    {
        return nullptr;
    }

    FabricIndex index = emberAfCurrentCommand()->source->GetSecureSession().GetFabricIndex();
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Finding fabric with fabricIndex %d", index);
    return Server::GetInstance().GetFabricTable().FindFabricWithIndex(index);
}

// TODO: The code currently has two sources of truths for fabrics, the fabricInfo table + the attributes. There should only be one,
// the attributes list. Currently the attributes are not persisted so we are keeping the fabric table to have the
// fabrics/admrins be persisted. Once attributes are persisted, there should only be one sorce of truth, the attributes list and
// only that should be modifed to perosst/read/write fabrics.
// TODO: Once attributes are persisted, implement reading/writing/manipulation fabrics around that and remove fabricTable
// logic.
class OpCredsFabricTableDelegate : public FabricTableDelegate
{

    // Gets called when a fabric is deleted from KVS store
    void OnFabricDeletedFromStorage(FabricIndex fabricId) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Fabric 0x%" PRIX16 " was deleted from fabric storage.", fabricId);
        writeFabricsIntoFabricsListAttribute();
    }

    // Gets called when a fabric is loaded into the FabricTable from KVS store.
    void OnFabricRetrievedFromStorage(FabricInfo * fabric) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "OpCreds: Fabric 0x%" PRIX16 " was retrieved from storage. FabricId 0x" ChipLogFormatX64
                       ", NodeId 0x" ChipLogFormatX64 ", VendorId 0x%04" PRIX16,
                       fabric->GetFabricIndex(), ChipLogValueX64(fabric->GetFabricId()),
                       ChipLogValueX64(fabric->GetPeerId().GetNodeId()), fabric->GetVendorId());
        writeFabricsIntoFabricsListAttribute();
    }

    // Gets called when a fabric in FabricTable is persisted to KVS store.
    void OnFabricPersistedToStorage(FabricInfo * fabric) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "OpCreds: Fabric %" PRIX8 " was persisted to storage. FabricId " ChipLogFormatX64
                       ", NodeId " ChipLogFormatX64 ", VendorId 0x%04" PRIX16,
                       fabric->GetFabricIndex(), ChipLogValueX64(fabric->GetFabricId()),
                       ChipLogValueX64(fabric->GetPeerId().GetNodeId()), fabric->GetVendorId());
        writeFabricsIntoFabricsListAttribute();
    }
};

OpCredsFabricTableDelegate gFabricDelegate;

void MatterOperationalCredentialsPluginServerInitCallback(void)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Initiating OpCreds cluster by writing fabrics list from fabric table.");

#if CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
    registerAttributeAccessOverride(&gAttrAccess);
#endif

    Server::GetInstance().GetFabricTable().SetFabricDelegate(&gFabricDelegate);
    writeFabricsIntoFabricsListAttribute();
}

namespace {
class FabricCleanupExchangeDelegate : public Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
    void OnExchangeClosing(Messaging::ExchangeContext * ec) override
    {
        FabricIndex currentFabricIndex = ec->GetSecureSession().GetFabricIndex();
        ec->GetExchangeMgr()->GetSessionManager()->ExpireAllPairingsForFabric(currentFabricIndex);
    }
};

FabricCleanupExchangeDelegate gFabricCleanupExchangeDelegate;

} // namespace

bool emberAfOperationalCredentialsClusterRemoveFabricCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::RemoveFabric::DecodableType & commandData)
{
    auto & fabricBeingRemoved = commandData.fabricIndex;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: RemoveFabric"); // TODO: Generate emberAfFabricClusterPrintln

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    CHIP_ERROR err       = Server::GetInstance().GetFabricTable().Delete(fabricBeingRemoved);
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    app::DnssdServer::Instance().StartServer();

exit:
    writeFabricsIntoFabricsListAttribute();
    emberAfSendImmediateDefaultResponse(status);
    if (err == CHIP_NO_ERROR)
    {
        Messaging::ExchangeContext * ec = commandObj->GetExchangeContext();
        FabricIndex currentFabricIndex  = ec->GetSecureSession().GetFabricIndex();
        if (currentFabricIndex == fabricBeingRemoved)
        {
            // If the current fabric is being removed, expiring all the secure sessions causes crashes as
            // the message sent by emberAfSendImmediateDefaultResponse() is still in the queue. Also, RMP
            // retries to send the message and runs into issues.
            // We are hijacking the exchange delegate here (as no more messages should be received on this exchange),
            // and wait for it to close, before expiring the secure sessions for the fabric.
            // TODO: https://github.com/project-chip/connectedhomeip/issues/9642
            ec->SetDelegate(&gFabricCleanupExchangeDelegate);
        }
        else
        {
            ec->GetExchangeMgr()->GetSessionManager()->ExpireAllPairingsForFabric(fabricBeingRemoved);
        }
    }
    return true;
}

bool emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(app::CommandHandler * commandObj,
                                                                   const app::ConcreteCommandPath & commandPath,
                                                                   const Commands::UpdateFabricLabel::DecodableType & commandData)
{
    auto & Label = commandData.label;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: UpdateFabricLabel");

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    CHIP_ERROR err;

    // Fetch current fabric
    FabricInfo * fabric = retrieveCurrentFabric();
    VerifyOrExit(fabric != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    // Set Label on fabric
    err = fabric->SetFabricLabel(Label);
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    // Persist updated fabric
    err = Server::GetInstance().GetFabricTable().Store(fabric->GetFabricIndex());
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    writeFabricsIntoFabricsListAttribute();
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

namespace {

FabricInfo gFabricBeingCommissioned;

CHIP_ERROR SendNOCResponse(app::Command * commandObj, EmberAfNodeOperationalCertStatus status, uint8_t index, ByteSpan debug_text)
{
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID,
                                         ZCL_NOC_RESPONSE_COMMAND_ID, (app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;

    VerifyOrReturnError(commandObj != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(commandObj->PrepareCommand(cmdParams));
    writer = commandObj->GetCommandDataElementTLVWriter();
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(0), status));
    if (status == EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS)
    {
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(1), index));
    }
    // TODO: Change DebugText to CHAR_STRING once strings are supported in command/response fields
    ReturnErrorOnFailure(writer->Put(TLV::ContextTag(2), debug_text));
    return commandObj->FinishCommand();
}

EmberAfNodeOperationalCertStatus ConvertToNOCResponseStatus(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        return EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS;
    }
    else if (err == CHIP_ERROR_INVALID_PUBLIC_KEY)
    {
        return EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_PUBLIC_KEY;
    }
    else if (err == CHIP_ERROR_INVALID_FABRIC_ID || err == CHIP_ERROR_WRONG_NODE_ID)
    {
        return EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_NODE_OP_ID;
    }
    else if (err == CHIP_ERROR_CA_CERT_NOT_FOUND || err == CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED ||
             err == CHIP_ERROR_CERT_PATH_TOO_LONG || err == CHIP_ERROR_CERT_USAGE_NOT_ALLOWED || err == CHIP_ERROR_CERT_EXPIRED ||
             err == CHIP_ERROR_CERT_NOT_VALID_YET || err == CHIP_ERROR_UNSUPPORTED_CERT_FORMAT ||
             err == CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE || err == CHIP_ERROR_CERT_LOAD_FAILED ||
             err == CHIP_ERROR_CERT_NOT_TRUSTED || err == CHIP_ERROR_WRONG_CERT_SUBJECT)
    {
        return EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_NOC;
    }
    else if (err == CHIP_ERROR_NO_MEMORY)
    {
        return EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_TABLE_FULL;
    }

    return EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_INVALID_NOC;
}

} // namespace

bool emberAfOperationalCredentialsClusterAddNOCCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::AddNOC::DecodableType & commandData)
{
    auto & NOCValue      = commandData.NOCValue;
    auto & ICACValue     = commandData.ICACValue;
    auto & adminVendorId = commandData.adminVendorId;

    EmberAfNodeOperationalCertStatus nocResponse = EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS;

    CHIP_ERROR err          = CHIP_NO_ERROR;
    FabricIndex fabricIndex = 0;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added an Op Cert");

    err = gFabricBeingCommissioned.SetNOCCert(NOCValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = gFabricBeingCommissioned.SetICACert(ICACValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    gFabricBeingCommissioned.SetVendorId(adminVendorId);

    err = Server::GetInstance().GetFabricTable().AddNewFabric(gFabricBeingCommissioned, &fabricIndex);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = Server::GetInstance().GetFabricTable().Store(fabricIndex);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // We might have a new operational identity, so we should start advertising it right away.
    app::DnssdServer::Instance().AdvertiseOperational();

exit:

    gFabricBeingCommissioned.Reset();
    SendNOCResponse(commandObj, nocResponse, fabricIndex, ByteSpan());

    if (nocResponse != EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddNOC request. Status %d", nocResponse);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterUpdateNOCCallback(app::CommandHandler * commandObj,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::UpdateNOC::DecodableType & commandData)
{
    auto & NOCValue  = commandData.NOCValue;
    auto & ICACValue = commandData.ICACValue;

    EmberAfNodeOperationalCertStatus nocResponse = EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS;

    CHIP_ERROR err          = CHIP_NO_ERROR;
    FabricIndex fabricIndex = 0;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: an administrator has updated the Op Cert");

    // Fetch current fabric
    FabricInfo * fabric = retrieveCurrentFabric();
    VerifyOrExit(fabric != nullptr, nocResponse = ConvertToNOCResponseStatus(CHIP_ERROR_INVALID_FABRIC_ID));

    err = fabric->SetNOCCert(NOCValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = fabric->SetICACert(ICACValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    fabricIndex = fabric->GetFabricIndex();

    // We have a new operational identity and should start advertising it.  We
    // can't just wait until we get network configuration commands, because we
    // might be on the operational network already, in which case we are
    // expected to be live with our new identity at this point.
    app::DnssdServer::Instance().AdvertiseOperational();

exit:

    SendNOCResponse(commandObj, nocResponse, fabricIndex, ByteSpan());

    if (nocResponse != EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed UpdateNOC request. Status %d", nocResponse);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterCertificateChainRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::CertificateChainRequest::DecodableType & commandData)
{
    auto & certificateType = commandData.certificateType;

    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has requested Device Attestation Credentials");

    uint8_t derBuf[Credentials::kMaxDERCertLength];
    MutableByteSpan derBufSpan(derBuf);

    Commands::CertificateChainResponse::Type response;

    Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    if (certificateType == kDACCertificate)
    {
        SuccessOrExit(err = dacProvider->GetDeviceAttestationCert(derBufSpan));
    }
    else if (certificateType == kPAICertificate)
    {
        err = dacProvider->GetProductAttestationIntermediateCert(derBufSpan);
        if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE) // If Node does not have a PAI Certificate
        {
            // Send an empty octet string
            derBufSpan = MutableByteSpan();
        }
        else
        {
            SuccessOrExit(err);
        }
    }
    else
    {
        SuccessOrExit(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    response.certificate = derBufSpan;
    SuccessOrExit(err = commandObj->AddResponseData(commandPath, response));

exit:
    if (err != CHIP_NO_ERROR)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed CertificateChainRequest: %s", ErrorStr(err));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterAttestationRequestCallback(app::CommandHandler * commandObj,
                                                                    const app::ConcreteCommandPath & commandPath,
                                                                    const Commands::AttestationRequest::DecodableType & commandData)
{
    auto & attestationNonce = commandData.attestationNonce;

    CHIP_ERROR err = CHIP_NO_ERROR;
    Platform::ScopedMemoryBuffer<uint8_t> attestationElements;
    size_t attestationElementsLen;
    Crypto::P256ECDSASignature signature;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has requested Attestation");

    Commands::AttestationResponse::Type response;

    Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(attestationNonce.size() == 32, err = CHIP_ERROR_INVALID_ARGUMENT);

    {
        uint8_t certDeclBuf[512];
        MutableByteSpan certDeclSpan(certDeclBuf);

        // TODO: retrieve vendor information to populate the fields below.
        uint32_t timestamp = 0;
        ByteSpan firmwareInfo;
        ByteSpan * vendorReservedArray = nullptr;
        size_t vendorReservedArraySize = 0;
        uint16_t vendorId              = 0;
        uint16_t profileNum            = 0;

        SuccessOrExit(err = dacProvider->GetCertificationDeclaration(certDeclSpan));
        // TODO: Retrieve firmware Information

        attestationElementsLen = certDeclSpan.size() + attestationNonce.size() + sizeof(uint64_t) * 8;
        VerifyOrExit(attestationElements.Alloc(attestationElementsLen), err = CHIP_ERROR_NO_MEMORY);

        MutableByteSpan attestationElementsSpan(attestationElements.Get(), attestationElementsLen);
        SuccessOrExit(err = Credentials::ConstructAttestationElements(certDeclSpan, attestationNonce, timestamp, firmwareInfo,
                                                                      vendorReservedArray, vendorReservedArraySize, vendorId,
                                                                      profileNum, attestationElementsSpan));
        attestationElementsLen = attestationElementsSpan.size();
    }

    {
        uint8_t md[Crypto::kSHA256_Hash_Length];
        MutableByteSpan messageDigestSpan(md);

        // TODO: Create an alternative way to retrieve the Attestation Challenge without this huge amount of calls.
        // Retrieve attestation challenge
        ByteSpan attestationChallenge = commandObj->GetExchangeContext()
                                            ->GetExchangeMgr()
                                            ->GetSessionManager()
                                            ->GetSecureSession(commandObj->GetExchangeContext()->GetSecureSession())
                                            ->GetCryptoContext()
                                            .GetAttestationChallenge();

        Hash_SHA256_stream hashStream;
        SuccessOrExit(err = hashStream.Begin());
        SuccessOrExit(err = hashStream.AddData(ByteSpan(attestationElements.Get(), attestationElementsLen)));
        SuccessOrExit(err = hashStream.AddData(attestationChallenge));
        SuccessOrExit(err = hashStream.Finish(messageDigestSpan));

        MutableByteSpan signatureSpan(signature, signature.Capacity());
        SuccessOrExit(err = dacProvider->SignWithDeviceAttestationKey(messageDigestSpan, signatureSpan));
        SuccessOrExit(err = signature.SetLength(signatureSpan.size()));
    }

    response.attestationElements = ByteSpan(attestationElements.Get(), attestationElementsLen);
    response.signature           = ByteSpan(signature, signature.Length());
    SuccessOrExit(err = commandObj->AddResponseData(commandPath, response));

exit:
    if (err != CHIP_NO_ERROR)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AttestationRequest: %s", ErrorStr(err));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterOpCSRRequestCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::OpCSRRequest::DecodableType & commandData)
{
    auto & CSRNonce = commandData.CSRNonce;

    CHIP_ERROR err = CHIP_NO_ERROR;

    Platform::ScopedMemoryBuffer<uint8_t> csr;
    size_t csrLength = Crypto::kMAX_CSR_Length;

    chip::Platform::ScopedMemoryBuffer<uint8_t> csrElements;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has requested an OpCSR");

    Commands::OpCSRResponse::Type response;

    TLV::TLVWriter csrElementWriter;
    TLV::TLVType containerType;

    VerifyOrExit(csr.Alloc(Crypto::kMAX_CSR_Length), err = CHIP_ERROR_NO_MEMORY);

    if (gFabricBeingCommissioned.GetOperationalKey() == nullptr)
    {
        Crypto::P256Keypair keypair;
        keypair.Initialize();
        SuccessOrExit(err = gFabricBeingCommissioned.SetEphemeralKey(&keypair));
    }

    err = gFabricBeingCommissioned.GetOperationalKey()->NewCertificateSigningRequest(csr.Get(), csrLength);
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: NewCertificateSigningRequest returned %" CHIP_ERROR_FORMAT, err.Format());
    SuccessOrExit(err);
    VerifyOrExit(csrLength < UINT8_MAX, err = CHIP_ERROR_INTERNAL);

    VerifyOrExit(csrElements.Alloc(kMaxRspLen), err = CHIP_ERROR_NO_MEMORY);
    csrElementWriter.Init(csrElements.Get(), kMaxRspLen);

    SuccessOrExit(err = csrElementWriter.StartContainer(TLV::AnonymousTag, TLV::TLVType::kTLVType_Structure, containerType));
    SuccessOrExit(err = csrElementWriter.Put(TLV::ContextTag(1), ByteSpan(csr.Get(), csrLength)));
    SuccessOrExit(err = csrElementWriter.Put(TLV::ContextTag(2), CSRNonce));
    SuccessOrExit(err = csrElementWriter.Put(TLV::ContextTag(3), ByteSpan()));
    SuccessOrExit(err = csrElementWriter.Put(TLV::ContextTag(4), ByteSpan()));
    SuccessOrExit(err = csrElementWriter.Put(TLV::ContextTag(5), ByteSpan()));
    SuccessOrExit(err = csrElementWriter.EndContainer(containerType));
    SuccessOrExit(err = csrElementWriter.Finalize());

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Write CSR Elements
    response.NOCSRElements = ByteSpan(csrElements.Get(), csrElementWriter.GetLengthWritten());
    // TODO - Write attestation signature using attestation key
    response.attestationSignature = ByteSpan();
    SuccessOrExit(err = commandObj->AddResponseData(commandPath, response));

exit:
    if (err != CHIP_NO_ERROR)
    {
        gFabricBeingCommissioned.Reset();
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed OpCSRRequest: %s", ErrorStr(err));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterAddTrustedRootCertificateCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::AddTrustedRootCertificate::DecodableType & commandData)
{
    auto & RootCertificate = commandData.rootCertificate;

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added a trusted root Cert");

    VerifyOrExit(gFabricBeingCommissioned.SetRootCert(RootCertificate) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    emberAfSendImmediateDefaultResponse(status);
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        gFabricBeingCommissioned.Reset();
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddTrustedRootCert request.");
    }

    return true;
}

bool emberAfOperationalCredentialsClusterRemoveTrustedRootCertificateCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::RemoveTrustedRootCertificate::DecodableType & commandData)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
