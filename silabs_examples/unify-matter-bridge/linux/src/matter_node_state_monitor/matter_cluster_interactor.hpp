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
#ifndef MATTER_DEVICE_TRANSLATOR_INTERACTOR_HPP
#define MATTER_DEVICE_TRANSLATOR_INTERACTOR_HPP

#include "matter.h"
#include <string>
#include <vector>
#include <unordered_map>

#include "unify_node_state_monitor.hpp"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"
#include "cluster_emulator.hpp"

namespace unify::matter_bridge
{
// class device_translator;
// class matter_endpoint_builder;

class cluster_interactor
{
  public:
  cluster_interactor(
      ClusterEmulator &_emulator,
      const device_translator &_translator,
                     matter_endpoint_builder &_endpoint_builder);
                     
  std::optional<uint16_t> get_matter_type() const;

  void build_matter_cluster(
    const std::unordered_map<std::string, node_state_monitor::cluster>
      &clusters);

  matter_endpoint_builder &endpoint_builder;

  private:
  ClusterEmulator& emulator;
  const device_translator &translator;
};
}  // namespace unify::matter_bridge

#endif