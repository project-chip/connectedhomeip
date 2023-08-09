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
#ifndef MPC_ATTRIBUTE_RESOLVER_RULES_H
#define MPC_ATTRIBUTE_RESOLVER_RULES_H

#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

#define ATTRIBUTE_UNID_TYPE 105
#define ATTRIBUTE_NETWORK_LIST 106

using namespace chip;

enum commandType
{
    readAttribute,
    writeAttribute,
    // Add one more for generic one
};

typedef struct mpc_frame
{
    NodeId nodeId;
    EndpointId endpoint;
    ClusterId clusterId;
    AttributeId attributeId;
    commandType command;
} mpc_frame_t;

/**
 * @brief Initialization function for the attribute_resolver_rules subcomponent
 *
 * Register attributes resolver rules
 */
sl_status_t mpc_attribute_resolver_rules_init();

/**
 * @brief function that the MPC attribute resolver uses to setup parameters
 * required for sending the get MATTER frames
 *
 * @param node            Attribute node for which the frame has to be sent
 * @param *frame          Pointer of Genereted data to send
 * @param *frame_length   Pointer of length of generated data
 * @returns sl_status_t SL_STATUS_OK if the parameters setup succeed.
 */
sl_status_t mpc_resolver_get_rule(attribute_store_node_t node, uint8_t * frame, uint16_t * frame_length);

#endif
