/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

// Generic includes
#include <stdbool.h>
#include <string.h>
#include <string>
#include <vector>

// Includes from other components
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "attribute_store.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "sl_log.h"
#include "sl_status.h"

#include "mpc_attribute_resolver_cluster_rules.h"
#include "mpc_attribute_resolver_rules.h"
#include "mpc_attribute_store.h"

using namespace std;
using namespace attribute_store;
using namespace chip;

#define LOG_TAG "MPC_RESOLVER_RULE"

sl_status_t mpc_resolver_get_rule(attribute_store_node_t node, uint8_t * frame, uint16_t * frame_length)
{
    try
    {
        mpc_frame_t * pframe = (mpc_frame_t *) frame;
        attribute attr       = attribute(node);
        pframe->command      = readAttribute;

        auto typeID = attr.type();
        // Get Attribute ID
        pframe->attributeId = ((uint32_t) typeID) & 0xFFFF;

        // Get cluster ID
        pframe->clusterId = (((uint32_t) typeID) & 0xFFFF0000) >> 16;
        sl_log_debug(LOG_TAG, "Cluster ID: %d", pframe->clusterId);

        // Get endpoint ID
        attribute endpoint = attr.first_parent(ATTRIBUTE_ENDPOINT_ID);
        pframe->endpoint   = endpoint.reported<chip::EndpointId>();
        sl_log_debug(LOG_TAG, "Endpoint ID: %d", pframe->endpoint);

        // Get nodeID
        attribute nodeId = endpoint.first_parent(ATTRIBUTE_NODE_ID);
        auto nwList      = nodeId.child_by_type(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST).reported<std::string>();
        auto nodeIdStr   = nwList.erase(0, nwList.find(":") + 1);
        sl_log_debug(LOG_TAG, "Node ID: %s [%llu]", nodeIdStr.c_str(), stoull(nodeIdStr));
        pframe->nodeId = stoull(nodeIdStr);

        return SL_STATUS_OK;
    } catch (...)
    {
        std::exception_ptr p = std::current_exception();
        sl_log_error(LOG_TAG, "Failed to setup parameters for sending frame [%s]", (p ? p.__cxa_exception_type()->name() : "null"));
        return SL_STATUS_FAIL;
    }
}

sl_status_t mpc_attribute_resolver_rules_init()
{
    sl_status_t status = SL_STATUS_OK;

    // Attribute resolver rules
    // Device Type
    status = attribute_resolver_register_rule(ATTRIBUTE_DEVICETYPELIST_ID, nullptr, &mpc_resolver_get_rule);
    // Server List
    status = attribute_resolver_register_rule(ATTRIBUTE_SERVERLIST_ID, nullptr, &mpc_resolver_get_rule);
    // Client List
    status = attribute_resolver_register_rule(ATTRIBUTE_CLIENTLIST_ID, nullptr, &mpc_resolver_get_rule);
    // Part List
    status = attribute_resolver_register_rule(ATTRIBUTE_PARTSLIST_ID, nullptr, &mpc_resolver_get_rule);

    // Register cluster specific attribute resolver rules
    status = mpc_attribute_resolver_register_cluster_rules();

    return status;
}
