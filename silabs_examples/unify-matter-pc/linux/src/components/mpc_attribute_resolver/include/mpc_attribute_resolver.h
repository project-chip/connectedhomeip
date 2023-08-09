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
 * @defgroup mpc_attribute_resolver MPC Attribute Resolver
 * @ingroup mpc_components
 * @brief MPC part of the attribute resolver
 *
 * The MPC Attribute resolver is in charge of sending frames and listening for Send
 * data callbacks in the context of resolving attributes. It then relays
 * resolution information to the @ref attribute_resolver
 *
 * @{
 */

#ifndef MPC_ATTRIBUTE_RESOLVER_H
#define MPC_ATTRIBUTE_RESOLVER_H

#include "attribute_resolver_rule.h"
#include "attribute_store.h"

typedef enum
{
    /// Frame was delivered to the node without supervision.
    FRAME_SENT_EVENT_OK_NO_SUPERVISION,
    /// Frame was delivered to the node with Supervision,
    /// and it responded with SUPERVISION_REPORT_SUCCESS.
    FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS,
    /// Frame was delivered to the node with Supervision,
    /// and it responded with SUPERVISION_REPORT_NO_SUPPORT.
    FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT,
    /// Frame was delivered to the node with Supervision,
    /// and it responded with SUPERVISION_REPORT_WORKING.
    FRAME_SENT_EVENT_OK_SUPERVISION_WORKING,
    /// Frame was delivered to the node with Supervision,
    /// and it responded with SUPERVISION_REPORT_FAIL.
    FRAME_SENT_EVENT_OK_SUPERVISION_FAIL,
    /// Frame was not delivered to the node.
    FRAME_SENT_EVENT_FAIL,
} mpc_resolver_event_t;

typedef void (*mpc_resolver_event_notification_function_t)(attribute_store_node_t node, resolver_rule_type_t rule_type,
                                                           mpc_resolver_event_t event);

/**
 * @brief sets resolution listener to process interview completetion
 *
 * @param mNodeId top level node of the end device being interviewed
 */
void mpc_attribute_resolver_helper_set_resolution_listener(attribute_store_node_t mNodeId);

#endif // MPC_ATTRIBUTE_RESOLVER_H
/** @} end mpc_attribute_resolver */
