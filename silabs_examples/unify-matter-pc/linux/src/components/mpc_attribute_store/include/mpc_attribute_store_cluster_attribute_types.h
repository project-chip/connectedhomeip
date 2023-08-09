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
#include "attribute_store.h"

/**
 * @defgroup mpc_attribute_store_cluster_attribute_types
 * @ingroup mpc_components
 * @brief TODO: Defines cluster specific attributes that are not defined in ZCL
 *
 * @{
 */

#ifndef MPC_ATTRIBUTE_STORE_CLUSTER_ATTRIBUTE_TYPES_H
#define MPC_ATTRIBUTE_STORE_CLUSTER_ATTRIBUTE_TYPES_H

// Define ON-OFF cluster's globle attributes
DEFINE_ATTRIBUTE(ONOFF_ATTRIBUTE_LIST, 0x0006FFFB)
DEFINE_ATTRIBUTE(ONOFF_FEATURE_MAP_ID, 0x0006FFFC)
DEFINE_ATTRIBUTE(ONOFF_CLUSTER_REVISION_ID, 0x0006FFFD)
DEFINE_ATTRIBUTE(ONOFF_GENERATED_COMMAND_ID, 0x0006FFF8)
DEFINE_ATTRIBUTE(ONOFF_ACCEPTED_COMMAND_ID, 0x0006FFF9)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // MPC_ATTRIBUTE_STORE_CLUSTER_ATTRIBUTE_TYPES_H
/** @} end mpc_attribute_store_cluster_attribute_types */
