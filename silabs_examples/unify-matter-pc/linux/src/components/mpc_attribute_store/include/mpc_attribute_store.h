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
#ifndef MPC_ATTRIBUTE_STORE_H
#define MPC_ATTRIBUTE_STORE_H

#include "app/server/Server.h"
#include "attribute_store.h"
#include "sl_status.h"
#include "unify_dotdot_attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize MPC/Matter attribute store
 * @return SL_STATUS_OK on success, any other code on failure.
 */
sl_status_t mpc_attribute_store_init();

/**
 * @brief Get the matter node ID and endpoint ID and the from an attribute
 *
 * This function will search the attribute store node and its parents to locate the matter IDs
 *
 * @return SL_STATUS_OK if both nodeID and endpoint id was found
 */
sl_status_t mpc_attribute_store_get_node_and_endpoint_from_attribute(attribute_store_node_t node, chip::NodeId & nodeID,
                                                                     chip::EndpointId & endpointID);

/**
 * @brief Get the matter node ID and endpoint ID and the from the unid and endpoint
 *
 * @param unid
 * @param endpoint
 * @param nodeID
 * @param endpointID
 * @return SL_STATUS_OK if both nodeID and endpoint id was found
 */
sl_status_t mpc_attribute_store_get_endpoint_and_node_from_unid(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint,
                                                                chip::NodeId & nodeID, chip::EndpointId & endpointID);

/**
 * @brief get attribute store node for endpoint ID type with given endpoint ID for given UNID
 * 
 * @param node_unid UNID of the end node/device
 * @param endpoint_id endpoint ID for which attribute store node is to be fetched
 * @returns attribute store node for the requested UNID and ep ID pair
*/
attribute_store_node_t mpc_attribute_store_network_helper_get_endpoint_node(const dotdot_unid_t node_unid,
                                                                               dotdot_endpoint_id_t endpoint_id);
#ifdef __cplusplus
}
#endif

#endif // MPC_ATTRIBUTE_STORE_H
