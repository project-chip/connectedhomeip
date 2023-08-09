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
 * @defgroup mpc_attribute_store_type_registration MPC attribute store known attribute types
 * @ingroup mpc_attribute_store
 * @brief Lists and registers all attribute types
 *
 * @{
 */

#ifndef MPC_ATTRIBUTE_STORE_TYPE_REGISTRATION_H
#define MPC_ATTRIBUTE_STORE_TYPE_REGISTRATION_H

#include "sl_status.h"

/**
 * @brief Register all known MPC/Matter attribute types
 * @return SL_STATUS_OK on success, any other code on failure.
 */
sl_status_t mpc_attribute_store_register_known_attribute_types();

#endif // ZPC_ATTRIBUTE_STORE_TYPE_REGISTRATION_H
/** @} end zpc_attribute_store_type_registration */
