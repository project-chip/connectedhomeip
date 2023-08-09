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
 * @defgroup mpc_application_monitoring MPC Application Monitoring
 * @ingroup mpc_components
 * @brief Initializes the mpc Application Monitoring
 *
 * @{
 */

#ifndef MPC_APPLICATION_MONITORING_H
#define MPC_APPLICATION_MONITORING_H

#include "sl_status.h"

// Name of the mpc application used for Application Monitoring
#define MPC_APPLICATION_NAME "MPC (Matter Protocol Controller)"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the ApplicationMonitoring resources for the mpc.
 *
 * @return SL_STATUS_OK on success, any other value in case of error.
 */
sl_status_t mpc_application_monitoring_init();

#ifdef __cplusplus
}
#endif

#endif // MPC_APPLICATION_MONITORING_H
/** @} end mpc_application_monitoring */
