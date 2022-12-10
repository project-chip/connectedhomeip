/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include <unordered_map>
#include <vector>
#include <sstream>

// Matter includes
#include "matter.h"
// Unify bridge components
#include "matter_node_state_monitor.hpp"
#include "matter_cluster_interactor.hpp"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"
#include "matter_data_storage.hpp"
// Unify lib components 
#include "sl_log.h"
namespace unify::zap_types {
  #include "zap-types.h"
}

constexpr const char *LOG_TAG = "matter_node_state_monitor";
namespace unify::matter_bridge
{
matter_node_state_monitor::matter_node_state_monitor(
  const class device_translator &translator, UnifyEmberInterface &ember_interface) :
  matter_device_translator(translator), unify_ember_interface(ember_interface)
{
  unify::node_state_monitor::node_state_monitor::get_instance().set_interface(
    this);
}

void matter_node_state_monitor::on_unify_node_added(
  const unify::node_state_monitor::node &node)
{
  for (const auto &[ep_id, ep]: node.endpoints) {
    uint8_t count_number_of_clusters_matched_to_matter = 0;

    // TODO: This is a temporary solution to ignore endpoints with only the group cluster.
    if (ep.clusters.size() <= 2 && (ep.clusters.begin()->first == "Groups" || ep.clusters.begin()->first == "NameAndLocation")) {
      sl_log_warning(LOG_TAG, "Ignoring endpoint %d from UNID %s with only the 'Groups' or 'NameAndLocation' clusters", ep_id, node.unid.c_str());
      continue;
    }

    for (const auto &[cluster_name, cluster]: ep.clusters) {
      // Skip clusters that doesn't have attributes
      if (cluster.attributes.empty()) {
        continue;
      }
      if (matter_device_translator.get_cluster_id(cluster_name).has_value()) {
        count_number_of_clusters_matched_to_matter++;
      }
    }
    if (count_number_of_clusters_matched_to_matter == 0) {
      continue;
    }
    matter_endpoint_builder builder;
    cluster_interactor cluster_interactor(matter_device_translator, builder);
    cluster_interactor.build_matter_cluster(ep.clusters);
    auto matter_device_type = cluster_interactor.get_matter_type();
    if (!matter_device_type.has_value()) {
      continue;
    }
    matter_endpoint_context ember_endpoint = builder.finalize();
    struct bridged_endpoint bridge(std::move(ember_endpoint));
    bridge.unify_unid     = node.unid;
    bridge.unify_endpoint = ep_id;
    bridge.matter_type    = matter_device_type.value();
    // check the unify node has already assigned dynamic endpoint from
    // persisted storage. If it exists, use the persisted endpoint
    // if not get the next available endpoint using 'get_next_ember_endpoint_id'
    matter_data_storage::endpoint_mapping unify_node
      = {bridge.unify_unid.c_str(), bridge.unify_endpoint};
    bool status_persisted_endpoint_map
      = matter_data_storage::instance().get_persisted_data(unify_node);
    if (status_persisted_endpoint_map) {
      bridge.matter_endpoint = unify_node.matter_endpoint.value();
    } else {
      bridge.matter_endpoint = unify_ember_interface.getNextDynamicAvailableEndpointIndex();
      matter_data_storage::endpoint_mapping endpoint_map_info
        = {bridge.unify_unid.c_str(),
           bridge.unify_endpoint,
           bridge.matter_endpoint};
      matter_data_storage::instance().persist_data(endpoint_map_info);
    }
    
    // The bridge.matter_endpoint is then next available endpoint. The index
    // when registered will be added by FIXED_ENDPOINT_COUNT. Which is why we
    // substract it first.
    bridge.index = bridge.matter_endpoint - unify_ember_interface.emberAfFixedEndpointCountUnify();
    
    // Update or add the bridged endpoint
    erase_mapper_endpoint(node.unid, bridge.matter_endpoint);
    auto new_ep
      = bridged_endpoints.insert(make_pair(node.unid, std::move(bridge)));
    
    register_dynamic_endpoint(new_ep->second);
    invoke_listeners(new_ep->second, NODE_ADDED);
    //When the node is online we also invoke that the node state is reachable
    new_ep->second.reachable
      = (node.state == unify::zap_types::ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL);
    invoke_listeners(new_ep->second, update_t::NODE_STATE_CHANGED);
  }
} 

void matter_node_state_monitor::on_unify_node_removed(const std::string &unid)
{
  const auto &[start, end] = bridged_endpoints.equal_range(unid);
  for (auto ep = start; ep != end; ep++) {
    unify_ember_interface.emberAfClearDynamicEndpointUnify(ep->second.index);
    // delete the persisted endpoint map entry
    matter_data_storage::endpoint_mapping unify_node
      = {ep->second.unify_unid.c_str(), ep->second.unify_endpoint};
    matter_data_storage::instance().remove_persisted_data(unify_node);
    invoke_listeners(ep->second, update_t::NODE_DELETED);
  }
  bridged_endpoints.erase(start, end);
}

void matter_node_state_monitor::on_unify_node_state_changed(
  const node_state_monitor::node &node)
{
  const auto &[start, end] = bridged_endpoints.equal_range(node.unid);
  for (auto ep = start; ep != end; ep++) {
    ep->second.reachable
      = (node.state == unify::zap_types::ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL);
    invoke_listeners(ep->second, update_t::NODE_STATE_CHANGED);
  }
}

const bridged_endpoint *
  matter_node_state_monitor::bridged_endpoint(const std::string &unid,
                                              int epid) const
{
  const auto &[start, end] = bridged_endpoints.equal_range(unid);
  for (auto ep = start; ep != end; ep++) {
    if (ep->second.unify_endpoint == epid) {
      return &(ep->second);
    }
  }
  return nullptr;
}

const bridged_endpoint *matter_node_state_monitor::bridged_endpoint(
  chip::EndpointId matter_endpoint) const
{
  for (const auto &[key, value]: bridged_endpoints) {
    if (value.matter_endpoint == matter_endpoint) {
      return &value;
    }
  }
  return nullptr;
}

void matter_node_state_monitor::invoke_listeners(
  const struct bridged_endpoint &ep, update_t update) const
{
  for (const auto &listener: event_listeners) {
    listener(ep, update);
  }
}

void matter_node_state_monitor::register_event_listener(
  const event_listener_t &event_listener)
{
  event_listeners.push_back(event_listener);
}

void matter_node_state_monitor::erase_mapper_endpoint(const std::string unid, chip::EndpointId endpoint) 
{
  const auto &[start, end] = bridged_endpoints.equal_range(unid);
  for (auto ep = start; ep != end; ep++) {
    if (ep->second.matter_endpoint == endpoint) {
      bridged_endpoints.erase(ep);
      return;
    }
  }
}

void matter_node_state_monitor::display_map()
{
  sl_log_debug(LOG_TAG, "Unify Unid|\tUnify Endpoint|\tMatter Endpoint\n");
  for (auto ep = bridged_endpoints.begin(); ep != bridged_endpoints.end(); ep++) {
      sl_log_debug(LOG_TAG, "%s | %d | %d",
                   ep->second.unify_unid.c_str(), 
                   ep->second.unify_endpoint,
                   ep->second.matter_endpoint);
  }
}

void matter_node_state_monitor::register_dynamic_endpoint(const struct bridged_endpoint &bridge)
{
  uint16_t index = unify_ember_interface.emberAfGetDynamicIndexFromEndpointUnify(bridge.matter_endpoint);
  if (index != kEmberInvalidEndpointIndex) {
    chip::EndpointId ep = unify_ember_interface.emberAfClearDynamicEndpointUnify(index);
    if (ep != bridge.matter_endpoint) {
      sl_log_error(LOG_TAG, "The endpoint %d is not cleared from the dynamic endpoint index %d", bridge.matter_endpoint, index);
    }
    sl_log_debug(LOG_TAG, "Dynamic endpoint already exists for bridge endpoint %d. Cleared out the endpoint to replace it.", bridge.matter_endpoint);
  }
  
  EmberAfStatus status = unify_ember_interface.emberAfSetDynamicEndpointUnify(
    bridge.index,
    bridge.matter_endpoint,
    const_cast<EmberAfEndpointType *>(*bridge.ember_endpoint),
    bridge.ember_endpoint.data_version_span(),
    bridge.ember_endpoint.device_type_span(bridge.matter_type),
    1);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
        sl_log_error(
      LOG_TAG,
        "The unify node [%s] is not added on matter bridge as a dynamic endpoint",
      bridge.unify_unid.c_str());
  }
  if (status == EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED) {
    sl_log_error(LOG_TAG,
                "There are not sufficient space to add the unify node [%s] on "
                "matter fabric domain",
                bridge.unify_unid.c_str());
  }
}

std::set<chip::ClusterId> matter_node_state_monitor::get_supported_cluster(chip::EndpointId endpoint)
{
  std::set<chip::ClusterId> cluster_list;
  auto ep = bridged_endpoint(endpoint);
  auto iter                  = bridged_endpoints.find(ep->unify_unid);
  if (iter != bridged_endpoints.end()) {
      const EmberAfEndpointType *endpoint_meta_data = const_cast<EmberAfEndpointType*>(*(iter->second.ember_endpoint));
      for (uint8_t i = 0; i < endpoint_meta_data->clusterCount; i++){
          cluster_list.insert(endpoint_meta_data->cluster[i].clusterId);
      }
  }
  return cluster_list;
}

}  // namespace unify::matter_bridge
