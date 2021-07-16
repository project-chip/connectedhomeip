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
#include <transport/AdminPairingTable.h>

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Transport;

/*
 * Temporary flow for fabric management until addOptCert + fabric index are implemented:
 * 1) When Commissioner pairs with CHIP device, store device nodeId in Admin Pairing table as NodeId
 *    and store commissioner nodeId in Admin Pairing table as FabricId (This is temporary until AddOptCert is implemented and
 * Fabrics are implemented correctely) 2) When pairing is complete, commissioner calls SetFabric to set the vendorId on the newly
 * created fabric. The corresponding fabric is found by looking in admin pairing table and finding a fabric that has the matching
 * commissioner node ID as fabricId + device nodeId as nodeId and an uninitialized vendorId. 3) RemoveFabric uses the passed in
 * fabricId, nodeId, vendorID to find matching entry and remove it from admin pairing table. Once fabricIndex is implemented, it
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
                   "OpCreds: Writing admin into attribute store at index %d: fabricId 0x" ChipLogFormatX64
                   ", nodeId 0x" ChipLogFormatX64 " vendorId 0x%04" PRIX16,
                   index, ChipLogValueX64(fabricId), ChipLogValueX64(nodeId), vendorId);
    status = writeFabricAttribute((uint8_t *) &fabricDescriptor, index);
    return status;
}

CHIP_ERROR writeAdminsIntoFabricsListAttribute()
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Call to writeAdminsIntoFabricsListAttribute");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Loop through admins
    int32_t fabricIndex = 0;
    for (auto & pairing : GetGlobalAdminPairingTable())
    {
        NodeId nodeId               = pairing.GetNodeId();
        uint64_t fabricId           = pairing.GetFabricId();
        uint16_t vendorId           = pairing.GetVendorId();
        const uint8_t * fabricLabel = pairing.GetFabricLabel();

        // Skip over uninitialized admins
        if (nodeId == kUndefinedNodeId || fabricId == kUndefinedFabricId || vendorId == kUndefinedVendorId)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                           "OpCreds: Skipping over unitialized admin with fabricId 0x" ChipLogFormatX64
                           ", nodeId 0x" ChipLogFormatX64 " vendorId 0x%04" PRIX16,
                           ChipLogValueX64(fabricId), ChipLogValueX64(nodeId), vendorId);
            continue;
        }
        else if (writeFabric(fabricId, nodeId, vendorId, fabricLabel, fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                           "OpCreds: Failed to write admin with fabricId 0x" ChipLogFormatX64 " in fabrics list",
                           ChipLogValueX64(fabricId));
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
            break;
        }
        fabricIndex++;
    }

    // Store the count of fabrics we just stored
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Stored %" PRIu32 " admins in fabrics list attribute.", fabricIndex);
    if (writeFabricAttribute((uint8_t *) &fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write admin count %" PRIu32 " in fabrics list", fabricIndex);
        err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    return err;
}

/*
 * Look at "Temporary flow for fabric management" comment above for current fabric management flow.
 * To retrieve the current admin, we retrieve the emberAfCurrentCommand()->source which should be set
 * to the commissioner node Id, which we are temporarily using as the fabricId.
 * We should also figure out how to retrieve the device nodeId and vendorId if we can so that we use multiple
 * fields to find the current admin. Once addOptCert and fabric index are implemented, remove all this and use fabricIndex.
 */

static AdminPairingInfo * retrieveCurrentAdmin()
{
    uint64_t fabricId = emberAfCurrentCommand()->SourceNodeId();
    // TODO: Figure out how to get device node id so we can do FindAdminForNode(fabricId, nodeId)...
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Finding admin with fabricId 0x" ChipLogFormatX64 ".", ChipLogValueX64(fabricId));
    return GetGlobalAdminPairingTable().FindAdminForNode(fabricId);
}

// TODO: The code currently has two sources of truths for admins, the pairing table + the attributes. There should only be one,
// the attributes list. Currently the attributes are not persisted so we are keeping the admin pairing table to have the
// fabrics/admrins be persisted. Once attributes are persisted, there should only be one sorce of truth, the attributes list and
// only that should be modifed to perosst/read/write fabrics.
// TODO: Once attributes are persisted, implement reading/writing/manipulation fabrics around that and remove adminPairingTable
// logic.
class OpCredsAdminPairingTableDelegate : public AdminPairingTableDelegate
{

    // Gets called when a fabric is deleted from KVS store
    void OnAdminDeletedFromStorage(AdminId adminId) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Admin 0x%" PRIX16 " was deleted from admin storage.", adminId);
        writeAdminsIntoFabricsListAttribute();
    }

    // Gets called when a fabric is loaded into the AdminPairingTable from KVS store.
    void OnAdminRetrievedFromStorage(AdminPairingInfo * admin) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "OpCreds: Admin 0x%" PRIX16 " was retrieved from storage. FabricId 0x" ChipLogFormatX64
                       ", NodeId 0x" ChipLogFormatX64 ", VendorId 0x%04" PRIX16,
                       admin->GetAdminId(), ChipLogValueX64(admin->GetFabricId()), ChipLogValueX64(admin->GetNodeId()),
                       admin->GetVendorId());
        writeAdminsIntoFabricsListAttribute();
    }

    // Gets called when a fabric in AdminPairingTable is persisted to KVS store.
    void OnAdminPersistedToStorage(AdminPairingInfo * admin) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "OpCreds: Admin %" PRIX16 " was persisted to storage. FabricId %0x" ChipLogFormatX64
                       ", NodeId %0x" ChipLogFormatX64 ", VendorId 0x%04" PRIX16,
                       admin->GetAdminId(), ChipLogValueX64(admin->GetFabricId()), ChipLogValueX64(admin->GetNodeId()),
                       admin->GetVendorId());
        writeAdminsIntoFabricsListAttribute();
    }
};

OpCredsAdminPairingTableDelegate gAdminDelegate;

void emberAfPluginOperationalCredentialsServerInitCallback(void)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Initiating OpCreds cluster by writing fabrics list from admin pairing table.");
    GetGlobalAdminPairingTable().SetAdminPairingDelegate(&gAdminDelegate);
    writeAdminsIntoFabricsListAttribute();
}

// TODO: Use FabricIndex as a parameter instead of fabricId/nodeId/vendorId once AddOptCert + FabricIndex are implemented
bool emberAfOperationalCredentialsClusterRemoveFabricCallback(chip::app::CommandHandler * commandObj, chip::FabricId fabricId,
                                                              chip::NodeId nodeId, uint16_t vendorId)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: RemoveFabric"); // TODO: Generate emberAfFabricClusterPrintln

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    AdminPairingInfo * admin;
    AdminId adminId;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Fetch matching admin
    admin = GetGlobalAdminPairingTable().FindAdminForNode(fabricId, nodeId, vendorId);
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_SUCCESS); // Admin has already been removed

    // Delete admin
    adminId = admin->GetAdminId();
    err     = GetGlobalAdminPairingTable().Delete(adminId);
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    writeAdminsIntoFabricsListAttribute();
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// TODO: remove SetFabric once AddOptCert + FabricIndex are implemented
bool emberAfOperationalCredentialsClusterSetFabricCallback(chip::app::CommandHandler * commandObj, uint16_t VendorId)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: SetFabric with vendorId %" PRIX16, VendorId);

    EmberAfStatus status   = EMBER_ZCL_STATUS_SUCCESS;
    EmberStatus sendStatus = EMBER_SUCCESS;
    CHIP_ERROR err         = CHIP_NO_ERROR;

    // Fetch current admin
    AdminPairingInfo * admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    // Store vendorId
    admin->SetVendorId(VendorId);
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: vendorId is now set %" PRIX16, admin->GetVendorId());
    err = GetGlobalAdminPairingTable().Store(admin->GetAdminId());
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

bool emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(chip::app::CommandHandler * commandObj, uint8_t * Label)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: UpdateFabricLabel");

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    CHIP_ERROR err;

    // Fetch current fabric
    AdminPairingInfo * admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    // Set Label on fabric
    err = admin->SetFabricLabel(Label);
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    // Persist updated fabric
    err = GetGlobalAdminPairingTable().Store(admin->GetAdminId());
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    writeAdminsIntoFabricsListAttribute();
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

namespace {
void DoRemoveAllFabrics(intptr_t)
{
    OpenDefaultPairingWindow(ResetAdmins::kYes);
}
} // namespace

// Up for discussion in Multi-Admin TT: chip-spec:#2891
bool emberAfOperationalCredentialsClusterRemoveAllFabricsCallback(chip::app::CommandHandler * commandObj)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Remove all Fabrics");
    PlatformMgr().ScheduleWork(DoRemoveAllFabrics, 0);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfOperationalCredentialsClusterAddOpCertCallback(chip::app::CommandHandler * commandObj, chip::ByteSpan NOCArray,
                                                           chip::ByteSpan IPKValue, chip::NodeId CaseAdminNode,
                                                           uint16_t AdminVendorId)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added an Op Cert");

    AdminPairingInfo * admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    VerifyOrExit(admin->SetOperationalCertsFromCertArray(NOCArray) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(GetGlobalAdminPairingTable().Store(admin->GetAdminId()) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    // We have a new operational identity and should start advertising it.  We
    // can't just wait until we get network configuration commands, because we
    // might be on the operational network already, in which case we are
    // expected to be live with our new identity at this point.
    chip::app::Mdns::AdvertiseOperational();

exit:
    emberAfSendImmediateDefaultResponse(status);
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddOpCert request.");
    }

    return true;
}

bool emberAfOperationalCredentialsClusterOpCSRRequestCallback(chip::app::CommandHandler * commandObj, chip::ByteSpan CSRNonce)
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

    // Fetch current admin
    AdminPairingInfo * admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    VerifyOrExit(csr.Alloc(Crypto::kMAX_CSR_Length), status = EMBER_ZCL_STATUS_FAILURE);

    if (admin->GetOperationalKey() == nullptr)
    {
        Crypto::P256Keypair keypair;
        keypair.Initialize();
        VerifyOrExit(admin->SetOperationalKey(keypair) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);
    }

    err = admin->GetOperationalKey()->NewCertificateSigningRequest(csr.Get(), csrLength);
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

bool emberAfOperationalCredentialsClusterUpdateOpCertCallback(chip::app::CommandHandler * commandObj, chip::ByteSpan NOC,
                                                              chip::ByteSpan ICACertificate)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfOperationalCredentialsClusterAddTrustedRootCertificateCallback(chip::app::CommandHandler * commandObj,
                                                                           chip::ByteSpan RootCertificate)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added a trusted root Cert");

    // Fetch current admin
    AdminPairingInfo * admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(admin->SetRootCert(RootCertificate) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    VerifyOrExit(GetGlobalAdminPairingTable().Store(admin->GetAdminId()) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    emberAfSendImmediateDefaultResponse(status);
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddTrustedRootCert request.");
    }

    return true;
}

bool emberAfOperationalCredentialsClusterRemoveTrustedRootCertificateCallback(chip::app::CommandHandler * commandObj,
                                                                              chip::ByteSpan TrustedRootIdentifier)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
