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
 * @defgroup identify_command_translator
 * @brief Identify cluster command translator for matter interface
 *
 * Translate Identify cluster commands from the matter protocol to unify
 * dotdot data model.
 *
 * @{
 */

#ifndef IDENTIFY_COMMAND_TRANSLATOR_HPP
#define IDENTIFY_COMMAND_TRANSLATOR_HPP

#pragma once
#include "command_translator_interface.hpp"

namespace unify::matter_bridge
{
class IdentifyClusterCommandHandler : public command_translator_interface
{
  public:
  IdentifyClusterCommandHandler(
    const matter_node_state_monitor &node_state_monitor) :
    command_translator_interface(node_state_monitor,
                                 chip::app::Clusters::Identify::Id,"Identify")
  {}
  void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext
                       &HandlerContext) override;
};
}  // namespace unify::matter_bridge

#endif  //IDENTIFY_COMMAND_TRANSLATOR_HPP
/** @} end identify_command_translator */
