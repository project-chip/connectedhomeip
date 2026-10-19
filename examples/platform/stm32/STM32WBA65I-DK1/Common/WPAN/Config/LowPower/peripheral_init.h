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

#ifndef PERIPHERAL_INIT_H
#define PERIPHERAL_INIT_H

#if (CFG_LPM_STANDBY_SUPPORTED == 1)
/**
 * @brief  Configure the SoC peripherals at Standby mode exit.
 * @param  None
 * @retval None
 */
void MX_StandbyExit_PeripheralInit(void);
#endif
#if (CFG_LPM_STOP2_SUPPORTED == 1)
/**
 * @brief  Configure the SoC peripherals at Stop2 mode exit.
 * @param  None
 * @retval None
 */
void MX_Stop2Exit_PeripheralInit(void);
#endif

#endif /* PERIPHERAL_INIT_H */
