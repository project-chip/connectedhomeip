/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BLE_DBG_CONF_H
#define BLE_DBG_CONF_H

/**
 * Enable or Disable traces from BLE
 */

#define BLE_DBG_APP_EN 0

/**
 * Macro definition
 */
#if (BLE_DBG_APP_EN != 0)
#define BLE_DBG_APP_MSG PRINT_MESG_DBG
#else
#define BLE_DBG_APP_MSG PRINT_NO_MESG
#endif

#endif /*BLE_DBG_CONF_H */
