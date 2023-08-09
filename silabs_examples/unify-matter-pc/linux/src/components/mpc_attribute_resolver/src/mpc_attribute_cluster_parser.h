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

/**
 * @defgroup mpc_attribute_cluster_parser
 * @brief TODO: Implements cluster specific parsing logic
 *
 * @{
 */

#ifndef MPC_ATTRIBUTE_CLUSTER_PARSER_H
#define MPC_ATTRIBUTE_CLUSTER_PARSER_H

#include "attribute_store.h"
#include "mpc_attribute_resolver_callbacks.h"
#include "mpc_command_sender.hpp"

using namespace std;
using namespace chip;
using namespace chip::app;

/**
 * @brief function that the MPC OnOff cluster parse the attributes
 *
 * @param path          Concrete Data Attribute Path
 * @param * data        Pointer of data to be parse.
 * @param mNode         Attribute store node.
 * @returns sl_status_t SL_STATUS_OK if the attribute parsed successfully.
 *                      SL_STATUS_Fail in case of any failure.
 */
sl_status_t mpc_onoff_cluster_parser(const ConcreteDataAttributePath & path, TLV::TLVReader * data, attribute_store_node_t mNode);

/**
 * @brief function that the MPC descriptor cluster parse the attributes
 *
 * @param path          Concrete Data Attribute Path
 * @param * data        Pointer of data to be parse.
 * @param mNode         Attribute store node.
 * @returns sl_status_t SL_STATUS_OK if the attribute parsed successfully.
 *                      SL_STATUS_Fail in case of any failure.
 */
sl_status_t mpc_descriptor_cluster_parser(const ConcreteDataAttributePath & path, TLV::TLVReader * data,
                                          attribute_store_node_t mNode);

/**
 * @brief checks if MPC supports the given cluster
 *
 * @param clusterId cluster Id of the cluster to be checked for
 *
 * @returns true if cluster is supported, false if it is not
 */
bool is_cluster_supported(chip::ClusterId clusterId);

#endif // MPC_ATTRIBUTE_CLUSTER_PARSER_H
/** @} end mpc_attribute_cluster_parser */
