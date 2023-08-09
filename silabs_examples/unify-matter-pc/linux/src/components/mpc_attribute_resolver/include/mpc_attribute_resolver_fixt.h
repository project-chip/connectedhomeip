/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup mpc_attribute_resolver_fixt MPC Attribute Resolver Fixtured
 * @ingroup mpc_attribute_resolver
 * @brief MPC Attribute Resolver fixtures
 *
 * Setup and Teardown fixtures for the @ref mpc_attribute_resolver
 *
 * @{
 */
#ifndef MPC_ATTRIBUTE_RESOLVER_FIXT_H
#define MPC_ATTRIBUTE_RESOLVER_FIXT_H

#include "sl_status.h"

/**
 * @brief Initialize the MPC attribute resolver, which
 * in turn calls Unify attribute resolver init
 * @returns sl_status_t indicating the outcome of initialization
 */
sl_status_t mpc_attribute_resolver_init();

/**
 * @brief Teardown the MPC attribute resolver
 * It will also teardown the Unify Attribute resolver.
 * @returns 0 on success, any other value in case of error
 */
int mpc_attribute_resolver_teardown();

#endif // MPC_ATTRIBUTE_RESOLVER_FIXT
/** @} end mpc_attribute_resolver */
