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
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <platform/PlatformManager.h>
#include <stdbool.h>
#include <string.h>
#include <vector>

#include "mpc_attribute_cluster_parser.h"
#include "mpc_attribute_parser_fwk.h"
#include "mpc_attribute_resolver.h"
#include "mpc_attribute_resolver_callbacks.h"
#include "mpc_attribute_resolver_rules.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "mpc_command_sender.hpp"
#include "mpc_node_monitor.h"
#include "mpc_attribute_store.h"
#include "zap-types.h"

// Unify Components
#include "sl_log.h"
#include "sl_status.h"

// Includes from other components
#include "attribute.hpp"
#include "attribute_resolver.h"
#include "attribute_resolver_rule.h"
#include "attribute_store.h"
#include "matter_pc_main.hpp"

// Generic includes
#include <boost/algorithm/string.hpp>
#include <map>

using namespace std;
using namespace attribute_store;
using namespace chip;
using namespace chip::DeviceLayer;

constexpr char LOG_TAG[] = "mpc_attribute_resolver";


void MPCAttributeReaderCallback::OnAttributeData(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data,
                                                 const chip::app::StatusIB & status)
{
    CHIP_ERROR error = status.ToChipError();

    if (error != CHIP_NO_ERROR || data == nullptr)
    {
        sl_log_error(LOG_TAG, "Response : %s", chip::ErrorStr(error));
        on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED, 0, mNodeId, RESOLVER_GET_RULE);
    } else {
        sl_log_debug(LOG_TAG, "Response : %s", chip::ErrorStr(error));        
        // In MATTER SDK we don't have a callback for transmission complete, so in the response msg we are
        // calling transmission completion api.
        on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK, 0, mNodeId, RESOLVER_GET_RULE);

        if (SL_STATUS_OK != mpc_attribute_parser_invoke(path, data, mNodeId)) {
            sl_log_error(LOG_TAG, "Attribute parse error of  id = %i ", mNodeId);
        }
    }
    mpc_schedule_contiki();
}

void MPCAttributeReaderCallback::OnError(CHIP_ERROR err)
{
    sl_log_error(LOG_TAG, "Error reading attribute : %s", chip::ErrorStr(err));
    on_resolver_send_data_complete(RESOLVER_SEND_STATUS_FAIL,
                                   0, // ToDo: Check the significance of this and pass proper value.
                                   mNodeId, RESOLVER_GET_RULE);    
}

static bool mpc_attribute_resolver_check_app_cluster_resolution(attribute ep)
{
    auto serverListNode = ep.child_by_type(ATTRIBUTE_SERVERLIST_ID);
    if (!serverListNode.is_valid()) {
        return true; // serverList will not be present for MPC, just return true
    }
    auto serverListStr = ep.child_by_type(ATTRIBUTE_SERVERLIST_ID).reported<std::string>();
    std::vector<std::string> server_list;
    boost::algorithm::split(server_list, serverListStr.c_str(), boost::is_any_of(","));

    // In server list check mpc supported cluster present or not.
    for (size_t i = 0; i < server_list.size(); i++)
    {
        if (is_cluster_supported(stoul(server_list[i])))
        {
            attribute_store_type_t clusterBaseId = ((stoul(server_list[i]) & 0xFFFF) << 16);
            attribute_store_type_t clusterAttrListId =
                clusterBaseId | (chip::app::Clusters::Globals::Attributes::AttributeList::Id & 0xFFFF);

            // Check application cluster's attribute defined or not
            auto attrListStr = ep.child_by_type(clusterAttrListId).reported<std::string>();
            std::vector<std::string> attrList;
            boost::algorithm::split(attrList, attrListStr.c_str(), boost::is_any_of(","));
            size_t j = 0;
            for (; j < attrList.size(); j++)
            {
                attribute_store_type_t clusterAttrId = clusterBaseId | (stoul(attrList[j]) & 0xFFFF);
                if (!ep.child_by_type(clusterAttrId).reported_exists())
                {
                    sl_log_error(LOG_TAG, "attribute %u not resolved for %u", clusterAttrId, ep);
                    break;
                }
            }
            // If all attribute are resolved successfully for atleast one app cluster, publish as functional
            if (j == attrList.size())
            {
                return true;
            }
        }
    }
    return false;
}

void mpc_attribute_resolver_resolution_completion(attribute_store_node_t mNodeId)
{
    // if not even single app cluster is completely resolved marked as non-functional
    NodeStateNetworkStatus state = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;

    try
    {
        auto epList = attribute(mNodeId).children(ATTRIBUTE_ENDPOINT_ID);

        sl_log_info(LOG_TAG, "Node Interview Completion Process ongoing for %u", mNodeId);
        for (auto ep : epList)
        {
            // if any on cluster has atleast one supported app cluster resolved mark functional
            if (mpc_attribute_resolver_check_app_cluster_resolution(ep))
            {
                state = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
                break;
            }
        }
    } catch (...)
    {
        std::exception_ptr p = std::current_exception();
        sl_log_error(LOG_TAG, "Failed to process node interview completion [%s]", (p ? p.__cxa_exception_type()->name() : ""));
    }

    attribute_store_set_child_reported(mNodeId, DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, &state, sizeof(state));
    mpc_node_monitor_initiate_monitoring(mNodeId);
}

void mpc_attribute_resolver_helper_set_resolution_listener(attribute_store_node_t mNodeId)
{
    attribute_resolver_set_resolution_listener(mNodeId, mpc_attribute_resolver_resolution_completion);
}
