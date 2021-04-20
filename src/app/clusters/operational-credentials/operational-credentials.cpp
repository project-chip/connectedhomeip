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

#include "gen/af-structs.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Transport;

EmberAfStatus writeFabricAttribute(uint8_t endpoint, AttributeId attributeId, uint8_t * buffer, int32_t index = -1)
{
    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = ZCL_OPERATIONAL_CREDENTIALS_CLUSTER_ID;
    record.clusterMask      = CLUSTER_MASK_SERVER;
    record.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
    record.attributeId      = attributeId;

    // When reading or writing a List attribute the 'index' value could have 3 types of values:
    //  -1: Read/Write the whole list content, including the number of elements in the list
    //   0: Read/Write the number of elements in the list, represented as a uint16_t
    //   n: Read/Write the nth element of the list
    //
    // Since the first 2 bytes of the attribute are used to store the number of elements, elements indexing starts
    // at 1. In order to hide this to the rest of the code of this file, the element index is incremented by 1 here.
    // This also allows calling writeAttribute() with no index arg to mean "write the length".
    
    return emAfReadOrWriteAttribute(&record, NULL, buffer, 0, true, index + 1);
}

EmberAfStatus writeFabric(FabricId fabricId, NodeId nodeId, uint16_t vendorId, int32_t index)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_FABRICS_ATTRIBUTE_ID;

    EmberAfFabricDescriptor fabricDescriptor;
    fabricDescriptor.FabricId = fabricId;
    fabricDescriptor.NodeId   = nodeId;
    fabricDescriptor.VendorId = vendorId;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Writing admin into attribute store at index %d: fabricId %" PRIX64 
                            ", nodeId %" PRIX64 " vendorId %" PRIX16, index, fabricId, nodeId, vendorId);
    status =  writeFabricAttribute(0, attributeId, (uint8_t *) &fabricDescriptor, index);
    return status;
}

CHIP_ERROR writeAdminsIntoFabricsListAttribute(void)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Call to writeAdminsIntoFabricsListAttribute");

    // Loop through admins
    auto pairing = GetGlobalAdminPairingTable().cbegin();
    int32_t fabricIndex = 0;
    while (pairing != GetGlobalAdminPairingTable().cend())
    { 
        NodeId nodeId = pairing->GetNodeId();
        uint64_t fabricId = pairing->GetFabricId();
        uint16_t vendorId = pairing->GetVendorId();

        // Skip over uninitialized admins
        if (nodeId != kUndefinedNodeId && fabricId != kUndefinedFabricId && vendorId != kUndefinedVendorId)
        {
            if (writeFabric(fabricId, nodeId, vendorId, fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write admin with fabricId %" PRIX64 " in fabrics list", fabricId);
            }
            fabricIndex ++;
        }
        pairing++;
    }

    // Store the count of fabrics we just stored
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Stored %" PRIX32 " admins in fabrics list attribute.", fabricIndex);
    if (writeFabricAttribute(0, ZCL_FABRICS_ATTRIBUTE_ID, (uint8_t *) &fabricIndex) != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed to write admin count %" PRIX32 " in fabrics list", fabricIndex);
    }

    return CHIP_NO_ERROR;
}

AdminPairingInfo * retrieveCurrentAdmin()
{
    uint64_t fabricId = emberAfCurrentCommand()->source;
    uint64_t nodeId;
    CHIP_ERROR err = ConfigurationMgr().GetDeviceId(nodeId);
    if (err == CHIP_NO_ERROR)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Finding admin with fabricId  %" PRIX64 ".", fabricId);
        return GetGlobalAdminPairingTable().FindAdmin(fabricId, nodeId);
    }
    return nullptr;
}

class OpCredsAdminPairingTableDelegate : public AdminPairingTableDelegate
{

    void OnAdminDeletedFromStorage(AdminId adminId) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Admin %" PRIX16 " was deleted from admin storage.");
        writeAdminsIntoFabricsListAttribute();
    }
    void OnAdminRetrievedFromStorage(AdminId adminId, FabricId fabricId, NodeId nodeId) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Admin %" PRIX16 " was retrieved from storage. FabricId %" PRIX64
                                            ", NodeId %" PRIX64 ", VendorId %" PRIX64, adminId, fabricId, nodeId);
        writeAdminsIntoFabricsListAttribute();
    }
    void OnAdminPersistedToStorage(AdminId adminId, FabricId fabricId, NodeId nodeId) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Admin %" PRIX16 " was persisted to storage. FabricId %" PRIX64
                                            ", NodeId %" PRIX64 ", VendorId %" PRIX64, adminId, fabricId, nodeId);
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

bool emberAfOperationalCredentialsClusterRemoveFabricCallback(chip::app::Command * commandObj, chip::FabricId fabricId,
                                                              chip::NodeId nodeId, uint16_t vendorId)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: RemoveFabric"); // TODO: Generate emberAfFabricClusterPrintln
    
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    AdminPairingInfo * admin;
    AdminId adminId;
    CHIP_ERROR err;

    // Fetch current admin
    admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_SUCCESS); // Admin has already been removed

    // Delete admin
    adminId = admin->GetAdminId();
    GetGlobalAdminPairingTable().ReleaseAdminId(adminId);
    err = AdminPairingInfo::DeleteFromKVS(adminId);
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

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
    err = admin->StoreIntoKVS();
    VerifyOrExit(err == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    // Return FabricId
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
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: UpdateFabricLabel to %s", (char*) Label);
    
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}