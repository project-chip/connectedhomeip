/*******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "../../mpc_attribute_resolver/src/mpc_attribute_resolver_cluster_rules.h"
#include "attribute_store_type_registration.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "unify_dotdot_defined_attribute_types.h"
#include <vector>
// defines
#define UNSIGNED_LONG_STORAGE_TYPE ((sizeof(unsigned long) == sizeof(uint32_t)) ? U32_STORAGE_TYPE : U64_STORAGE_TYPE)

typedef struct
{
    attribute_store_type_t type;
    const char * name;
    attribute_store_type_t parent_type;
    attribute_store_storage_type_t storage_type;
} attribute_schema_t;

static const std::vector<attribute_schema_t> attribute_schema = {
    { ATTRIBUTE_TREE_ROOT, "Root node", ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE, EMPTY_STORAGE_TYPE },
    // Generic Matter attributes
    { ATTRIBUTE_NODE_ID, "NodeID", ATTRIBUTE_TREE_ROOT, C_STRING_STORAGE_TYPE },
    { ATTRIBUTE_ENDPOINT_ID, "Endpoint ID", ATTRIBUTE_NODE_ID, U16_STORAGE_TYPE },
    //{ATTRIBUTE_CLUSTER_ID, "Cluster ID", ATTRIBUTE_ENDPOINT_ID, U16_STORAGE_TYPE},
    { ATTRIBUTE_DEVICETYPELIST_ID, "DeviceType List", ATTRIBUTE_ENDPOINT_ID, C_STRING_STORAGE_TYPE },
    { ATTRIBUTE_SERVERLIST_ID, "Server Cluster List", ATTRIBUTE_ENDPOINT_ID, C_STRING_STORAGE_TYPE },
    { ATTRIBUTE_CLIENTLIST_ID, "Client Cluster List", ATTRIBUTE_ENDPOINT_ID, C_STRING_STORAGE_TYPE },
    { ATTRIBUTE_PARTSLIST_ID, "Parts/Endpoint List", ATTRIBUTE_ENDPOINT_ID, C_STRING_STORAGE_TYPE },

    { ONOFF_ATTRIBUTE_LIST, "OnOff Cluster Attribute List", ATTRIBUTE_ENDPOINT_ID, C_STRING_STORAGE_TYPE },
    { ONOFF_FEATURE_MAP_ID, "OnOff Cluster Feature Map List", ATTRIBUTE_ENDPOINT_ID, U32_STORAGE_TYPE },
    { ONOFF_CLUSTER_REVISION_ID, "OnOff Cluster Revision List", ATTRIBUTE_ENDPOINT_ID, U16_STORAGE_TYPE },
    { ONOFF_GENERATED_COMMAND_ID, "OnOff Cluster Generated Commands List", ATTRIBUTE_ENDPOINT_ID, C_STRING_STORAGE_TYPE },
    { ONOFF_ACCEPTED_COMMAND_ID, "OnOff Cluster Accepted Commands List", ATTRIBUTE_ENDPOINT_ID, C_STRING_STORAGE_TYPE },
};
// clang-format on

sl_status_t mpc_attribute_store_register_known_attribute_types()
{
    sl_status_t status = SL_STATUS_OK;
    for (auto const & a : attribute_schema)
    {
        status |= attribute_store_register_type(a.type, a.name, a.parent_type, a.storage_type);
    }
    return status;
}
