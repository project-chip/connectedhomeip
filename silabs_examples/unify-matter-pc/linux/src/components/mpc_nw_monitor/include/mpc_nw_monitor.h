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
 * @defgroup mpc_nw_monitor MPC network monitoring extension
 * @ingroup mpc_components
 *
 * @brief Add the MPC-specific fixtures to the Unify \ref system.
 *
 * This module is the source of all MPC-specific network monitoring.
 *
 * The after initialization the network monitor will begin.
 *
 * @{
 */

#ifndef MPC_MW_MONITOR_H
#define MPC_MW_MONITOR_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

sl_status_t mpc_nw_monitor_init(void);

#ifdef __cplusplus
}
#endif

#endif // MPC_MW_MONITOR_H
