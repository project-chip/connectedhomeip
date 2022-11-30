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


#include "sl_status.h"
#include "matter_node_state_monitor.hpp"
#include "matter.h"


using namespace unify::matter_bridge; 
/**
 * @brief Initailize the CLI 
 * 
 * @return sl_status_t 
 */

sl_status_t matter_bridge_cli_init();
/**
 * @brief Set the node_state_monitor for CLI, so that CLI can call node_state_monitor's display functions
 * 
 */
void set_matter_node_state_monitor_for_cli(matter_node_state_monitor &);
