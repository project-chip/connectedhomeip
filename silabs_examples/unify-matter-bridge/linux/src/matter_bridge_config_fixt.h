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
 * @file matter_bridge_config_fixt.h
 * @defgroup matter_bridge_config_fixture Matter Bridge Config Fixture
 * @ingroup matter_bridge_config
 * @brief Matter Bridge Configuration fixture.
 *
 * Fixture to initialize the matter_bridge_config from uic_main,
 * to be used in \ref matter_bridge_config
 *
 * @{
 */

#ifndef MATTER_BRIDGE_CONFIG_FIXT_H
#define MATTER_BRIDGE_CONFIG_FIXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
/**
 * @brief Fixture for setting up the matter_bridge_config component.
 *
 * This will read configurations from \ref config, and store it in matter_bridge_config.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t matter_bridge_config_fixt_setup(void);

#ifdef __cplusplus
}
#endif

/** @} end of matter_bridge_config */
#endif // MATTER_BRIDGE_CONFIG_FIXT_H
