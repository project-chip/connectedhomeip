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
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

// Generic includes
#include <stdbool.h>
#include <string.h>
#include <vector>

// Includes from other components
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "attribute_store.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "sl_log.h"
#include "sl_status.h"

#include "mpc_attribute_resolver_cluster_rules.h"
#include "mpc_attribute_resolver_rules.h"

using namespace std;
using namespace attribute_store;
using namespace chip;

#define LOG_TAG "MPC_RESOLVER_CLUSTER_RULE"

sl_status_t mpc_attribute_resolver_register_onoff_cluster_rule()
{
    sl_status_t status;
    // ON-OFF cluster's attribute list
    status = attribute_resolver_register_rule(ONOFF_ATTRIBUTE_LIST, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's feature map list
    status |= attribute_resolver_register_rule(ONOFF_FEATURE_MAP_ID, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's revision cluster list
    status |= attribute_resolver_register_rule(ONOFF_CLUSTER_REVISION_ID, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's generated commands list
    status |= attribute_resolver_register_rule(ONOFF_GENERATED_COMMAND_ID, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's supported commands list
    status |= attribute_resolver_register_rule(ONOFF_ACCEPTED_COMMAND_ID, nullptr, &mpc_resolver_get_rule);

    return status;
}

sl_status_t mpc_attribute_resolver_register_onoff_attribute_list_rule()
{
    sl_status_t status;
    // ON-OFF cluster's attribute list
    status = attribute_resolver_register_rule(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's feature map list
    status |= attribute_resolver_register_rule(DOTDOT_ATTRIBUTE_ID_ON_OFF_GLOBAL_SCENE_CONTROL, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's revision cluster list
    status |= attribute_resolver_register_rule(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's generated commands list
    status |= attribute_resolver_register_rule(DOTDOT_ATTRIBUTE_ID_ON_OFF_OFF_WAIT_TIME, nullptr, &mpc_resolver_get_rule);

    // ON-OFF cluster's supported commands list
    status |= attribute_resolver_register_rule(DOTDOT_ATTRIBUTE_ID_ON_OFF_START_UP_ON_OFF, nullptr, &mpc_resolver_get_rule);
    return status;
}

sl_status_t mpc_attribute_resolver_register_cluster_rules()
{
    sl_status_t status;
    // Register rule for onoff cluster's global attributes
    status = mpc_attribute_resolver_register_onoff_cluster_rule();
    status |= mpc_attribute_resolver_register_onoff_attribute_list_rule();
    return status;
}
