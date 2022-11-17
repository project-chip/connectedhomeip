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

#ifndef MOCK_NODE_STATE_MONITOR_HPP
#define MOCK_NODE_STATE_MONITOR_HPP

#include "matter_node_state_monitor.hpp"

namespace unify::matter_bridge {
namespace Test {

class MockNodeStateMonitor : public unify::matter_bridge::matter_node_state_monitor
{
public:
    MockNodeStateMonitor(const unify::matter_bridge::device_translator & device_translator, UnifyEmberInterface & ember_interface) :
        unify::matter_bridge::matter_node_state_monitor(device_translator, ember_interface)
    {}

    void call_on_unify_node_added(const unify::node_state_monitor::node & node) { this->on_unify_node_added(node); }
};

} // namespace Test
} // namespace unify::matter_bridge

#endif
