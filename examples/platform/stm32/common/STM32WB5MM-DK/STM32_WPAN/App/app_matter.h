/* USER CODE BEGIN */
/**
 ******************************************************************************
 * File Name          : App/app_matter.h
 * Description        : Header for p2p_server_app.c module
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
#ifndef __APP_MATTER_H
#define __APP_MATTER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

typedef enum
{
    MATTER_STM_PEER_CONN_HANDLE_EVT,
    MATTER_STM_PEER_DISCON_HANDLE_EVT,
    MATTER_STM_ACK_INDICATE_EVT,
    MATTER_STM_INDICATE_ENABLED_EVT,
    MATTER_STM_INDICATE_DISABLED_EVT,
    MATTER_STM_READ_EVT,
    MATTER_STM_WRITE_EVT,
    MATTER_STM_BOOT_REQUEST_EVT,
} MATTER_STM_Opcode_evt_t;

typedef struct
{
    uint8_t * pPayload;
    uint8_t Length;
} MATTER_STM_Data_t;

typedef struct
{
    MATTER_STM_Opcode_evt_t P2P_Evt_Opcode;
    MATTER_STM_Data_t DataTransfered;
    uint16_t ConnectionHandle;
    uint8_t ServiceInstance;
} MATTER_App_Notification_evt_t;

typedef struct
{
    uint8_t * Payload;
    uint16_t Length;
    uint16_t connid;
} BLE_Matter_RX;

typedef struct
{
    uint16_t connid;
    uint8_t notif;
} BLE_Matter_TXCharCCCD;

typedef void (*BLEReceiveCallback)(BLE_Matter_RX * aMessage);
typedef void (*BLETXCharCCCDWriteCallback)(BLE_Matter_TXCharCCCD * aMessage);
typedef void (*BLEConnectionCallback)(void);
typedef void (*BLEDisconnectionCallback)(uint16_t * connid);
typedef void (*BLEDAckCallback)(uint16_t * connid);

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

/* Exported functions ---------------------------------------------*/
/* USER CODE BEGIN EF */
void APP_MATTER_Init(void);
void APP_MATTER_Send_Notification(uint16_t datalength, uint8_t * data);
void APP_MATTER_Notification(MATTER_App_Notification_evt_t * pNotification);

void APP_MATTER_BLE_Set_Connection_Callback(BLEConnectionCallback aCallback);
void APP_MATTER_BLE_Set_Disconnection_Callback(BLEDisconnectionCallback aCallback);
void APP_MATTER_BLE_Set_Receive_Callback(BLEReceiveCallback aCallback);
void APP_MATTER_BLE_Set_TXCharCCCDWrite_Callback(BLETXCharCCCDWriteCallback aCallback);
void APP_MATTER_BLE_Set_Ack_After_Indicate_Callback(BLEDAckCallback aCallback);
/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /*__APP_MATTER_H */
