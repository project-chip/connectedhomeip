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
// Includes from this component
#include <platform/PlatformManager.h>
#include <string.h>

#include "../../../mpc_command_sender.hpp"
#include "mpc_attribute_resolver_callbacks.h"
#include "mpc_attribute_resolver_rules.h"
#include "mpc_attribute_resolver_send.h"

// Unify Includes
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "attribute_resolver_rule.h"

#include "sl_log.h"

#define LOG_TAG "mpc_attribute_resolver_send"

using namespace std;
using namespace chip::DeviceLayer;
using namespace attribute_store;

void mpc_attribute_resolver_wrapper(intptr_t arg)
{
    attribute_store_node_t node = static_cast<attribute_store_node_t>(arg);
    on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK, 0, node, RESOLVER_GET_RULE);
}

sl_status_t mpc_attribute_resolver_send(attribute_store_node_t node, const uint8_t * frame_data, uint16_t frame_data_len,
                                        bool is_set)
{
    mpc_frame_t * pframe = (mpc_frame_t *) frame_data;
    auto request =
        chip::Platform::New<AttributeReadRequest>(pframe->nodeId, pframe->endpoint, pframe->clusterId, pframe->attributeId);
    request->SetCallbacks(chip::Platform::New<MPCAttributeReaderCallback>(node) );

    sl_log_debug(LOG_TAG, "NodeId : %d\tEndpointId : %d\tClusterId : %d\tAttributeId : %d\n", pframe->nodeId, pframe->endpoint,
                 pframe->clusterId, pframe->attributeId);

    request->SendCommand();

    return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Function shared among the component. Used for send_data callbacks
///////////////////////////////////////////////////////////////////////////////
void mpc_attribute_resolver_send_init()
{
    
}
