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
 * @defgroup mpc_attribute_types List of defined attributes types
 * @ingroup mpc_attribute_store
 * @brief List of defines Attribute Types for the attribute store.
 *
 * <b>Attribute Type Ranges:</b>
 * Range From | Range To   | Type
 * ---------- | ---------- | ----------------------------------
 * 0x00000000 | 0x00000001 | Attribute Store
 * 0x00000002 | 0x0000FFFF | Z-Wave Command Class Attributes
 * 0x00010000 | 0xFFFFFFFF | ZigBee Cluster Command Attributes
 * ?? MPC ??
 *
 * @{
 */

#ifndef MPC_ATTRIBUTE_STORE_DEFINED_ATTRIBUTE_TYPES_H
#define MPC_ATTRIBUTE_STORE_DEFINED_ATTRIBUTE_TYPES_H

#include "attribute_store.h"

#include "mpc_attribute_store_cluster_attribute_types.h"
#include "unify_dotdot_defined_attribute_types.h"

/**
 * @brief Type of a node in the attribute store.
 */
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

// Generic Node attributes, should be attached under NodeID nodes and not endpoints.
// Suggested range 0x01..0xFF

DEFINE_ATTRIBUTE(ATTRIBUTE_NODE_ID, 0x0003)
DEFINE_ATTRIBUTE(ATTRIBUTE_ENDPOINT_ID, 0x0004)

// In the below attribute types, 0x001D is Descriptor Cluster ID and is followed by
// the corresponding attribute ids of the cluster
DEFINE_ATTRIBUTE(ATTRIBUTE_DEVICETYPELIST_ID, 0x001D0000)
DEFINE_ATTRIBUTE(ATTRIBUTE_SERVERLIST_ID, 0x001D0001)
DEFINE_ATTRIBUTE(ATTRIBUTE_CLIENTLIST_ID, 0x001D0002)
DEFINE_ATTRIBUTE(ATTRIBUTE_PARTSLIST_ID, 0x001D0003)

#endif // MPC_ATTRIBUTE_STORE_DEFINED_ATTRIBUTE_TYPES_H
