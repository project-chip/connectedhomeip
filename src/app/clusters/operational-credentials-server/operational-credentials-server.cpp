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

#include <app/CommandHandler.h>
#include <app/common/gen/af-structs.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/server/Mdns.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/PeerId.h>
#include <platform/CHIPDeviceLayer.h>
#include <string.h>
#include <support/CodeUtils.h>
#include <support/ScopedBuffer.h>
#include <support/logging/CHIPLogging.h>
#include <transport/FabricTable.h>

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Transport;

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

EmberAfStatus writeFabric(FabricId fabricId, NodeId nodeId, uint16_t vendorId, const uint8_t * fabricLabel, int32_t index)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    EmberAfFabricDescriptor fabricDescriptor;
    fabricDescriptor.FabricId = fabricId;
    fabricDescriptor.NodeId   = nodeId;
    fabricDescriptor.VendorId = vendorId;
    if (fabricLabel != nullptr)
    {
        size_t lengthToStore   = strnlen(Uint8::to_const_char(fabricLabel), kFabricLabelMaxLengthInBytes);
        fabricDescriptor.Label = ByteSpan(fabricLabel, lengthToStore);
    }

    emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                   "OpCreds: Writing fabric into attribute store at index %d: fabricId 0x" ChipLogFormatX64
                   ", nodeId 0x" ChipLogFormatX64 " vendorId 0x%04" PRIX16,
                   index, ChipLogValueX64(fabricId), ChipLogValueX64(nodeId), vendorId);
    status = writeFabricAttribute((uint8_t *) &fabricDescriptor, index);
    return status;
}

CHIP_ERROR writeFabricsIntoFabricsListAttribute()
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Call to writeFabricsIntoFabricsListAttribute");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Loop through fabrics
    int32_t fabricIndex = 0;
    for (auto & pairing : GetGlobalFabricTable())
    {
        NodeId nodeId               = pairing.GetNodeId();
        uint64_t fabricId           = pairing.GetFabricId();
        uint16_t vendorId           = pairing.GetVendorId();
        const uint8_t * fabricLabel = pairing.GetFabricLabel();

        // Skip over uninitialized fabrics
        if (nodeId == kUndefinedNodeId || fabricId == kUndefinedFabricId || vendorId == kUndefinedVendorId)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                           "OpCreds: Skipping over unitialized fabric with fabricId 0x" ChipLogFormatX64
                           ", nodeId 0x" ChipLogFormatX64 " vendorId 0x%04" PRIX16,
                           ChipLogValueX64(fabricId), ChipLogValueX64(nodeId), vendorId);
            continue;
        }
        else if (writeFabric(fabricId, nodeId, vendorId, fabricLabel, fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
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
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Stored %" PRIu32 " fabrics in fabrics list attribute.", fabricIndex);
    if (writeFabricAttribute((uint8_t *) &fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write fabric count %" PRIu32 " in fabrics list", fabricIndex);
        err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    return err;
}

/*
 * Look at "Temporary flow for fabric management" comment above for current fabric management flow.
 * To retrieve the current fabric, we retrieve the emberAfCurrentCommand()->source which should be set
 * to the commissioner node Id, which we are temporarily using as the fabricId.
 * We should also figure out how to retrieve the device nodeId and vendorId if we can so that we use multiple
 * fields to find the current fabric. Once addOptCert and fabric index are implemented, remove all this and use fabricIndex.
 */

static FabricInfo * retrieveCurrentFabric()
{
    uint64_t fabricId = emberAfCurrentCommand()->SourceNodeId();
    // TODO: Figure out how to get device node id so we can do FindFabricForNode(fabricId, nodeId)...
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Finding fabric with fabricId 0x" ChipLogFormatX64 ".",
                   ChipLogValueX64(fabricId));
    return GetGlobalFabricTable().FindFabricForNode(fabricId);
}

// TODO: The code currently has two sources of truths for fabrics, the pairing table + the attributes. There should only be one,
// the attributes list. Currently the attributes are not persisted so we are keeping the fabric table to have the
// fabrics/admrins be persisted. Once attributes are persisted, there should only be one sorce of truth, the attributes list and
// only that should be modifed to perosst/read/write fabrics.
// TODO: Once attributes are persisted, implement reading/writing/manipulation fabrics around that and remove fabricPairingTable
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
                       fabric->GetFabricIndex(), ChipLogValueX64(fabric->GetFabricId()), ChipLogValueX64(fabric->GetNodeId()),
                       fabric->GetVendorId());
        writeFabricsIntoFabricsListAttribute();
    }

    // Gets called when a fabric in FabricTable is persisted to KVS store.
    void OnFabricPersistedToStorage(FabricInfo * fabric) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "OpCreds: Fabric %" PRIX16 " was persisted to storage. FabricId %0x" ChipLogFormatX64
                       ", NodeId %0x" ChipLogFormatX64 ", VendorId 0x%04" PRIX16,
                       fabric->GetFabricIndex(), ChipLogValueX64(fabric->GetFabricId()), ChipLogValueX64(fabric->GetNodeId()),
                       fabric->GetVendorId());
        writeFabricsIntoFabricsListAttribute();
    }
};

OpCredsFabricTableDelegate gFabricDelegate;

void emberAfPluginOperationalCredentialsServerInitCallback(void)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Initiating OpCreds cluster by writing fabrics list from fabric table.");
    GetGlobalFabricTable().SetFabricDelegate(&gFabricDelegate);
    writeFabricsIntoFabricsListAttribute();
}

// TODO: Use FabricIndex as a parameter instead of fabricId/nodeId/vendorId once AddOptCert + FabricIndex are implemented
bool emberAfOperationalCredentialsClusterRemoveFabricCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                              chip::FabricId fabricId, chip::NodeId nodeId, uint16_t vendorId)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: RemoveFabric"); // TODO: Generate emberAfFabricClusterPrintln

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    FabricInfo * fabric;
    FabricIndex fabricIndex;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Fetch matching fabric
    fabric = GetGlobalFabricTable().FindFabricForNode(fabricId, nodeId, vendorId);
    VerifyOrExit(fabric != nullptr, status = EMBER_ZCL_STATUS_SUCCESS); // Fabric has already been removed

    // Delete fabric
    fabricIndex = fabric->GetFabricIndex();
    err         = GetGlobalFabricTable().Delete(fabricIndex);
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    writeFabricsIntoFabricsListAttribute();
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// TODO: remove SetFabric once AddOptCert + FabricIndex are implemented
bool emberAfOperationalCredentialsClusterSetFabricCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                           uint16_t VendorId)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: SetFabric with vendorId %" PRIX16, VendorId);

    EmberAfStatus status   = EMBER_ZCL_STATUS_SUCCESS;
    EmberStatus sendStatus = EMBER_SUCCESS;
    CHIP_ERROR err         = CHIP_NO_ERROR;

    // Fetch current fabric
    FabricInfo * fabric = retrieveCurrentFabric();
    VerifyOrExit(fabric != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    // Store vendorId
    fabric->SetVendorId(VendorId);
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: vendorId is now set %" PRIX16, fabric->GetVendorId());
    err = GetGlobalFabricTable().Store(fabric->GetFabricIndex());
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    // Return FabricId - we are temporarily using commissioner nodeId (retrieved via emberAfCurrentCommand()->SourceNodeId()) as
    // fabricId until addOptCert + fabricIndex are implemented. Once they are, this method and its response will go away.
    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID,
                                             ZCL_SET_FABRIC_RESPONSE_COMMAND_ID, (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;

        VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        writer = commandObj->GetCommandDataElementTLVWriter();
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), commandObj->GetExchangeContext()->GetSecureSession().GetPeerNodeId()));
        SuccessOrExit(err = commandObj->FinishCommand());
    }

exit:
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed setFabricVendorId.");
        emberAfSendImmediateDefaultResponse(status);
    }
    if (sendStatus != EMBER_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to send SetFabric response: 0x%x", sendStatus);
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command: %s", ErrorStr(err));
    }

    return true;
}

bool emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(chip::EndpointId endpoint,
                                                                   chip::app::CommandHandler * commandObj, uint8_t * Label)
{
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
    err = GetGlobalFabricTable().Store(fabric->GetFabricIndex());
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    writeFabricsIntoFabricsListAttribute();
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

namespace {
void DoRemoveAllFabrics(intptr_t)
{
    OpenDefaultPairingWindow(ResetFabrics::kYes);
}

CHIP_ERROR SendNOCResponse(chip::app::Command * commandObj, EmberAfNodeOperationalCertStatus status, uint8_t index,
                           ByteSpan debug_text)
{
    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID,
                                         ZCL_NOC_RESPONSE_COMMAND_ID, (chip::app::CommandPathFlags::kEndpointIdValid) };
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

// Up for discussion in Multi-Admin TT: chip-spec:#2891
bool emberAfOperationalCredentialsClusterRemoveAllFabricsCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Remove all Fabrics");
    PlatformMgr().ScheduleWork(DoRemoveAllFabrics, 0);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfOperationalCredentialsClusterAddNOCCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                        chip::ByteSpan NOCArray, chip::ByteSpan IPKValue,
                                                        chip::NodeId CaseAdminNode, uint16_t AdminVendorId)
{
    EmberAfNodeOperationalCertStatus nocResponse = EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS;

    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t fabricIndex = 0;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added an Op Cert");

    FabricInfo * fabric = retrieveCurrentFabric();
    VerifyOrExit(fabric != nullptr, nocResponse = EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_MISSING_CSR);

    err = fabric->SetOperationalCertsFromCertArray(NOCArray);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = GetGlobalFabricTable().Store(fabric->GetFabricIndex());
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // We have a new operational identity and should start advertising it.  We
    // can't just wait until we get network configuration commands, because we
    // might be on the operational network already, in which case we are
    // expected to be live with our new identity at this point.
    chip::app::Mdns::AdvertiseOperational();

    fabricIndex = GetGlobalFabricTable().GetFabricIndex(fabric);

exit:

    SendNOCResponse(commandObj, nocResponse, fabricIndex, ByteSpan());

    if (nocResponse != EMBER_ZCL_NODE_OPERATIONAL_CERT_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddNOC request. Status %d", nocResponse);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterOpCSRRequestCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                              chip::ByteSpan CSRNonce)
{
    EmberAfStatus status   = EMBER_ZCL_STATUS_SUCCESS;
    EmberStatus sendStatus = EMBER_SUCCESS;
    CHIP_ERROR err         = CHIP_NO_ERROR;

    chip::Platform::ScopedMemoryBuffer<uint8_t> csr;
    size_t csrLength = Crypto::kMAX_CSR_Length;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has requested an OpCSR");

    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID,
                                         ZCL_OP_CSR_RESPONSE_COMMAND_ID, (chip::app::CommandPathFlags::kEndpointIdValid) };

    TLV::TLVWriter * writer = nullptr;

    // Fetch current fabric
    FabricInfo * fabric = retrieveCurrentFabric();
    VerifyOrExit(fabric != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    VerifyOrExit(csr.Alloc(Crypto::kMAX_CSR_Length), status = EMBER_ZCL_STATUS_FAILURE);

    if (fabric->GetOperationalKey() == nullptr)
    {
        Crypto::P256Keypair keypair;
        keypair.Initialize();
        VerifyOrExit(fabric->SetOperationalKey(keypair) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);
    }

    err = fabric->GetOperationalKey()->NewCertificateSigningRequest(csr.Get(), csrLength);
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: NewCertificateSigningRequest returned %d", err);
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(csrLength < UINT8_MAX, status = EMBER_ZCL_STATUS_FAILURE);

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
    writer = commandObj->GetCommandDataElementTLVWriter();
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), ByteSpan(csr.Get(), csrLength)));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), CSRNonce));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(2), ByteSpan(nullptr, 0)));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(3), ByteSpan(nullptr, 0)));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(4), ByteSpan(nullptr, 0)));
    SuccessOrExit(err = writer->Put(TLV::ContextTag(5), ByteSpan(nullptr, 0)));
    SuccessOrExit(err = commandObj->FinishCommand());

exit:
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed OpCSRRequest.");
        emberAfSendImmediateDefaultResponse(status);
    }
    if (sendStatus != EMBER_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to send OpCSRRequest: 0x%x", sendStatus);
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command: %s", ErrorStr(err));
    }

    return true;
}

bool emberAfOperationalCredentialsClusterUpdateOpCertCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                              chip::ByteSpan NOC, chip::ByteSpan ICACertificate)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOperationalCredentialsClusterAddTrustedRootCertificateCallback(chip::EndpointId endpoint,
                                                                           chip::app::CommandHandler * commandObj,
                                                                           chip::ByteSpan RootCertificate)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added a trusted root Cert");

    // Fetch current fabric
    FabricInfo * fabric = retrieveCurrentFabric();
    VerifyOrExit(fabric != nullptr, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(fabric->SetRootCert(RootCertificate) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    VerifyOrExit(GetGlobalFabricTable().Store(fabric->GetFabricIndex()) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    emberAfSendImmediateDefaultResponse(status);
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddTrustedRootCert request.");
    }

    return true;
}

bool emberAfOperationalCredentialsClusterRemoveTrustedRootCertificateCallback(chip::EndpointId endpoint,
                                                                              chip::app::CommandHandler * commandObj,
                                                                              chip::ByteSpan TrustedRootIdentifier)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
