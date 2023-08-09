/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef DOTDOT_MQTT_TO_MATTER_HPP
#define DOTDOT_MQTT_TO_MATTER_HPP

#include "sl_status.h"

sl_status_t mpc_on_off_cluster_mapper_init();
sl_status_t mpc_level_cluster_mapper_init();

#endif // DOTDOT_MQTT_TO_MATTER_HPP
