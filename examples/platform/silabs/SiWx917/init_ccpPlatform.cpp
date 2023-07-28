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

#if 0 //common flash
using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Credentials;
#endif
//volatile int apperror_cnt;
//static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

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

#if 0 //common flash
void application_start(const void *unused)
{
  sl_status_t status;
  CHIP_ERROR err = CHIP_NO_ERROR;
  SILABS_LOG("Wireless init starting");
  if ((status = wfx_wifi_rsi_init()) != SL_STATUS_OK)
  {
      SILABS_LOG("wfx_wifi_start() failed: %s", status);
      return;
  }
  SILABS_LOG("WIFI init completed");
  if (SilabsMatterConfig::InitMatter(BLE_DEV_NAME) != CHIP_NO_ERROR)
  {
      appError(CHIP_ERROR_INTERNAL);
  }
  gExampleDeviceInfoProvider.SetStorageDelegate(&chip::Server::GetInstance().GetPersistentStorage());
  chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
  chip::DeviceLayer::PlatformMgr().LockChipStack();
#ifdef SILABS_ATTESTATION_CREDENTIALS
  SetDeviceAttestationCredentialsProvider(Silabs::GetSilabsDacProvider());
#else
  SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif
  chip::DeviceLayer::PlatformMgr().UnlockChipStack();
  SILABS_LOG("Starting App Task");
#ifndef WINDOWS_APP
  err = AppTask::GetAppTask().StartAppTask();
#else
  WindowApp & app = WindowApp::Instance();
  err = app.Init();
#endif
  if (err != CHIP_NO_ERROR)
  {
      appError(CHIP_ERROR_INTERNAL);
  }
  osThreadTerminate(main_Task);
}
void Create_application_task(void)
{
  main_Task = osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
  SILABS_LOG("Starting FreeRTOS scheduler");
  vTaskStartScheduler();
}
#endif