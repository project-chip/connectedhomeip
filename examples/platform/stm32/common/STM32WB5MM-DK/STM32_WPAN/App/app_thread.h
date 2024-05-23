/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : App/app_thread.h
 * Description        : Header for Thread Application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
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
#ifndef APP_THREAD_H
#define APP_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include "stm32wbxx_core_interface_def.h"
#include "tl.h"
#include "tl_thread_hci.h"

/* OpenThread Library */
#include OPENTHREAD_CONFIG_FILE

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

/* Thread application generic defines */
/*------------------------------------*/
typedef enum
{
    APP_THREAD_LIMITED,
    APP_THREAD_FULL,
} APP_THREAD_InitMode_t;

/* ipv6-addressing defines        */
/*------------------------------------*/
/* Key Point: A major difference between FTDs and MTDs are that FTDs subscribe to the ff03::2 multicast address.
 * MTDs do not. */

#define MULICAST_FTD_MED "ff03::1"
#define MULICAST_FTD_BORDER_ROUTER "ff03::2"

/* Application errors                 */
/*------------------------------------*/

/*
 *  List of all errors tracked by the Thread application
 *  running on M4. Some of these errors may be fatal
 *  or just warnings
 */
typedef enum
{
    ERR_REC_MULTI_MSG_FROM_M0,
    ERR_THREAD_SET_STATE_CB,
    ERR_THREAD_SET_CHANNEL,
    ERR_THREAD_SET_PANID,
    ERR_THREAD_IPV6_ENABLE,
    ERR_THREAD_START,
    ERR_THREAD_ERASE_PERSISTENT_INFO,
    ERR_THREAD_SET_NETWORK_KEY,
    /* USER CODE BEGIN ERROR_APPLI_ENUM */
    ERR_THREAD_COAP_START,
    ERR_THREAD_COAP_ADD_RESSOURCE,
    ERR_THREAD_MESSAGE_READ,
    ERR_THREAD_COAP_SEND_RESPONSE,
    ERR_THREAD_COAP_APPEND,
    ERR_THREAD_COAP_SEND_REQUEST,
    ERR_THREAD_SETUP,
    ERR_THREAD_LINK_MODE,
    ERR_TIMER_INIT,
    ERR_TIMER_START,
    ERR_THREAD_COAP_NEW_MSG,
    ERR_THREAD_COAP_ADDRESS_NOT_DEFINED,
    ERR_THREAD_STOP,
    /* USER CODE END ERROR_APPLI_ENUM */
    ERR_THREAD_CHECK_WIRELESS
} ErrAppliIdEnum_t;
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ------------------------------------------------------- */
void APP_THREAD_Init_Dyn_1(void);
void APP_THREAD_Init_Dyn_2(void);
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode);
void APP_THREAD_RegisterCmdBuffer(TL_CmdPacket_t * p_buffer);
void APP_THREAD_ProcessMsgM0ToM4(void);
void APP_THREAD_TL_THREAD_INIT(void);

/* USER CODE BEGIN EF */

/* USER CODE END EF */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_THREAD_H */
