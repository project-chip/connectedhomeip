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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_ENTRY_H
#define __APP_ENTRY_H

#include "stm32wbxx_hal.h"
#include "tl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
typedef struct
{
    uint8_t Pushed_Button;
    uint8_t State; // 1 pushed
} Push_Button_st;

typedef void (*PushButtonCallback)(Push_Button_st * aMessage);

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void APPE_Init(void);
void APP_ENTRY_RegisterCmdBuffer(TL_CmdPacket_t * p_buffer);
void APP_ENTRY_ProcessMsgM0ToM4(void);
void APP_ENTRY_Init_CFG_CLI_UART(void);
void APP_ENTRY_TL_THREAD_INIT(void);
void APP_ENTRY_PBSetReceiveCallback(PushButtonCallback aCallback);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ENTRY_H */
