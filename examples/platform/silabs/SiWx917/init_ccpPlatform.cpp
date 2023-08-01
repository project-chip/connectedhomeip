/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppConfig.h"
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

//#include "main_task.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "init_ccpPlatform.h"
#include "rsi_pll.h"
#include "rsi_rom_clks.h"
#include "sl_system_init.h"
#include "sli_siwx917_soc.h"
#include "em_core.h"
#include <assert.h>
#include <mbedtls/platform.h>
#include <string.h>

void initAntenna(void);

/* GPIO button config */
void soc_pll_config(void);
void RSI_Wakeupsw_config(void);
void RSI_Wakeupsw_config_gpio0(void);

void init_ccpPlatform(void)
{
    sl_system_init();
    NVIC_SetPriority(SVCall_IRQn, CORE_INTERRUPT_HIGHEST_PRIORITY);

    soc_pll_config();
    RSI_Wakeupsw_config();

    RSI_Wakeupsw_config_gpio0();
#if SILABS_LOG_ENABLED
    silabsInitLog();
#endif
}

#ifdef __cplusplus
}
#endif
