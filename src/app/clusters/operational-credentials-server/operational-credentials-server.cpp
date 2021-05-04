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

#include <app/Command.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/AdminPairingTable.h>
#include <app/server/Server.h>
#include <lib/core/PeerId.h>

#include "gen/af-structs.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Transport;

/*
* Temporary flow for fabric management until addOptCert + fabric index are implemented:
* 1) When Commissioner pairs with CHIP device, store device nodeId in Admin Pairing table as NodeId
*    and store commissioner nodeId in Admin Pairing table as FabricId (This is temporary until AddOptCert is implemented and Fabrics are implemented correctely)
* 2) When pairing is complete, commissioner calls SetFabric to set the vendorId on the newly created fabric. The corresponding fabric is found by looking
*    in admin pairing table and finding a fabric that has the matching commissioner node ID as fabricId + device nodeId as nodeId and an uninitialized vendorId.
* 3) RemoveFabric uses the passed in fabricId, nodeId, vendorID to find matching entry and remove it from admin pairing table. Once fabricIndex is implemented, it should use that instead.
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
                                    0, // read length
                                    true, // write ?
                                    index + 1);
}

EmberAfStatus writeFabric(FabricId fabricId, NodeId nodeId, uint16_t vendorId, int32_t index)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;

    EmberAfFabricDescriptor fabricDescriptor;
    fabricDescriptor.FabricId = fabricId;
    fabricDescriptor.NodeId   = nodeId;
    fabricDescriptor.VendorId = vendorId;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Writing admin into attribute store at index %d: fabricId %" PRIX64
                            ", nodeId %" PRIX64 " vendorId %" PRIX16, index, fabricId, nodeId, vendorId);
    status =  writeFabricAttribute((uint8_t *) &fabricDescriptor, index);
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
        NodeId nodeId = pairing.GetNodeId();
        uint64_t fabricId = pairing.GetFabricId();
        uint16_t vendorId = pairing.GetVendorId();

        // Skip over uninitialized admins
        if (nodeId == kUndefinedNodeId || fabricId == kUndefinedFabricId || vendorId == kUndefinedVendorId)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Skipping over unitialized admin with fabricId %" PRIX64
                            ", nodeId %" PRIX64 " vendorId %" PRIX16, fabricId, nodeId, vendorId);
            continue;
        } else if (writeFabric(fabricId, nodeId, vendorId, fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
        {
            emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write admin with fabricId %" PRIX64 " in fabrics list", fabricId);
            err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
            break;
        }
        fabricIndex ++;
    }

    // Store the count of fabrics we just stored
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Stored %" PRIX32 " admins in fabrics list attribute.", fabricIndex);
    if (writeFabricAttribute((uint8_t *) &fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write admin count %" PRIX32 " in fabrics list", fabricIndex);
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

AdminPairingInfo * retrieveCurrentAdmin()
{
    uint64_t fabricId = emberAfCurrentCommand()->source;
    // TODO: Figure out how to get device node id so we can do FindAdminForNode(fabricId, nodeId)...
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Finding admin with fabricId  %" PRIX64 ".", fabricId);
    return GetGlobalAdminPairingTable().FindAdminForNode(fabricId);
}


// TODO: The code currently has two sources of truths for admins, the pairing table + the attributes. There should only be one,
// the attributes list. Currently the attributes are not persisted so we are keeping the admin pairing table to have the fabrics/admrins
// be persisted. Once attributes are persisted, there should only be one sorce of truth, the attributes list and only that should be
// modifed to perosst/read/write fabrics.
// TODO: Once attributes are persisted, implement reading/writing/manipulation fabrics around that and remove adminPairingTable logic.
class OpCredsAdminPairingTableDelegate : public AdminPairingTableDelegate
{

    // Gets called when a fabric is deleted from KVS store
    void OnAdminDeletedFromStorage(AdminId adminId) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Admin %" PRIX16 " was deleted from admin storage.", adminId);
        writeAdminsIntoFabricsListAttribute();
    }

    // Gets called when a fabric is loaded into the AdminPairingTable from KVS store.
    void OnAdminRetrievedFromStorage(AdminPairingInfo * admin) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Admin %" PRIX16 " was retrieved from storage. FabricId %" PRIX64
                                            ", NodeId %" PRIX64 ", VendorId %" PRIX64, admin->GetAdminId(), admin->GetFabricId(), admin->GetNodeId());
        writeAdminsIntoFabricsListAttribute();
    }

     // Gets called when a fabric in AdminPairingTable is persisted to KVS store.
    void OnAdminPersistedToStorage(AdminPairingInfo * admin) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Admin %" PRIX16 " was persisted to storage. FabricId %" PRIX64
                                            ", NodeId %" PRIX64 ", VendorId %" PRIX64, admin->GetAdminId(), admin->GetFabricId(), admin->GetNodeId());
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
bool emberAfOperationalCredentialsClusterRemoveFabricCallback(chip::app::Command * commandObj, chip::FabricId fabricId,
                                                              chip::NodeId nodeId, uint16_t vendorId)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: RemoveFabric"); // TODO: Generate emberAfFabricClusterPrintln

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    AdminPairingInfo * admin;
    AdminId adminId;
    CHIP_ERROR err;

    // Fetch matching admin
    admin = GetGlobalAdminPairingTable().FindAdminForNode(fabricId, nodeId, vendorId);
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_SUCCESS); // Admin has already been removed

    // Delete admin
    adminId = admin->GetAdminId();
    err = GetGlobalAdminPairingTable().Delete(admin->GetAdminId());
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    writeAdminsIntoFabricsListAttribute();
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// TODO: remove SetFabric once AddOptCert + FabricIndex are implemented
bool emberAfOperationalCredentialsClusterSetFabricCallback(chip::app::Command * commandObj, uint16_t VendorId)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: SetFabric with vendorId %d", VendorId);

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    EmberStatus sendStatus = EMBER_SUCCESS;
    CHIP_ERROR err;

    // Fetch current admin
    AdminPairingInfo * admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_FAILURE);

    // Store vendorId
    admin->SetVendorId(VendorId);
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: vendorId is now set %d", admin->GetVendorId());
    err = GetGlobalAdminPairingTable().Store(admin->GetAdminId());
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    // Return FabricId - we are temporarily using commissioner nodeId (retrieved via emberAfCurrentCommand()->source) as fabricId
    // until addOptCert + fabricIndex are implemented. Once they are, this method and its response will go away.
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID,
                              ZCL_SET_FABRIC_RESPONSE_COMMAND_ID, "y", emberAfCurrentCommand()->source);
    sendStatus = emberAfSendResponse();

exit:
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed setFabricVendorId.");
        emberAfSendImmediateDefaultResponse(status);
    }
    if (sendStatus != EMBER_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to send %s response: 0x%x", "set_fabric", sendStatus);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(chip::app::Command * commandObj, uint8_t * Label)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: UpdateFabricLabel");

    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
