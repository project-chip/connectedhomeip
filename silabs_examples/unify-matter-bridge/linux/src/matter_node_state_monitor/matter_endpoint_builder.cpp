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
#include "matter_endpoint_builder.hpp"
#include "matter_device_translator.hpp"
#include "matter.h"
#include <memory>
#include<iostream>

#include "sl_log.h"

namespace
{
template<typename T>
std::shared_ptr<const std::vector<T>> make_shared(const std::vector<T> &data)
{
  return std::make_shared<const std::vector<T>>(data);
}
}  // namespace
namespace unify::matter_bridge
{
matter_cluster_builder::matter_cluster_builder(
  const std::function<void(const matter_cluster_builder &)> &on_done) :
  on_done(on_done)
{}

matter_cluster_builder::~matter_cluster_builder()
{
  on_done(*this);
}


matter_cluster_builder
  matter_endpoint_builder::register_cluster(const chip::ClusterId &cluster_id)

{
  // when we gathered all attributes we can fill in the final cluster struct.
  // this is done in this callback.
  auto on_done = [&](const matter_cluster_builder &cluster_attributes) {
    auto attributes        = make_shared(cluster_attributes.attributes);
    auto incoming_commands = make_shared(cluster_attributes.incoming_commands);
    auto outgoing_commands = make_shared(cluster_attributes.outgoing_commands);

    clusters.emplace_back(
      EmberAfCluster {cluster_id,
                      attributes.get()->data(),
                      static_cast<uint16_t>(attributes.get()->size()),
                      static_cast<uint16_t>(0),
                      static_cast<uint8_t>(CLUSTER_MASK_SERVER),
                      nullptr,
                      incoming_commands->data(),
                      outgoing_commands->data()});

    this->owned_ember_endpoint.add_lifetime(std::move(attributes));
    this->owned_ember_endpoint.add_lifetime(std::move(incoming_commands));
    this->owned_ember_endpoint.add_lifetime(std::move(outgoing_commands));
  };
  return matter_cluster_builder(on_done);
}

const matter_endpoint_context matter_endpoint_builder::finalize()
{
  auto ember_clusters                      = make_shared(clusters);
  this->owned_ember_endpoint->cluster      = ember_clusters->data();
  this->owned_ember_endpoint->clusterCount = ember_clusters->size();
  this->owned_ember_endpoint.add_lifetime(std::move(ember_clusters));
  return this->owned_ember_endpoint;
}

}  // namespace unify::matter_bridge