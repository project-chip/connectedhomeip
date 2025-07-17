/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_ble.h
  * @author  MCD Application Team
  * @brief   Header for ble application
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef APP_BLE_H
#define APP_BLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_compiler.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  APP_BLE_IDLE,
  APP_BLE_LP_CONNECTING,
  APP_BLE_CONNECTED_SERVER,
  APP_BLE_CONNECTED_CLIENT,
  APP_BLE_ADV_FAST,
  APP_BLE_ADV_LP,

} APP_BLE_ConnStatus_t;

/**
  * HCI Event Packet Types
  */
typedef __PACKED_STRUCT
{
  uint32_t *next;
  uint32_t *prev;
} BleEvtPacketHeader_t;

typedef __PACKED_STRUCT
{
  uint8_t   evtcode;
  uint8_t   plen;
  uint8_t   payload[1];
} BleEvt_t;

typedef __PACKED_STRUCT
{
  uint8_t   type;
  BleEvt_t  evt;
} BleEvtSerial_t;

typedef __PACKED_STRUCT __ALIGNED(4)
{
  BleEvtPacketHeader_t  header;
  BleEvtSerial_t        evtserial;
} BleEvtPacket_t;

typedef enum
{
  PROC_GAP_GEN_PHY_TOGGLE,
  PROC_GAP_GEN_CONN_TERMINATE,
  PROC_GATT_EXCHANGE_CONFIG,
  /* USER CODE BEGIN ProcGapGeneralId_t*/

  /* USER CODE END ProcGapGeneralId_t */
}ProcGapGeneralId_t;

typedef enum
{
  PROC_GAP_PERIPH_ADVERTISE_START_LP,
  PROC_GAP_PERIPH_ADVERTISE_START_FAST,
  PROC_GAP_PERIPH_ADVERTISE_STOP,
  PROC_GAP_PERIPH_ADVERTISE_DATA_UPDATE,
  PROC_GAP_PERIPH_CONN_PARAM_UPDATE,

  PROC_GAP_PERIPH_SET_BROADCAST_MODE,
  /* USER CODE BEGIN ProcGapPeripheralId_t */

  /* USER CODE END ProcGapPeripheralId_t */
}ProcGapPeripheralId_t;

typedef enum
{
  PROC_GAP_CENTRAL_SCAN_START,
  PROC_GAP_CENTRAL_SCAN_TERMINATE,
  /* USER CODE BEGIN ProcGapCentralId_t */

  /* USER CODE END ProcGapCentralId_t */
}ProcGapCentralId_t;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/** 
  * ST Manufacturer ID
**/
#define ST_MANUF_ID  0x30

/** 
  * BlueSTSDK Version
**/
enum
{
  BLUESTSDK_V1 =  0x01,
  BLUESTSDK_V2 =  0x02
};

/** 
  * BOARD ID 
**/
enum
{
  BOARD_ID_NUCLEO_WBA =  0x8B
};

/** 
  * FIRMWARE ID 
**/
enum
{
  FW_ID_P2P_SERVER =  0x83,
  FW_ID_P2P_ROUTER =  0x85,
  FW_ID_DT_SERVER  =  0x88,
  FW_ID_COC_PERIPH =  0x87,
  FW_ID_HEART_RATE =  0x89,
  FW_ID_HEALTH_THERMO = 0x8A
};
/* USER CODE END EC */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
#define SCAN_WIN_MS(x) ((uint16_t)((x)/0.625f))
#define SCAN_INT_MS(x) ((uint16_t)((x)/0.625f))
#define CONN_INT_MS(x) ((uint16_t)((x)/1.25f))
#define CONN_SUP_TIMEOUT_MS(x) ((uint16_t)((x)/10.0f))
#define CONN_CE_LENGTH_MS(x) ((uint16_t)((x)/0.625f))
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void APP_BLE_Init(void);
APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status(void);
void APP_BLE_Procedure_Gap_General(ProcGapGeneralId_t ProcGapGeneralId);
void APP_BLE_Procedure_Gap_Peripheral(ProcGapPeripheralId_t ProcGapPeripheralId);
/* USER CODE BEGIN EFP */
void APP_BLE_AdvStart(void);
void APP_BLE_AdvLowPower(void);
void APP_BLE_AdvStop(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*APP_BLE_H */
