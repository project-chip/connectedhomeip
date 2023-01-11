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

#ifndef MATTER_NODE_STATE_MONITOR_HPP
#define MATTER_NODE_STATE_MONITOR_HPP

#include "matter_context.hpp"
#include "unify_ember_interface.hpp"
#include "unify_node_state_monitor.hpp"
#include <map>
#include <optional>
#include <ostream>
#include <string>

namespace unify::matter_bridge {
class device_translator;
class ClusterEmulator;

/**
 * @brief definition of a bridged endpoint
 *
 * this holds all the information needed to map the
 * matter endpoints to a unify endpoint
 *
 */
struct bridged_endpoint
{
    /// the unid of the bridged endpoint
    ///
    std::string unify_unid;

    /// the unify endpoint number
    ///
    uint8_t unify_endpoint;

    /// the endpoint id assigned on the matter bridge
    ///
    chip::EndpointId matter_endpoint;

    /// the index that holds the matter endpoint in
    /// memory
    uint16_t index;

    /// Matter Device type of the endpoint
    ///
    uint16_t matter_type;

    /// the ember endpoint structure, this will contain
    /// the complete cluster list
    matter_endpoint_context ember_endpoint;

    /// Tells if the node is reachable, ie it is possible to communicate with the
    /// device it may be that the node is a non-listening device, so it could be
    /// that the communication latency is very high
    bool reachable;

    bridged_endpoint(matter_endpoint_context && context) : ember_endpoint(std::move(context)) {}
};

/**
 * @brief Matter node state monitor
 *
 * The matter nodestate monitor is responsible for using a unify_node_state_monitor
 * to check when new unify nodes has been added to-  or removed from- the network and
 * then crating a set of bridged endpoints.
 *
 * As this components is the first point of contact between the Matter world
 * and the unify world, the node state monitor also provides a translation between
 * matter endpoints ids and the unify (unid,epid) tuple.
 *
 */
class matter_node_state_monitor : private unify::node_state_monitor::node_state_monitor_interface
{
public:
    /**
     * @brief Construct a new matter node state monitor object
     *
     * @param matter_device_translator Device translator used to calculate the matter device type
     * @param cluster_emulator         Cluster Emulator used to extend the endpoint descriptor with emulated clusters
     * @param ember_interface          A class which wraps the ember interface for the node state monitor.
     */
    matter_node_state_monitor(const device_translator & matter_device_translator, ClusterEmulator & cluster_emulator,
                              UnifyEmberInterface & ember_interface);

    /**
     * @brief Get the bridged endpoint by object from unify addresses
     *
     * @param unid
     * @param epid
     * @return const bridged_endpoint&
     */
    const struct bridged_endpoint * bridged_endpoint(const std::string & unid, int epid) const;
    /**
     * @brief Get the unify endpoint address from a matter endpoint id
     *
     * @return std::pair<std::string unid, int epid>
     */
    const struct bridged_endpoint * bridged_endpoint(chip::EndpointId endpoint) const;

    enum update_t
    {
        NODE_ADDED,        //<<< A node has been added (or just found)
        NODE_DELETED,      ///<<< A node has been deleted
        NODE_STATE_CHANGED /// A node has changed its reachable state
    };

    /**
     * @brief Callback function for event notifications
     *
     */
    using event_listener_t = std::function<void(const struct bridged_endpoint &, update_t)>;

    /**
     * @brief Register an event listener with the node state monitor
     *
     * An event listener may be registered with the node state monitor allowing a user
     * to get notified when certain events occur.
     *
     * @param event_listener Callback function to be called
     */
    void register_event_listener(const event_listener_t & event_listener);

    /**
     * @brief Get the supported cluster from matter endpoint id
     *
     * @param endpoint
     * @return std::set<chip::ClusterId>
     */
    std::set<chip::ClusterId> get_supported_cluster(chip::EndpointId endpoint);

    /**
     * @brief Display mapping of Matter endpoints to Unify UNID/endpoints
     * @param std::ostream
     */
    void display_map(std::ostream & os);

    /**
     * @brief Convenience getter for the cluster emulator
     *
     * @return ClusterEmulator&
     */
    ClusterEmulator & emulator() const { return cluster_emulator; }

protected:
    /**
     * @brief called when a unify device is added
     *
     * When a device is added a number of matter endpoints are created. All
     * newly added endpints will get a matter endpoint id assigned.
     *
     * Then add endpints are created their existence is published on the matter
     * fabric. Using the function emberAfSetDynamicEndpoint
     *
     * @param node
     */
    void on_unify_node_added(const unify::node_state_monitor::node & node);

    /**
     * @brief Called when a Unify node is deleted.
     *
     * This will remove all endpoints belonging to a unid from mapped endpoint
     * list. The removal of the endpoint is announced on the matter fabric
     * Using emberAfClearDynamicEndpoint
     *
     * @param unid
     */
    void on_unify_node_removed(const std::string & unid);

    /**
     * @brief Called when a unify node changes state.
     *
     * When this function is called all endpoints belonging to a Unify node
     * changes its state The updated state must be published on the matter
     * fabric
     *
     * @param unid
     * @param state
     */
    void on_unify_node_state_changed(const node_state_monitor::node & node);

    /**
     * @brief Invoke all listeners
     *
     * Invokes all listeners for all endpoints of the node
     *
     * @param node
     */
    void invoke_listeners(const struct bridged_endpoint & ep, update_t update) const;
    /**
     * @brief construct a new bridged endpoint.
     *
     * this function used by the matter bridge to construct an instance of a
     * bridged endpoint. this function will query the unify node state monitor
     * about details regarding the endpoint. note all matter endpoints
     * has a device type, this is not the case for unify endpoints.
     *
     * in some cases we need to look at the capabilities of endpoint 0
     * to determine the actual device type.
     *
     * @param node
     * @return std::vector<bridged_endpoint>
     */
    std::vector<struct bridged_endpoint> new_bridged_endpoints(const unify::node_state_monitor::node & node);
    const device_translator & matter_device_translator;

    /**
     * @brief Erase matter endpoint in bridged endpoints if it exsits
     *
     */
    void erase_mapper_endpoint(const std::string unid, chip::EndpointId endpoint);

    /**
     * @brief registers a bridged_endpoint to matter.
     */
    void register_dynamic_endpoint(const struct bridged_endpoint & bridge);

    /** @brief map containing all bridged endpoints which are currently registered
    the node state monitor. If an entry is dropped from this list any associated
    resources which might be in use by matter will released as well
    */
    std::multimap<std::string, struct bridged_endpoint> bridged_endpoints;
    std::vector<event_listener_t> event_listeners;
    ClusterEmulator & cluster_emulator;
    UnifyEmberInterface & unify_ember_interface;
};
} // namespace unify::matter_bridge

#endif // MATTER_NODE_STATE_MONITOR_HPP
