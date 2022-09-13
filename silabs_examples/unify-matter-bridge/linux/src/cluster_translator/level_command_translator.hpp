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

/**
 * @defgroup level_command_translator
 * @brief Level command translator for matter interface
 *
 * Translates commands from matter to unify and unify to matter.
 *
 * @{
 */

#ifndef LEVEL_COMMAND_TRANSLATOR_HPP
#define LEVEL_COMMAND_TRANSLATOR_HPP

#pragma once
#include "matter.h"
#include "command_translator_interface.hpp"

namespace unify::matter_bridge
{
class LevelClusterCommandHandler : public command_translator_interface
{
  public:
  LevelClusterCommandHandler(
    const matter_node_state_monitor &node_state_monitor) :
    command_translator_interface(
      node_state_monitor, chip::app::Clusters::LevelControl::Id, "Groups")
  {}
  // CommandHandlerInterface
  void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext
                       &handlerContext) override;
};
}  // namespace unify::matter_bridge

#endif  //LEVEL_COMMAND_TRANSLATOR_HPP
        /** @} end level_command_translator */