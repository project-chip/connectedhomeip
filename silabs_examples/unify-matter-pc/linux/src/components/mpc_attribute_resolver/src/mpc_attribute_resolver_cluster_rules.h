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
 * @defgroup mpc_attribute_resolver_cluster_rule
 * @brief Implements cluster specific rules and their registration
 *
 * @{
 */

#ifndef MPC_ATTRIBUTE_RESOLVER_CLUSTER_RULES_H
#define MPC_ATTRIBUTE_RESOLVER_CLUSTER_RULES_H

#include "attribute_store.h"
#include "mpc_attribute_store_cluster_attribute_types.h"
#include "unify_dotdot_defined_attribute_types.h"

/**
 * @brief function for the register cluster specific attribute resolver rules.
 *
 * In this function we'll register attribute resolver rules for cluster specific attributes.
 */
sl_status_t mpc_attribute_resolver_register_cluster_rules();
#endif // MPC_ATTRIBUTE_RESOLVER_CLUSTER_RULES_H
/** @} end mpc_attribute_resolver_cluster_rules */
