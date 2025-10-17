/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_thread.h
 * @author  MCD Application Team
 * @version
 * @date
 * @brief   Header file for thread application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef APP_THREAD_H
#define APP_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cli.h"
#include "coap.h"
#include "tasklet.h"
#include "thread.h"

void Thread_Init(void);
void ProcessLinkLayer(void);
void ProcessTasklets(void);
void ProcessAlarm(void);
void ProcessUsAlarm(void);
void APP_THREAD_LockThreadStack(void);
bool APP_THREAD_TryLockThreadStack(void);
void APP_THREAD_UnLockThreadStack(void);
void ProcessOpenThreadTasklets(void);

void APP_THREAD_Init(void);
void APP_THREAD_ScheduleUART(void);
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode);
otInstance* APP_THREAD_GetotInstance(void);
/* ipv6-addressing defines        */
/*------------------------------------*/
/* Key Point: A major difference between FTDs and MTDs are that FTDs subscribe to the ff03::2 multicast address.
 * MTDs do not. */

#define MULICAST_FTD_MED            "ff03::1"
#define MULICAST_FTD_BORDER_ROUTER  "ff03::2"

#define APP_READ32_REG(base_addr) \
    (*(volatile uint32_t *)(base_addr))
#define APP_WRITE32_REG(base_addr, data) \
      (*(volatile uint32_t *)(base_addr) = (data))

/*
 *  List of all errors tracked by the Thread application
 *  running on M4. Some of these errors may be fatal
 *  or just warnings
 */
typedef enum
{
  ERR_THREAD_SET_STATE_CB,
  ERR_THREAD_SET_CHANNEL,
  ERR_THREAD_SET_PANID,
  ERR_THREAD_SET_THRESHOLD,
  ERR_THREAD_SET_MASTERKEY,
  ERR_THREAD_IPV6_ENABLE,
  ERR_THREAD_START,
  ERR_THREAD_ERASE_PERSISTENT_INFO,
  ERR_THREAD_SET_NETWORK_KEY,
/* USER CODE BEGIN ERROR_APPLI_ENUM */
  ERR_THREAD_COAP_START,
  ERR_THREAD_COAP_ADD_RESSOURCE,
  ERR_THREAD_MESSAGE_READ,
  ERR_THREAD_COAP_SEND_RESPONSE,
  ERR_THREAD_COAP_NEW_MSG,
  ERR_THREAD_COAP_APPEND,
  ERR_THREAD_COAP_SEND_REQUEST,
  ERR_THREAD_COAP_DATA_RESPONSE,
  ERR_THREAD_SETUP,
  ERR_THREAD_LINK_MODE,
  ERR_TIMER_INIT,
  ERR_TIMER_START,
  ERR_THREAD_MSG_COMPARE_FAILED,
  ERR_THREAD_COAP_ADDRESS_NOT_DEFINED,
/* USER CODE END ERROR_APPLI_ENUM */
  ERR_THREAD_CHECK_WIRELESS
  } ErrAppliIdEnum_t;

#ifdef __cplusplus
}
#endif

#endif /* APP_THREAD_H */
