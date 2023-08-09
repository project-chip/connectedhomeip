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
#include <stdio.h>

#include "app/server/Server.h"
#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_configuration.h"
#include "attribute_store_helper.h"
#include "mpc_attribute_store.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "mpc_attribute_store_type_registration.h"
#include "sl_status.h"
#include "unify_dotdot_attribute_store.h"

typedef char unid_t[MAXIMUM_UNID_SIZE];

#define UNID_FORMAT "mt-%016lX"

static sl_status_t mpc_attribute_store_network_helper_get_unid_endpoint_from_node(attribute_store_node_t node, unid_t unid,
                                                                              dotdot_endpoint_id_t * endpoint_id);

static sl_status_t mpc_attribute_store_network_helper_get_unid_from_node(attribute_store_node_t node, unid_t unid);

static const unify_dotdot_attribute_store_configuration_t mpc_configuration = {
    .get_endpoint_node_function                  = mpc_attribute_store_network_helper_get_endpoint_node,
    .get_unid_endpoint_function                  = &mpc_attribute_store_network_helper_get_unid_endpoint_from_node,
    .get_unid_function                           = mpc_attribute_store_network_helper_get_unid_from_node,
    .update_attribute_desired_values_on_commands = true,
    .clear_reported_on_desired_updates           = false,
    .automatic_deduction_of_supported_commands   = true,
    .force_read_attributes_enabled               = false,
    .write_attributes_enabled                    = true,
    .publish_desired_attribute_values_to_mqtt    = true,
    .publish_reported_attribute_values_to_mqtt   = true,
    .node_type                                   = ATTRIBUTE_NODE_ID,
    .endpoint_type                               = ATTRIBUTE_ENDPOINT_ID
};

static attribute_store_node_t attribute_store_network_helper_get_node_id_node(const unid_t node_unid)
{
    attribute_store_node_t root_node = attribute_store_get_root();
    if (root_node == ATTRIBUTE_STORE_INVALID_NODE)
    {
        // Attribute store not initialized
        return ATTRIBUTE_STORE_INVALID_NODE;
    }
    auto len = strlen(node_unid) + 1; //c string storage includes '\0' as well in the size hence strlen() + 1 is needed
    return attribute_store_get_node_child_by_value(root_node, ATTRIBUTE_NODE_ID, REPORTED_ATTRIBUTE, (uint8_t *) node_unid,
                                                   len, 0);
}

attribute_store_node_t mpc_attribute_store_network_helper_get_endpoint_node(const dotdot_unid_t node_unid,
                                                                               dotdot_endpoint_id_t endpoint_id)
{
    attribute_store_node_t node_id_identifier = attribute_store_network_helper_get_node_id_node(node_unid);
    if (node_id_identifier == ATTRIBUTE_STORE_INVALID_NODE)
    {
        return ATTRIBUTE_STORE_INVALID_NODE;
    }

    // Look for an endpoint under the node_id node:
    chip::EndpointId epID = endpoint_id;
    return attribute_store_get_node_child_by_value(node_id_identifier, ATTRIBUTE_ENDPOINT_ID, REPORTED_ATTRIBUTE, (uint8_t *)&epID,
                                                   sizeof(chip::EndpointId), 0);
}

static sl_status_t mpc_attribute_store_network_helper_get_unid_from_node(attribute_store_node_t node, unid_t unid)
{
    if (attribute_store_get_node_type(node) != ATTRIBUTE_NODE_ID)
    {
        // Climb up until we find a NodeID type of node
        node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);
    }
    if (node == ATTRIBUTE_STORE_INVALID_NODE)
    {
        // Abort if no NodeID found.
        return SL_STATUS_NOT_FOUND;
    }

    if (SL_STATUS_OK != attribute_store_get_reported_string(node, unid, sizeof(unid_t)))
    {
        // Abort if no NodeID data was retrieved.
        return SL_STATUS_FAIL;
    }

    return SL_STATUS_OK;
}

static sl_status_t mpc_attribute_store_network_helper_get_unid_endpoint_from_node(attribute_store_node_t node, unid_t unid,
                                                                              dotdot_endpoint_id_t * endpoint_id)
{
    if (attribute_store_get_node_type(node) != ATTRIBUTE_ENDPOINT_ID)
    {
        node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
    }
    if (node == ATTRIBUTE_STORE_INVALID_NODE)
    {
        return SL_STATUS_NOT_FOUND;
    }

    chip::EndpointId endpoint;
    if (SL_STATUS_OK != attribute_store_get_reported(node, &endpoint, sizeof(endpoint)))
    {
        // Abort if no endpoint data was retrieved.
        return SL_STATUS_FAIL;
    }

    mpc_attribute_store_network_helper_get_unid_from_node(node, unid);
    *endpoint_id = endpoint;
    return SL_STATUS_OK;
}

sl_status_t mpc_attribute_store_get_node_and_endpoint_from_attribute(attribute_store_node_t node, chip::NodeId & nodeID,
                                                                     chip::EndpointId & endpointID)
{
    using namespace attribute_store;
    try
    {
        attribute attr = node;
        // Get endpoint ID
        attribute endpoint;
        if (attr.type() != ATTRIBUTE_ENDPOINT_ID)
            endpoint = attr.first_parent(ATTRIBUTE_ENDPOINT_ID);
        else
            endpoint = attr;
        endpointID         = endpoint.reported<chip::EndpointId>();

        // Get nodeID
        attribute nodeId = endpoint.first_parent(ATTRIBUTE_NODE_ID);
        auto nwList      = nodeId.child_by_type(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST).reported<std::string>();
        auto nodeIdStr   = nwList.erase(0, nwList.find(":") + 1);
        nodeID           = stoull(nodeIdStr);
        return SL_STATUS_OK;
    } catch (...)
    {
        return SL_STATUS_FAIL;
    }
}

sl_status_t mpc_attribute_store_get_endpoint_and_node_from_unid(const dotdot_unid_t unid, dotdot_endpoint_id_t dotdot_endpoint,
                                                                chip::NodeId & nodeID, chip::EndpointId & endpointID)
{
    attribute_store::attribute endpoint_node = mpc_attribute_store_network_helper_get_endpoint_node(unid, dotdot_endpoint);
    return mpc_attribute_store_get_node_and_endpoint_from_attribute(endpoint_node, nodeID, endpointID);
}

sl_status_t mpc_attribute_store_init()
{
    sl_status_t status = mpc_attribute_store_register_known_attribute_types();

    // Configure the Unify DotDot Attribute Store component:
#ifdef DEBUG_BUILD
    attribute_store_configuration_set_type_validation(true);
#endif
    unify_dotdot_attribute_store_set_configuration(&mpc_configuration);

    return status;
}
