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

/**
 * @defgroup mpc_config_fixture MPC Config Fixture
 * @ingroup mpc_config
 * @brief MPC Configuration fixture.
 *
 * Fixture to initialize the mpc_config from uic_main,
 * to be used in @ref mpc_config
 *
 * @{
 */

#ifndef MPC_CONFIG_FIXT_H
#define MPC_CONFIG_FIXT_H

#include "sl_status.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Fixture for setting up the mpc_config component.
 *
 * This will read configurations from \ref config, and store it in mpc_config.
 *
 * @return SL_STATUS_OK for success, SL_STATUS_FAIL if an error occurred
 */
sl_status_t mpc_config_fixt_setup(void);
#ifdef __cplusplus
}
#endif
/** @} end of mpc_config */
#endif // MPC_CONFIG_FIXT_H
