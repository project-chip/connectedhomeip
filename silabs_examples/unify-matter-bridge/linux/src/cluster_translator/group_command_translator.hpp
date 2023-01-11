/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file on_off_command_translator.h
 * @ingroup components
 *
 * @brief  OnOff cluster command handler for matter interface
 *
 * @{
 */
#pragma once
#include "command_translator_interface.hpp"
#include "matter.h"

namespace unify::matter_bridge {
class GroupClusterCommandHandler : public command_translator_interface
{
public:
    GroupClusterCommandHandler(const matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                               group_translator & group_translator) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Groups::Id, "Groups", unify_mqtt, group_translator),
        m_group_translator(group_translator)
    {
        group_translator.register_unify_group_mqtt_message();
    }
    // CommandHandlerInterface
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & handlerContext) override;

private:
    void RemoveAllGroups(chip::FabricIndex fabric_index);
    group_translator & m_group_translator;
};

} // namespace unify::matter_bridge