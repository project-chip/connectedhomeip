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
#include "user_low_power_config.h"
#include "app_conf.h"
#include "stdint.h"
#include "stm32wbaxx.h"

/* Exported constants --------------------------------------------------------*/

/* Table of CPU peripheral registers to backup/restore when standby mode available */
const volatile uint32_t * const register_backup_table[] = {
    &(SCB->CPACR), /* FPU SCB->CPACR register */
    &(SCB->VTOR),  /* SCB->VTOR register */

    /* USER CODE BEGIN RBL */

    /* USER CODE END RBL */
};

/* Table of registers size */
const uint32_t register_backup_table_size = (sizeof(register_backup_table) / sizeof(register_backup_table[0]));

/* USER CODE BEGIN EC */

/* USER CODE END EC */
