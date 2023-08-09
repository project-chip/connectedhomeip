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
#include "attribute.hpp"
#include "sl_status.h"

/**
 * @defgroup mpc_node_monitor
 * @ingroup mpc_components
 * @brief TODO: This sub-module is responsible for setting up reportable
 *              for volatile attributes of each end node during interview completion
 *              and ensure reportable are handled in order to update attributes
 *
 * @{
 */

/**
 * @brief Enables node monitoring logic.
 *
 */
sl_status_t mpc_node_monitor_init(void);

/**
 * @brief Start monitoring volatile attributes of given end node/device
 *
 * @param nodeId node id of type ATTRIBUTE_NODE_ID corresponding to the end node/device
 */
sl_status_t mpc_node_monitor_initiate_monitoring(attribute_store::attribute nodeId);

/**
 * @brief Stop/Cancel monitoring volatile attributes of given end node/device
 *
 * @param nodeId node id of type ATTRIBUTE_NODE_ID corresponding to the end node/device
 */
sl_status_t mpc_node_monitor_cancel_monitoring(attribute_store::attribute nodeId);

#ifndef MPC_NODE_MONITOR_H
#define MPC_NODE_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // MPC_NODE_MONITOR_H
/** @} end mpc_node_monitor */
