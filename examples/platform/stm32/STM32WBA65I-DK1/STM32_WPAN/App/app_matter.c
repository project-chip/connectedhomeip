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

/* Includes ------------------------------------------------------------------*/
#include "app_matter.h"
#include "app_common.h"
#include "app_conf.h"
#include "app_entry.h"
#include "ble_core.h"
#include "cmsis_os2.h"
#include "custom_stm.h"
#include "dbg_trace.h"
#include "log_module.h"
#include "svc_ctl.h"
#include "uuid.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
    uint8_t Device_Led_Selection;
    uint8_t Led1;
} P2P_LedCharValue_t;

typedef struct
{
    uint8_t Device_Button_Selection;
    uint8_t ButtonStatus;
} P2P_ButtonCharValue_t;

typedef struct
{
    uint8_t Notification_Status; /* used to check if P2P Server is enabled to Notify */
    P2P_LedCharValue_t LedControl;
    P2P_ButtonCharValue_t ButtonControl;
    uint16_t ConnectionHandle;
} P2P_Server_App_Context_t;
/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/**
 * START of Section BLE_APP_CONTEXT
 */

static P2P_Server_App_Context_t P2P_Server_App_Context;

BLEReceiveCallback BLEReceiveCb                 = NULL;
BLETXCharCCCDWriteCallback BLETXCharCCCDWriteCb = NULL;
BLEConnectionCallback BLEConnectionCb           = NULL;
BLEDisconnectionCallback BLEDisconnectionCb     = NULL;
BLEDAckCallback BLEAckCb                        = NULL;

/**
 * END of Section BLE_APP_CONTEXT
 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

void APP_MATTER_BLE_Set_Receive_Callback(BLEReceiveCallback aCallback)
{
    BLEReceiveCb = aCallback;
}

void APP_MATTER_BLE_Set_TXCharCCCDWrite_Callback(BLETXCharCCCDWriteCallback aCallback)
{
    BLETXCharCCCDWriteCb = aCallback;
}

void APP_MATTER_BLE_Set_Connection_Callback(BLEConnectionCallback aCallback)
{
    BLEConnectionCb = aCallback;
}

void APP_MATTER_BLE_Set_Disconnection_Callback(BLEDisconnectionCallback aCallback)
{
    BLEDisconnectionCb = aCallback;
}

void APP_MATTER_BLE_Set_Ack_After_Indicate_Callback(BLEDAckCallback aCallback)
{
    BLEAckCb = aCallback;
}

/* Functions Definition ------------------------------------------------------*/
void APP_MATTER_Notification(MATTER_App_Notification_evt_t * pNotification)
{
    /* USER CODE BEGIN APP_MATTER_Notification */
    BLE_Matter_TXCharCCCD message;
    /* USER CODE END APP_MATTER_Notification */
    switch (pNotification->Evt_Opcode)
    {
        /* USER CODE BEGIN APP_MATTER_Notification */
        /* USER CODE END APP_MATTER_Notification */

    case MATTER_STM_CONN_HANDLE_EVT:
        /* USER CODE BEGIN PEER_CONN_HANDLE_EVT */
        BLEConnectionCb();
        /* USER CODE END PEER_CONN_HANDLE_EVT */
        break;

    case MATTER_STM_DISCON_HANDLE_EVT:
        /* USER CODE BEGIN PEER_DISCON_HANDLE_EVT */
        BLEDisconnectionCb(&pNotification->ConnectionHandle);
        /* USER CODE END PEER_DISCON_HANDLE_EVT */
        break;

    case MATTER_STM_ACK_INDICATE_EVT:
        /* USER CODE BEGIN PEER_DISCON_HANDLE_EVT */
        BLEAckCb(&pNotification->ConnectionHandle);
        /* USER CODE END PEER_DISCON_HANDLE_EVT */
        break;

    case MATTER_STM_INDICATE_ENABLED_EVT:
        /* USER CODE BEGIN P2PS_STM__NOTIFY_ENABLED_EVT */
        message.connid                             = pNotification->ConnectionHandle;
        message.notif                              = 1;
        P2P_Server_App_Context.Notification_Status = 1;
        LOG_INFO_APP("-- Matter APPLICATION SERVER : INDICATE ENABLED\n");
        LOG_INFO_APP(" \n\r");
        BLETXCharCCCDWriteCb(&message);
        /* USER CODE END P2PS_STM__NOTIFY_ENABLED_EVT */
        break;

    case MATTER_STM_INDICATE_DISABLED_EVT:
        /* USER CODE BEGIN P2PS_STM_NOTIFY_DISABLED_EVT */
        message.connid                             = pNotification->ConnectionHandle;
        message.notif                              = 0;
        P2P_Server_App_Context.Notification_Status = 0;
        LOG_INFO_APP("-- Matter APPLICATION SERVER : INDICATE DISABLED\n");
        LOG_INFO_APP(" \n\r");
        BLETXCharCCCDWriteCb(&message);
        /* USER CODE END P2PS_STM_NOTIFY_DISABLED_EVT */
        break;

    case MATTER_STM_WRITE_EVT:;
        BLE_Matter_RX Message;
        Message.Length  = pNotification->DataTransfered.Length;
        Message.Payload = pNotification->DataTransfered.pPayload;
        Message.connid  = pNotification->ConnectionHandle;
        BLEReceiveCb(&Message); // call matter callback
        /* USER CODE BEGIN MATTER_STM_WRITE_EVT */

        /* USER CODE END MATTER_STM_WRITE_EVT */
        break;

    default:
        /* USER CODE BEGIN APP_MATTER_Notification */

        /* USER CODE END APP_MATTER_Notification */
        break;
    }
    /* USER CODE BEGIN APP_MATTER_Notification */

    /* USER CODE END APP_MATTER_Notification */
    return;
}

void APP_MATTER_Init(void)
{
    /* USER CODE BEGIN APP_MATTER_Init */

    /**
     * Initialize LedButton Service
     */
    P2P_Server_App_Context.Notification_Status = 0;
    /* USER CODE END APP_MATTER_Init */
    return;
}

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/
void APP_MATTER_Send_Notification(uint16_t datalength, uint8_t * data)
{

    if (P2P_Server_App_Context.ButtonControl.ButtonStatus == 0x00)
    {
        P2P_Server_App_Context.ButtonControl.ButtonStatus = 0x01;
    }
    else
    {
        P2P_Server_App_Context.ButtonControl.ButtonStatus = 0x00;
    }

    if (P2P_Server_App_Context.Notification_Status)
    {
        CUSTOM_STM_App_Update_Char(P2P_NOTIFY_CHAR_UUID, (uint8_t *) data, datalength);
    }
    else
    {
        LOG_INFO_APP("-- Matter APPLICATION SERVER : CAN'T INFORM CLIENT -  NOTIFICATION DISABLED\n ");
    }

    return;
}

/* USER CODE END FD_LOCAL_FUNCTIONS*/
