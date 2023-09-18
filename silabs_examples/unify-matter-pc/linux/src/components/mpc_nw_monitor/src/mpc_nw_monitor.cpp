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

#include "mpc_nw_monitor.h"
#include "attribute.hpp"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "mpc_attribute_resolver.h"
#include "mpc_attribute_store_defined_attribute_types.h"
#include "mpc_node_monitor.h"
#include "sl_log.h"
#include "uic_attribute_definitions.h"
#include "unify_dotdot_attribute_store.h"
#include "zap-types.h"
#include <boost/algorithm/string.hpp>
#include "matter_pc_main.hpp"

// Matter includes
#include "app/server/Server.h"
#include "crypto/CHIPCryptoPAL.h"
#include "lib/core/DataModelTypes.h"
#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/ResolverProxy.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <sstream>
#include <stdbool.h>
#include <string>

#define LOG_TAG "mpc_nw_monitor"

using namespace chip;
using namespace attribute_store;
using namespace std;

static void generateUNID(string & unid)
{
    unsigned long rn;

    stringstream stream;
    stream.setf(ios::hex, ios::basefield);
    stream.setf(ios::internal, ios::adjustfield);
    stream.fill('0');
    stream.width(16);

    Crypto::DRBG_get_bytes(reinterpret_cast<uint8_t *>(&rn), sizeof(rn));
    stream << rn;

    unid.append("mt-" + stream.str());
}

#ifndef UMPC_DISABLE_DISCOVERY
class OperationalDiscover : public chip::Dnssd::OperationalBrowseDeleagete
{
    bool needs_update(attribute node)
    {
        try
        {
            auto state = node.child_by_type(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS).reported<NodeStateNetworkStatus>();
            return ((state != ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL) &&
                    (state != ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL));
        } catch (std::invalid_argument const & ex)
        {
            return false;
        }
    }
    void OnOperationalNodeDiscovered(const chip::Dnssd::OperationalNodeData & operationalData) override
    {
        sl_log_debug(LOG_TAG, "Found matter node with node ID " ChipLogFormatX64 ":" ChipLogFormatX64, 
                            ChipLogValueX64(operationalData.peerId.GetCompressedFabricId()),
                            ChipLogValueX64(operationalData.peerId.GetNodeId()));
        if (chip::Server::GetInstance().GetFabricTable().FindFabricWithCompressedId(
                                                        operationalData.peerId.GetCompressedFabricId()))
        {
            // prepare networkList entry
            auto networkListEntry = to_string(operationalData.peerId.GetCompressedFabricId());
            networkListEntry.append(":");
            networkListEntry.append(to_string(operationalData.peerId.GetNodeId()));
            attribute node;

            try
            {
                // If networkList entry already exists in one of the node, we need to skip processing
                for (auto unids : attribute::root().children(ATTRIBUTE_NODE_ID))
                {
                    node = unids.child_by_type(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST);
                    if (!node.is_valid()) 
                    {
                        // ideally not a possible case but skip if networklist doesn't exist
                        sl_log_debug(LOG_TAG, "Skipped node [%x] due to absence of networkList attribute", node);
                        continue;
                    }
                    auto node_nw_list_str = node.reported<string>();
                    std::vector<std::string> node_nw_list;
                    size_t findIndex = 0;
                    boost::algorithm::split(node_nw_list, node_nw_list_str.c_str(), boost::is_any_of(","));
                    for (findIndex = 0; (node_nw_list[findIndex] != networkListEntry) 
                                            && (findIndex < node_nw_list.size()); findIndex++);
                    if (findIndex != node_nw_list.size())
                    {
                        sl_log_debug(LOG_TAG, "entry already exists in networkList of %s", unids.reported<string>().c_str());
                        break;
                    }
                    node = attribute(ATTRIBUTE_STORE_INVALID_NODE);
                }

                if (node.is_valid())
                {
                    if (!needs_update(node))
                    {
                        sl_log_debug(LOG_TAG, "skipping addition of node %u since it already exists in attribute tree",
                                     operationalData.peerId.GetNodeId());
                        return;
                    }
                }
                else
                {
                    sl_log_debug(LOG_TAG, "Found matter node with node ID " ChipLogFormatX64, ChipLogValueX64(operationalData.peerId.GetNodeId()));
                    string unid;
                    generateUNID(unid);
                    node = attribute::root().add_node(ATTRIBUTE_NODE_ID);
                    attribute_store_set_reported_string(node, unid.c_str());
                }
                auto state = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
                // MPC itself doesn't need to be interviewed, directly move to Online Functional
                if (chip::Server::GetInstance()
                        .GetFabricTable()
                        .FindFabricWithCompressedId(operationalData.peerId.GetCompressedFabricId())
                        ->GetNodeId() == operationalData.peerId.GetNodeId())
                {
                    sl_log_info(LOG_TAG, "MPC moved to Online Functional");
                    state = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
                }
                else
                {
                    // Register resolver completion call back for all other node for post interview processing
                    mpc_attribute_resolver_helper_set_resolution_listener(node);
                }
                sl_log_debug(LOG_TAG, "networkList formed [%s] -> <%s>", node.reported<string>().c_str(), networkListEntry.c_str());
                attribute_store_set_reported_string(node.emplace_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST), networkListEntry.c_str());
                auto ep = node.emplace_node<EndpointId>(ATTRIBUTE_ENDPOINT_ID, 0);
                ep.emplace_node<NodeStateSecurity>(DOTDOT_ATTRIBUTE_ID_STATE_SECURITY, ZCL_NODE_STATE_SECURITY_MATTER);
                ep.emplace_node<uint32_t>(DOTDOT_ATTRIBUTE_ID_STATE_MAXIMUM_COMMAND_DELAY, 1);
                node.emplace_node<NodeStateNetworkStatus>(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, state);

            } catch (...)
            {
                if (node.is_valid())
                    node.delete_node();
                std::exception_ptr p = std::current_exception();
                sl_log_error(LOG_TAG, "Failed to add discovered node [%s]", (p ? p.__cxa_exception_type()->name() : "null"));
            }
        }
        mpc_schedule_contiki();
    }
};
#endif

static void mpc_on_ep_change_cb(attribute_store_node_t node, attribute_store_change_t change)
{
    // if there is only one node in under root, then it must belong to MPC itself
    if (attribute::root().child_count() > 1)
    {
        // If attribute node belongs to MPC itself we can skip the interview process.
        auto pFabricInfo = chip::Server::GetInstance().GetFabricTable().FindFabricWithIndex(1);
        if (!pFabricInfo) return;
        auto mpcNodeId = pFabricInfo->GetNodeId();

        auto parentOfChangedAttributeNode = attribute(node).parent().child_by_type(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST);
        if (!parentOfChangedAttributeNode.is_valid())
        {
            return;
        }
        auto parentNetworkList = parentOfChangedAttributeNode.reported<string>();

        if (change == ATTRIBUTE_CREATED)
        {
            vector<std::string> network_list_parsed;
            boost::algorithm::split(network_list_parsed, parentNetworkList.c_str(), boost::is_any_of(":,"));
            for (uint8_t i = 1; i < network_list_parsed.size(); i += 2)
            {
                if (network_list_parsed[i] == to_string(mpcNodeId))
                {
                    sl_log_info(LOG_TAG, "skipping descriptor cluster addition for MPC");
                    return;
                }
            }

            attribute ep = attribute(node);
            try
            {
                ep.emplace_node(ATTRIBUTE_SERVERLIST_ID);
                ep.emplace_node(ATTRIBUTE_CLIENTLIST_ID);
                ep.emplace_node(ATTRIBUTE_PARTSLIST_ID);
                ep.emplace_node(ATTRIBUTE_DEVICETYPELIST_ID);
            } catch (...)
            {
                uint8_t epid = ep.reported<uint8_t>();
                sl_log_error(LOG_TAG, "Failure in adding description cluster or its attributes to ep-%u", epid);
            }
        }
    }
}

static void mpc_start_node_discovery()
{
#ifndef UMPC_DISABLE_DISCOVERY
    static OperationalDiscover mDNSdiscover;
    // Start discovering nodes
    if (chip::Dnssd::Resolver::Instance().IsInitialized())
    {
        chip::Dnssd::Resolver::Instance().SetOperationalBrowseDelegate(&mDNSdiscover);
        chip::Dnssd::Resolver::Instance().DiscoverOperational();
    }
#endif
}

static void find_mpc_and_update_networklist()
{
    attribute mpcNode;
    string networkList;

    if (!attribute::root().child_count())
    {
        sl_log_warning(LOG_TAG, "find_mpc_and_update_networklist() called when before MPC entry");
        return;
    }
    // if there is only one node in under root, then it must belong to MPC itself
    if (attribute::root().child_count() == 1)
    {
        mpcNode = attribute::root().child_by_type(ATTRIBUTE_NODE_ID);
    }

    // loop logic here is applicable when multi-fabric is to be supported
    auto fabric = Server::GetInstance().GetFabricTable().cbegin();
    for (; fabric != Server::GetInstance().GetFabricTable().cend(); fabric++)
    {
        // prepare networkList entry
        auto networkListItem = to_string(fabric->GetCompressedFabricId());
        networkListItem.append(":");
        networkListItem.append(to_string(fabric->GetNodeId()));

        // append if it can fit into attribute storage size, else warn
        if (networkList.size() + networkListItem.size() + 1 < ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH)
            networkList.append(networkListItem + ",");
        else
            sl_log_warning(LOG_TAG, "networkListItem not appended because of size constraint, update attribute storage strategy");

        // check if mpcNode is already found we skip the search and move to check iteration
        if (mpcNode.is_valid())
            continue;
        // else search for node with networkList entry matching that of MPC
        for (auto node : attribute::root().children(ATTRIBUTE_NODE_ID))
        {
            auto nwListEntry = node.child_by_type(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST);
            if (nwListEntry.is_valid() && nwListEntry.reported<string>().find(networkListItem) != string::npos)
                mpcNode = node;
        }
    }
    networkList.pop_back(); // remove the last "," delimiter

    if (!mpcNode.is_valid())
    {
        sl_log_error(LOG_TAG, "did not find node that belongs to MPC in attribute store!");
        return;
    }
    attribute_store_set_reported_string(mpcNode.emplace_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST), networkList.c_str());
    mpcNode.emplace_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS)
        .set_reported<NodeStateNetworkStatus>(ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL);
}

static void EventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        mpc_start_node_discovery();
    }
}

static sl_status_t mpc_populate_attribute_store()
{
    try
    {
        string mpcUnid;
        generateUNID(mpcUnid);
        auto mpcNode = attribute::root().emplace_node(ATTRIBUTE_NODE_ID);
        attribute_store_set_reported_string(mpcNode, mpcUnid.c_str());
        auto epNode = mpcNode.emplace_node<EndpointId>(ATTRIBUTE_ENDPOINT_ID, 0);
        epNode.emplace_node<NodeStateSecurity>(DOTDOT_ATTRIBUTE_ID_STATE_SECURITY, ZCL_NODE_STATE_SECURITY_MATTER);
        epNode.emplace_node<uint32_t>(DOTDOT_ATTRIBUTE_ID_STATE_MAXIMUM_COMMAND_DELAY, 1);
        mpcNode.emplace_node<NodeStateNetworkStatus>(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                                     ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL);
        
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0)
        {
            find_mpc_and_update_networklist();
            mpc_start_node_discovery();
        }
        // else we won't populate networkList here as the MPC would not yet be commisioned
        return SL_STATUS_OK;
    } catch (...)
    {
        sl_log_error(LOG_TAG, "Error occured while populating MPC entries to attribute store");
        return SL_STATUS_FAIL;
    }
}

class MPCFabricDelegate : public FabricTable::Delegate
{
    void OnFabricUpdated(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        sl_log_info(LOG_TAG, "fabric with ID %x added at index %u",
                    fabricTable.FindFabricWithIndex(fabricIndex)->GetCompressedFabricId(), fabricIndex);
        find_mpc_and_update_networklist();
    }
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        sl_log_info(LOG_TAG, "fabric count for MPC [%u]", fabricTable.FabricCount());
        find_mpc_and_update_networklist();
        // TODO: remove all end node belonging to the fabric from attribute tree?
    }
};

sl_status_t mpc_nw_monitor_init()
{
    static MPCFabricDelegate mpc_fabric_delegate;
    attribute_store_register_callback_by_type(mpc_on_ep_change_cb, ATTRIBUTE_ENDPOINT_ID);

    // Listen to fabricTable events to update networkList of MPC
    Server::GetInstance().GetFabricTable().AddFabricDelegate(&mpc_fabric_delegate);

    // Listen to events to start node discovery on commision completion
    DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);
    
    // register listeners needed to setup reportables
    mpc_node_monitor_init();

    // Create attribute tree entries for MPC itself
    if (attribute::root().child_count() == 0)
    {
        // need to create entries for MPC only on first boot-up
        // i.e. there is no entries in attribute tree or data store.
        mpc_populate_attribute_store();
        return SL_STATUS_OK;
    }
    else
    {
        // TODO: we may need to check networkList against fabrictable to identify and delete stale fabric's entries
        // just checking availability if any fabric for now.
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            //if MPC doesn't have fabric info then its not present in any fabric 
            // in which case any data in database or attribute store is stale
            sl_log_warning(LOG_TAG, "MPC isn't part of any fabric, "
                                "attribute store is stale. deleting stale attribute store entries!");
            auto nodeList = attribute::root().children(ATTRIBUTE_NODE_ID);
            std::for_each(nodeList.begin(), nodeList.end(), [](attribute node){node.delete_node();});
            // wiped attribute store so need populate MPC entry
            mpc_populate_attribute_store();
            return SL_STATUS_OK;
        }
        // also start node discovery immediately to discover new nodes
        mpc_start_node_discovery();
    }

    // if MPC already has nodes available in attribute store, refresh them to publish last
    // known status of all the nodes including MPC itself
    sl_log_info(LOG_TAG, "refreshing all node states");
    for (auto unidNode : attribute::root().children())
    {
        mpc_attribute_resolver_helper_set_resolution_listener(unidNode);
        attribute_store_refresh_node_and_children_callbacks(unidNode);
    }

    return SL_STATUS_OK;
}
