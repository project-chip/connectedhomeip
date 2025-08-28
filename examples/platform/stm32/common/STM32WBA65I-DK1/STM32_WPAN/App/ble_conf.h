/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ble_conf.h
  * @author  MCD Application Team
  * @brief   Configuration file for BLE Middleware.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BLE_CONF_H
#define BLE_CONF_H

#include "app_conf.h"

/******************************************************************************
 *
 * Service Controller configuration
 *
 ******************************************************************************/

/**
 * There is one handler per BLE service
 * Note: There is no handler for the Device Information Service
 */
#define BLE_CFG_SVC_MAX_NBR_CB                    (7)
#define BLE_CFG_CLT_MAX_NBR_CB                    (0)

#define BLE_CFG_MAX_NBR_CB                        (0)

/* USER CODE BEGIN ble_conf_1 */

/* USER CODE END ble_conf_1 */

#endif /*BLE_CONF_H */
