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

#include "command_translator.hpp"

namespace unify::matter_bridge {
namespace Test {
class MockClusterCommandHandler : public command_translator_interface
{
public:
    MockClusterCommandHandler(matter_node_state_monitor & node_state_monitor, UnifyMqtt & unify_mqtt,
                               group_translator & group_translator, device_translator& dev_translator) :
        command_translator_interface(node_state_monitor, chip::app::Clusters::Groups::Id, "Mock", unify_mqtt,
        group_translator, dev_translator),
        m_group_translator(group_translator), m_dev_translator(dev_translator)
    {
    }

    // CommandHandlerInterface
    void InvokeCommand(chip::app::CommandHandlerInterface::HandlerContext & handlerContext)
    {
        
    }

private:
    group_translator & m_group_translator;
    device_translator & m_dev_translator;
    std::vector<const char*> unify_cluster_names() const override { return std::vector<const char*>(); }
};
} // namespace Test
} // namespace unify::matter_bridge