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
#include "matter.h"
#include "matter_node_state_monitor.hpp"
#include "matter_cluster_interactor.hpp"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"
#include "matter_data_storage.hpp"
#include "sl_log.h"
static constexpr uint8_t ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL = 0;
constexpr const char *LOG_TAG = "matter_node_state_monitor";
namespace
{
using namespace unify::matter_bridge;

/**
 * @brief the first endpoint ids inside matter are reserved for fixed endpoints.
 * This function is able to return the next available endpoint which can be used
 * to declare a dynamic endpoint with.
 *
 * @return the next available EndpointId which can be used for 
 */
static chip::EndpointId get_next_ember_endpoint_id()
{
  static chip::EndpointId current_id = FIXED_ENDPOINT_COUNT;

  if (current_id >= MAX_ENDPOINT_COUNT) {
    return 0xffff;
  } else {
    return current_id++;
  }
}

/**
 * @brief registers a bridged_endpoint to matter.
 */
static void register_dynamic_endpoint(const bridged_endpoint &bridge)
{
  EmberAfStatus status = emberAfSetDynamicEndpoint(
    bridge.index,
    bridge.matter_endpoint,
    const_cast<EmberAfEndpointType *>(*bridge.ember_endpoint),
    bridge.ember_endpoint.data_version_span(),
    bridge.ember_endpoint.device_type_span(bridge.matter_type),
    0);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    sl_log_error(
      LOG_TAG,
      "The unify node [%s] is not added on matter bridge as a dynamic endpoint",
      bridge.unify_unid.c_str());
  }
  if (status == EMBER_ZCL_STATUS_INSUFFICIENT_SPACE) {
    sl_log_error(LOG_TAG,
                 "There are not sufficient space to add the unify node [%s] on "
                 "matter fabric domain",
                 bridge.unify_unid.c_str());
  }
}
}  // namespace

namespace unify::matter_bridge
{
matter_node_state_monitor::matter_node_state_monitor(
  const class device_translator &translator) :
  matter_device_translator(translator)
{
  // Disable last fixed endpoint, which is used as a placeholder for all of the
  // supported clusters so that ZAP will generated the requisite code.
  emberAfEndpointEnableDisable(
    emberAfEndpointFromIndex(emberAfFixedEndpointCount() - 1),
    false);

  unify::node_state_monitor::node_state_monitor::get_instance().set_interface(
    this);
}

void matter_node_state_monitor::on_unify_node_added(
  const unify::node_state_monitor::node &node)
{
  for (const auto &[ep_id, ep]: node.endpoints) {
    uint8_t count_number_of_clusters_matched_to_matter = 0;
    for (const auto &[cluster_name, cluster]: ep.clusters) {
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
      bridge.matter_endpoint = get_next_ember_endpoint_id();
      matter_data_storage::endpoint_mapping endpoint_map_info
        = {bridge.unify_unid.c_str(),
           bridge.unify_endpoint,
           bridge.matter_endpoint};
      matter_data_storage::instance().persist_data(endpoint_map_info);
    }
    bridge.index = bridge.matter_endpoint - emberAfFixedEndpointCount();
    auto new_ep
      = bridged_endpoints.insert(make_pair(node.unid, std::move(bridge)));
    register_dynamic_endpoint(new_ep->second);
    invoke_listeners(new_ep->second, NODE_ADDED);
    //When the node is online we also invoke that the node state is reachable
    new_ep->second.reachable
      = (node.state == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL);
    invoke_listeners(new_ep->second, update_t::NODE_STATE_CHANGED);
  }
}  // namespace unify::matter_bridge

void matter_node_state_monitor::on_unify_node_removed(const std::string &unid)
{
  const auto &[start, end] = bridged_endpoints.equal_range(unid);
  for (auto ep = start; ep != end; ep++) {
    emberAfClearDynamicEndpoint(ep->second.index);
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
      = (node.state == ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL);
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

}  // namespace unify::matter_bridge