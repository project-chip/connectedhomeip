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
 * @defgroup mpc_attribute_parser_fwk
 * @ingroup mpc_components
 * @brief Implements framework to register and invoke cluster specific
 *        parser implementations.
 * @{
 */

#ifndef MPC_ATTRIBUTE_PARSER_FWK_H
#define MPC_ATTRIBUTE_PARSER_FWK_H

#include "app/ConcreteAttributePath.h"
#include "attribute_store.h"
#include "lib/core/TLVReader.h"
#include "sl_status.h"

/**
 * @brief Initialize parser framework and and registers all supported clusters'
 *        parser implementations
 */
sl_status_t mpc_attribute_parser_init(void);

/**
 * @brief Invokes cluster specific parser implementation if already registered
 *
 * @param path path of attribute to which the data to be parsed belongs
 * @param data TLVReader handle for the data to be parsed
 * @param mNode the node corresponding to the path which is to be updated with parsed value
 */
sl_status_t mpc_attribute_parser_invoke(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                                        attribute_store_node_t mNode);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // MPC_ATTRIBUTE_PARSER_FWK_H
/** @} end mpc_attribute_parser_fwk */
