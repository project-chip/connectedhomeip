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
#include "mpc_node_monitor.h"
#include "MPCNodeReportables.hpp"
#include "attribute_store.h"
#include "mpc_attribute_resolver.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "mpc_command_sender.hpp"
#include "mpc_config.h"
#include "mpc_volatile_attributes.h"
#include "sl_log.h"

#include <map>

#define LOG_TAG "mpc_node_monitoring"

using namespace attribute_store;

static SubscribeRequestParams reportablesSubsParam;

static std::map<attribute, std::vector<attribute>> reportables_setup_list;
static std::map<attribute, MPCNodeReportables *> ongoing_reportables;

static void mpc_on_volatile_attribute_discovery(attribute_store_node_t node, attribute_store_change_t changeType)
{
    // only process when volatile attribute gets added to attribute tree but not on update
    if (changeType != ATTRIBUTE_CREATED)
        return;
    sl_log_debug(LOG_TAG, "Attempting to add volatile attribute node %x to reportables setup list", node);
    try
    {
        auto epNode   = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
        auto unidNode = attribute(epNode).parent();

        auto setupList = reportables_setup_list.find(unidNode);
        if (setupList != reportables_setup_list.end())
        {
            setupList->second.push_back(attribute(node));
        }
        else
        {
            reportables_setup_list[unidNode] = { attribute(node) };
        }
        sl_log_debug(LOG_TAG, "added volatile attribute node %x to reportables setup list", node);
    } catch (...)
    {
        sl_log_error(LOG_TAG, "Something went wrong while adding volatile attribute to reportable list");
    }
}

sl_status_t mpc_node_monitor_initiate_monitoring(attribute_store::attribute nodeId)
{
    auto setupList = reportables_setup_list.find(nodeId);

    if (setupList == reportables_setup_list.end())
    {
        sl_log_warning(LOG_TAG, "No volatile attribute discovered during interview of %u", nodeId);
        return SL_STATUS_OK;
    }
    // TODO: Do we persist the entry to storage/db/attribute_store here since interview is successful at this point?
    // we will end up not seting up reportables on reboot if we don't
    sl_log_info(LOG_TAG, "Setting up reportables for volatile attributes of %u", nodeId);
    try
    {
        auto networkList = nodeId.child_by_type(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST).reported<std::string>();
        auto nodeIdStr   = networkList.erase(0, networkList.find(":") + 1);
        std::vector<AttributePathParams> pathList;

        std::for_each(setupList->second.begin(), setupList->second.end(), [&pathList](const attribute node) {
            auto epNode        = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
            EndpointId epID    = attribute(epNode).reported<EndpointId>();
            auto type          = node.type();
            ClusterId clustID  = (type & 0xFFFF0000) >> 16;
            AttributeId attrID = type & 0xFFFF;
            pathList.push_back(AttributePathParams(epID, clustID, attrID));
        });
        NodeId destID   = std::stoull(nodeIdStr);
        auto reportable = chip::Platform::New<MPCNodeReportables>(attribute(nodeId), destID, pathList, reportablesSubsParam);
        if (reportable == nullptr)
        {
            sl_log_error(LOG_TAG, "Failed to allocate reportable");
            return SL_STATUS_FAIL;
        }
        ongoing_reportables.insert({ nodeId, reportable });

        return reportable->Initiate();
    } catch (...)
    {
        sl_log_error(LOG_TAG, "Error while setting up reportables");
        return SL_STATUS_FAIL;
    }
}

sl_status_t mpc_node_monitor_cancel_monitoring(attribute_store::attribute nodeId)
{
    auto reportableEntry = ongoing_reportables.find(nodeId);

    if (reportableEntry != ongoing_reportables.end())
    {
        auto reportable = reportableEntry->second;
        reportable->Terminate();
        ongoing_reportables.erase(nodeId);
        // Do not delete reportables here, as it is handled internally by Terminate
    }
    return SL_STATUS_OK;
}

sl_status_t mpc_node_monitor_init(void)
{
    try
    {
        std::vector<attribute_store_type_t> typeList;
        std::for_each(mpc_volatilte_attr_list.begin(), mpc_volatilte_attr_list.end(),
                      [&typeList](const std::pair<chip::ClusterId, chip::AttributeId> attr) {
                          attribute_store_type_t type;
                          type = ((attr.first & 0xFFFF) << 16) | (attr.second & 0xFFFF);
                          typeList.push_back(type);
                          attribute_store_register_callback_by_type(mpc_on_volatile_attribute_discovery, type);
                      });
        // TODO: do we setup persisted reportables for node already interviewed in previous power cycle here?

        auto cfg                              = mpc_get_config();
        reportablesSubsParam.minInterval      = 0;//cfg->reportMin;
        reportablesSubsParam.maxInterval      = cfg->reportMax;
        reportablesSubsParam.keepSubscription = false;
        return SL_STATUS_OK;
    } catch (...)
    {
        return SL_STATUS_FAIL;
    }
}
