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
 * @defgroup mpc_attribute_resolver_callbacks MPC Attribute Resolver callbacks
 * @ingroup mpc_attribute_resolver_internals
 * @brief Collects send data callbacks from @ref attribute_resolver_send and
 * @ref attribute_resolver_group_send and relays them to the
 * @ref attribute_resolver
 *
 * @{
 */

#ifndef MPC_ATTRIBUTE_RESOLVER_CALLBACKS_H
#define MPC_ATTRIBUTE_RESOLVER_CALLBACKS_H

#include "attribute_resolver.h"
#include "attribute_resolver_rule.h"
#include "attribute_store.h"
#include "mpc_command_sender.hpp"

#include "app/ReadClient.h"

using namespace std;
using namespace chip::app;

/**
 * @brief Struct keeping track of a node
 * resolved as well as the rule type being executed
 */
typedef struct resolution_map
{
    attribute_store_node_t node;
    resolver_rule_type_t rule_type;
} resolution_map_t;

class MPCAttributeReaderCallback : public ReadClient::Callback
{
public:
    MPCAttributeReaderCallback(attribute_store_node_t nodeId) : mNodeId(nodeId) {}
    void OnDone(ReadClient * client) override{ chip::Platform::Delete(this);
    }

    void OnError(CHIP_ERROR err) override;

    void OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                         const chip::app::StatusIB & status) override;
private:
    attribute_store_node_t mNodeId;

};

typedef sl_status_t (*mpc_attribute_resolver_resolution_completion_t)(attribute_store_node_t mNode);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief process interview completion with necessary checks and updates state
 *
 * @param mNodeId top most node for a given end device whose interview is completed
 */
void mpc_attribute_resolver_resolution_completion(attribute_store_node_t mNodeId);

#ifdef __cplusplus
}
#endif

#endif // MPC_ATTRIBUTE_RESOLVER_CALLBACKS_H
/** @} end mpc_attribute_resolver_callbacks */
