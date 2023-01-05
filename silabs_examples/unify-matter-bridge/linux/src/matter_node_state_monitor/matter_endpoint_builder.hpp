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
#ifndef MATTER_ENDPOINT_BUILDER_HPP
#define MATTER_ENDPOINT_BUILDER_HPP

#include "matter_context.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace unify::matter_bridge {
class matter_cluster_builder
{
public:
    matter_cluster_builder(const std::function<void(const matter_cluster_builder &)> & on_done);
    ~matter_cluster_builder();

    std::vector<chip::CommandId> incoming_commands;
    std::vector<chip::CommandId> outgoing_commands;
    std::vector<EmberAfAttributeMetadata> attributes;

private:
    const std::function<void(const matter_cluster_builder &)> on_done;
};

/**
 * This class builds a dynamic matter endpoint context in a memory safe manner.
 * it makes use of 2 `builder<T>` classes, builder<chip::EmberAfCluster> and
 * builder<chip::EmberAfAttributeMetadata> in order to build matter attributes
 * and subsequently matter clusters.
 *
 * When all clusters and attributes are added, the final context attribute can
 * be retrieved by calling `build_and_get()`.
 *
 * Since a EmberAfEndpointType contains pointers to arrays of clusters and attributes,
 * we need to guarantee that these arrays can never life shorter as the EmberAfEndpointType
 * structure itself. The context object therefore holds lifetime references to these arrays
 * to satisfy this requirement. Only the whole context can be removed or added.
 * this way the lifetime of EmberAfEndpointType equals the life time of the arrays
 * its referencing.
 */
class matter_endpoint_builder : private builder<EmberAfCluster>
{
public:
    /**
     * @brief This function returns a builder object to append attributes for a
     * given Cluster. note that cluster will be registered when the builder object
     * goes out of scope or explicitly `build()` is called on it.
     */
    matter_cluster_builder register_cluster(const chip::ClusterId & cluster_id);

    /**
     * @brief finalize building dynamic matter endpoint and return the context
     * object. This context object is constant, meaning it can not be changed. Via
     * this way we know there cannot be tempored with its internal memory.
     */
    const matter_endpoint_context finalize();

    /**
     * @brief List of matter clusters that are registered in this endpoint.
     * 
     */
    std::vector<EmberAfCluster> clusters;

  private:
    matter_endpoint_context owned_ember_endpoint;
};
} // namespace unify::matter_bridge
#endif