/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License,
 * Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy
 * of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to
 * in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *
 * limitations under the License.
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
#define BLE_CFG_SVC_MAX_NBR_CB (7)
#define BLE_CFG_CLT_MAX_NBR_CB (0)

#define BLE_CFG_MAX_NBR_CB (0)

/* USER CODE BEGIN ble_conf_1 */

/* USER CODE END ble_conf_1 */

#endif /*BLE_CONF_H */
