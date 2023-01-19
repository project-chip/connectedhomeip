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

#include "group_translator.hpp"
#include "matter.h"
#include "matter_node_state_monitor.hpp"
#include "sl_status.h"
#include "matter_bridge_qrcode_publisher.hpp"

using namespace unify::matter_bridge; 
/**
 * @brief Initialize the CLI 
 * 
 * @return sl_status_t 
 */

sl_status_t matter_bridge_cli_init();
/**
 * @brief Set endpoint mapping and group mapping display instance;
 *        --> The node_state_monitor instance is for endpoint mapping display
 *        --> The group_translator instance is for group mapping display
 */
void set_mapping_display_instance(matter_node_state_monitor & n, group_translator & m);
void set_qr_code_publisher(QRCodePublisher & p);
