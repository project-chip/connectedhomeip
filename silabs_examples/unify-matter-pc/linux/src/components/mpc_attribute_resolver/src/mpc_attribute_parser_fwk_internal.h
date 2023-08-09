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
 * @defgroup mpc_attribute_parser_fwk_internal
 * @ingroup mpc_components
 * @brief parser framework internals
 * @{
 */

#ifndef MPC_ATTRIBUTE_PARSER_FWK_INTERNAL_H
#define MPC_ATTRIBUTE_PARSER_FWK_INTERNAL_H
#include "mpc_attribute_parser_fwk.h"
#include <functional>
/**
 * @brief callback format to be followed by cluster parser implementors
 */
using AttributeParserFunction =
    std::function<sl_status_t(const chip::app::ConcreteDataAttributePath &, chip::TLV::TLVReader *, attribute_store_node_t)>;

/**
 * @brief interface used to register cluster specific parser
 *
 * @param clusterID cluster ID for which parser belongs to
 * @param parserCallback  callback function implementing the cluster specific parsing
 *                logic @ref AttributeParserFunction
 */
sl_status_t mpc_attribute_parser_register(chip::ClusterId clusterID, AttributeParserFunction parserCallback);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // MPC_ATTRIBUTE_PARSER_FWK_INTERNAL_H
/** @} end mpc_attribute_parser_fwk_internal */
