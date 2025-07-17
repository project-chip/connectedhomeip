/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_ble.c
 * @author  MCD Application Team
 * @brief   BLE Application
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "ble.h"
#include "app_ble.h"
#include "cmsis_os2.h"
#include "host_stack_if.h"
#include "ll_sys_if.h"
#include "ll_intf.h"
#include "stm32_rtos.h"
#include "otp.h"
#include "stm32_timer.h"
#include "stm_list.h"
#include "advanced_memory_manager.h"
#include "blestack.h"
#include "nvm.h"
#include "simple_nvm_arbiter.h"
#include "custom_stm.h"
#include "app_matter.h"
#if (BLE_RADIO_ACTIVITY_ON_LED_SUPPORT != 0)

#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Security parameters structure */
typedef struct {
	/* IO capability of the device */
	uint8_t ioCapability;

	/**
	 * Authentication requirement of the device
	 * Man In the Middle protection required?
	 */
	uint8_t mitm_mode;

	/* Bonding mode of the device */
	uint8_t bonding_mode;

	/**
	 * this variable indicates whether to use a fixed pin
	 * during the pairing process or a passkey has to be
	 * requested to the application during the pairing process
	 * 0 implies use fixed pin and 1 implies request for passkey
	 */
	uint8_t Use_Fixed_Pin;

	/* Minimum encryption key size requirement */
	uint8_t encryptionKeySizeMin;

	/* Maximum encryption key size requirement */
	uint8_t encryptionKeySizeMax;

	/**
	 * fixed pin to be used in the pairing process if
	 * Use_Fixed_Pin is set to 1
	 */
	uint32_t Fixed_Pin;

	/**
	 * this flag indicates whether the host has to initiate
	 * the security, wait for pairing or does not have any security
	 * requirements.
	 * 0x00 : no security required
	 * 0x01 : host should initiate security by sending the slave security
	 *        request command
	 * 0x02 : host need not send the clave security request but it
	 * has to wait for paiirng to complete before doing any other
	 * processing
	 */
	uint8_t initiateSecurity;
/* USER CODE BEGIN tSecurityParams*/

/* USER CODE END tSecurityParams */
} SecurityParams_t;

/* Global context contains all BLE common variables. */
typedef struct {
	/* Security requirements of the host */
	SecurityParams_t bleSecurityParam;

	/* GAP service handle */
	uint16_t gapServiceHandle;

	/* Device name characteristic handle */
	uint16_t devNameCharHandle;

	/* Appearance characteristic handle */
	uint16_t appearanceCharHandle;

	/**
	 * connection handle of the current active connection
	 * When not in connection, the handle is set to 0xFFFF
	 */
	uint16_t connectionHandle;

/* USER CODE BEGIN BleGlobalContext_t*/

/* USER CODE END BleGlobalContext_t */
} BleGlobalContext_t;

typedef struct {
	BleGlobalContext_t BleApplicationContext_legacy;
	APP_BLE_ConnStatus_t Device_Connection_Status;
	/* USER CODE BEGIN PTD_1*/
	uint8_t connIntervalFlag;
/* USER CODE END PTD_1 */
} BleApplicationContext_t;

/* Private defines -----------------------------------------------------------*/
/* GATT buffer size (in bytes)*/
#define BLE_GATT_BUF_SIZE \
          BLE_TOTAL_BUFFER_SIZE_GATT(CFG_BLE_NUM_GATT_ATTRIBUTES, \
                                     CFG_BLE_NUM_GATT_SERVICES, \
                                     CFG_BLE_ATT_VALUE_ARRAY_SIZE)

#define MBLOCK_COUNT              (BLE_MBLOCKS_CALC(PREP_WRITE_LIST_SIZE, \
                                                    CFG_BLE_ATT_MTU_MAX, \
                                                    CFG_BLE_NUM_LINK) \
                                   + CFG_BLE_MBLOCK_COUNT_MARGIN)

#define BLE_DYN_ALLOC_SIZE \
        (BLE_TOTAL_BUFFER_SIZE(CFG_BLE_NUM_LINK, MBLOCK_COUNT))

/* USER CODE BEGIN PD */
#define ADV_TIMEOUT_MS                 (60 * 1000)

#if (BLE_RADIO_ACTIVITY_ON_LED_SUPPORT != 0)
#define LED_ON_TIMEOUT                 (500)
#endif          
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static tListNode BleAsynchEventQueue;

static const uint8_t a_MBdAddr[BD_ADDR_SIZE] = {
        (uint8_t) ((CFG_BD_ADDRESS & 0x0000000000FF)),
		(uint8_t) ((CFG_BD_ADDRESS & 0x00000000FF00) >> 8),
		(uint8_t) ((CFG_BD_ADDRESS & 0x000000FF0000) >> 16),
		(uint8_t) ((CFG_BD_ADDRESS & 0x0000FF000000) >> 24),
		(uint8_t) ((CFG_BD_ADDRESS & 0x00FF00000000) >> 32),
		(uint8_t) ((CFG_BD_ADDRESS & 0xFF0000000000) >> 40) };

static uint8_t a_BdAddrUdn[BD_ADDR_SIZE];

/* Identity root key used to derive IRK and DHK(Legacy) */
static const uint8_t a_BLE_CfgIrValue[16] = CFG_BLE_IR;

/* Encryption root key used to derive LTK(Legacy) and CSRK */
static const uint8_t a_BLE_CfgErValue[16] = CFG_BLE_ER;
static BleApplicationContext_t bleAppContext;
MATTER_App_Notification_evt_t handleNotification;

static const uint8_t a_GapDeviceName[] = { 'S', 'T', '_', 'W', 'B', 'A', '_',
		'M', 'A', 'T', 'T', 'E', 'R' }; /* Gap Device Name */

/* Advertising Data */
// ADV for matter
uint8_t a_AdvData[15] = { 0x02, 0x01, 0x06, 0x0B, 0x16, 0xF6, 0xFF, 0x00, 0x00, 0x0F, 0xF1, 0xFF, 0x04, 0x80, 0x00, };

uint64_t buffer_nvm[CFG_BLEPLAT_NVM_MAX_SIZE] = { 0 };

static AMM_VirtualMemoryCallbackFunction_t APP_BLE_ResumeFlowProcessCb;

/* Host stack init variables */
static uint32_t buffer[DIVC(BLE_DYN_ALLOC_SIZE, 4)];
static uint32_t gatt_buffer[DIVC(BLE_GATT_BUF_SIZE, 4)];
static BleStack_init_t pInitParams;

/* BLE_STACK_TASK related resources */
osSemaphoreId_t BleStackSemaphore;
osThreadId_t BleStackThread;

osThreadId_t HCIasyncEvtThread;

osSemaphoreId_t ProcGapCompleteSemaphore;
/* HCI_ASYNC_EVT_TASK related resources */
osSemaphoreId_t HCIasyncEvtSemaphore;

osTimerId_t SwitchOffLedTimerId;

const osThreadAttr_t stBleStackTaskAttributes = {
		.name = "BLE Stack Task",
		.priority = CFG_TASK_PRIO_BLE_STACK,
		.stack_size = TASK_BLE_STACK_STACK_SIZE, };

const osThreadAttr_t stHCIasyncEvtTaskAttributes = {
		.name =	"HCI asynch event Task",
		.priority = CFG_TASK_PRIO_HCI_ASYNCH_EVT,
		.stack_size = TASK_HCI_ASYNCH_EVT_STACK_SIZE };

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private function prototypes -----------------------------------------------*/
static void BleStack_Process_BG(void);
static void Ble_UserEvtRx(void);
static void BLE_ResumeFlowProcessCallback(void);
static void Ble_Hci_Gap_Gatt_Init(void);
static const uint8_t* BleGetBdAddress(void);
static void gap_cmd_resp_wait(void);
static void gap_cmd_resp_release(void);
static void BLE_NvmCallback(SNVMA_Callback_Status_t);
static uint8_t HOST_BLE_Init(void);
static void BleUserEvtRx_Task_Entry(void *thread_input);
static void BleStack_Task_Entry(void *thread_input);
/* USER CODE BEGIN PFP */
static void fill_advData(uint8_t *p_adv_data, uint8_t tab_size,
		const uint8_t *p_bd_addr);
#if (BLE_RADIO_ACTIVITY_ON_LED_SUPPORT != 0)
static void Switch_OFF_Led(void *arg);
#endif
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/
void APP_BLE_Init(void) {
	/* USER CODE BEGIN APP_BLE_Init_1 */
#if (BLE_RADIO_ACTIVITY_ON_LED_SUPPORT != 0)
	tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
#endif
	/* USER CODE END APP_BLE_Init_1 */

	LST_init_head(&BleAsynchEventQueue);

	const osSemaphoreAttr_t BleStackSemaphore_attributes = {
			.name =	"BLE stack Semaphore" };

	BleStackSemaphore = osSemaphoreNew(1U, 0U, &BleStackSemaphore_attributes);

	if (BleStackSemaphore == NULL) {
		Error_Handler();
	}

	BleStackThread = osThreadNew(BleStack_Task_Entry, NULL,
			&stBleStackTaskAttributes);

	if (BleStackThread == NULL) {
		Error_Handler();
	}

	const osSemaphoreAttr_t HCIasyncEvtSemaphore_attributes = {
			.name =	"HCI asynch EVT Semaphore" };

	HCIasyncEvtSemaphore = osSemaphoreNew(1U, 0U,
			&HCIasyncEvtSemaphore_attributes);

	if (HCIasyncEvtSemaphore == NULL) {
		Error_Handler();
	}

	HCIasyncEvtThread = osThreadNew(BleUserEvtRx_Task_Entry, NULL,
			&stHCIasyncEvtTaskAttributes);

	if (HCIasyncEvtThread == NULL) {
		Error_Handler();
	}

	/* NVM emulation in RAM initialization */
	NVM_Init(buffer_nvm, 0, CFG_BLEPLAT_NVM_MAX_SIZE);

	/* First register the APP BLE buffer */
	SNVMA_Register(APP_BLE_NvmBuffer, (uint32_t*) buffer_nvm,
			(CFG_BLEPLAT_NVM_MAX_SIZE * 2));

	/* Realize a restore */
	SNVMA_Restore(APP_BLE_NvmBuffer);
	/* USER CODE BEGIN APP_BLE_Init_Buffers */

	/* USER CODE END APP_BLE_Init_Buffers */

	/* Check consistency */
	if (NVM_Get(NVM_FIRST, 0xFF, 0, 0, 0) != NVM_EOF) {
		NVM_Discard(NVM_ALL);
	}

	/* Initialize the BLE Host */
	if (HOST_BLE_Init() == 0u) {
		/* Initialization of HCI & GATT & GAP layer */
		Ble_Hci_Gap_Gatt_Init();

		const osSemaphoreAttr_t ProcGapCompleteSemaphore_attributes = {
				.name =	"ProcGapCompleteSemaphore" };

		ProcGapCompleteSemaphore = osSemaphoreNew(1U, 0U,
				&ProcGapCompleteSemaphore_attributes);
		if (ProcGapCompleteSemaphore == NULL) {
			Error_Handler();
		}

		/* Initialization of the BLE Services */
		SVCCTL_Init();

		/* Initialization of the BLE App Context */
		bleAppContext.Device_Connection_Status = APP_BLE_IDLE;
		bleAppContext.BleApplicationContext_legacy.connectionHandle = 0xFFFF;

		/* From here, all initialization are BLE application specific */

		/* USER CODE BEGIN APP_BLE_Init_4 */

		/* USER CODE END APP_BLE_Init_4 */

		SVCCTL_InitCustomSvc();
		/* USER CODE BEGIN APP_BLE_Init_3 */
#if (BLE_RADIO_ACTIVITY_ON_LED_SUPPORT != 0)
		ret = aci_hal_set_radio_activity_mask(0x0006);
		if (ret != BLE_STATUS_SUCCESS) {
			APP_DBG_MSG("  Fail   : aci_hal_set_radio_activity_mask command, result: 0x%2X\n", ret);
		} else {
			APP_DBG_MSG("  Success: aci_hal_set_radio_activity_mask command\n\r");
		}

		SwitchOffLedTimerId = osTimerNew(Switch_OFF_Led, osTimerOnce, NULL,
				NULL);
#endif

		/* USER CODE END APP_BLE_Init_3 */

	}
	/* USER CODE BEGIN APP_BLE_Init_2 */
	bleAppContext.connIntervalFlag = 0;
	/* USER CODE END APP_BLE_Init_2 */

	/**
	 * Initialize Matter Application
	 */
	APP_MATTER_Init();

	return;
}

SVCCTL_UserEvtFlowStatus_t SVCCTL_App_Notification(void *p_Pckt) {
	tBleStatus ret = BLE_STATUS_ERROR;
	hci_event_pckt *p_event_pckt;
	evt_le_meta_event *p_meta_evt;
	evt_blecore_aci *p_blecore_evt;

	p_event_pckt = (hci_event_pckt*) ((hci_uart_pckt*) p_Pckt)->data;
	UNUSED(ret);
	/* USER CODE BEGIN SVCCTL_App_Notification */

	/* USER CODE END SVCCTL_App_Notification */

	switch (p_event_pckt->evt) {
	case HCI_DISCONNECTION_COMPLETE_EVT_CODE: {
		hci_disconnection_complete_event_rp0 *p_disconnection_complete_event;
		p_disconnection_complete_event =
				(hci_disconnection_complete_event_rp0*) p_event_pckt->data;
		if (p_disconnection_complete_event->Connection_Handle
				== bleAppContext.BleApplicationContext_legacy.connectionHandle) {
			bleAppContext.BleApplicationContext_legacy.connectionHandle = 0;
			bleAppContext.Device_Connection_Status = APP_BLE_IDLE;
			LOG_INFO_APP(">>== HCI_DISCONNECTION_COMPLETE_EVT_CODE\n");
			LOG_INFO_APP(
					"     - Connection Handle:   0x%02X\n     - Reason:    0x%02X\n",
					p_disconnection_complete_event->Connection_Handle,
					p_disconnection_complete_event->Reason);

			/* USER CODE BEGIN EVT_DISCONN_COMPLETE_2 */

			/* USER CODE END EVT_DISCONN_COMPLETE_2 */
		}
		gap_cmd_resp_release();

		/* USER CODE BEGIN EVT_DISCONN_COMPLETE_1 */

		/* USER CODE END EVT_DISCONN_COMPLETE_1 */
		handleNotification.Evt_Opcode = MATTER_STM_DISCON_HANDLE_EVT;
		handleNotification.ConnectionHandle =
				p_disconnection_complete_event->Connection_Handle;
		APP_MATTER_Notification(&handleNotification);

		/* USER CODE BEGIN EVT_DISCONN_COMPLETE */
		/* USER CODE END EVT_DISCONN_COMPLETE */
		break; /* HCI_DISCONNECTION_COMPLETE_EVT_CODE */
	}

	case HCI_LE_META_EVT_CODE: {
		p_meta_evt = (evt_le_meta_event*) p_event_pckt->data;
		/* USER CODE BEGIN EVT_LE_META_EVENT */

		/* USER CODE END EVT_LE_META_EVENT */
		switch (p_meta_evt->subevent) {
		case HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE: {
			uint16_t conn_interval_us = 0;
			hci_le_connection_update_complete_event_rp0 *p_conn_update_complete;
			p_conn_update_complete =
					(hci_le_connection_update_complete_event_rp0*) p_meta_evt->data;
			conn_interval_us = p_conn_update_complete->Conn_Interval * 1250;
			LOG_INFO_APP(
					">>== HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE\n");
			LOG_INFO_APP(
					"     - Connection Interval:   %d.%02d ms\n     - Connection latency:    %d\n     - Supervision Timeout:   %d ms\n",
					conn_interval_us / 1000, (conn_interval_us % 1000) / 10,
					p_conn_update_complete->Conn_Latency,
					p_conn_update_complete->Supervision_Timeout * 10);
			UNUSED(p_conn_update_complete);

			/* USER CODE BEGIN EVT_LE_CONN_UPDATE_COMPLETE */

			/* USER CODE END EVT_LE_CONN_UPDATE_COMPLETE */
			break;
		}
		case HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE: {
			hci_le_phy_update_complete_event_rp0 *p_le_phy_update_complete;
			p_le_phy_update_complete =
					(hci_le_phy_update_complete_event_rp0*) p_meta_evt->data;
			UNUSED(p_le_phy_update_complete);

			gap_cmd_resp_release();

			/* USER CODE BEGIN EVT_LE_PHY_UPDATE_COMPLETE */

			/* USER CODE END EVT_LE_PHY_UPDATE_COMPLETE */
			break;
		}
		case HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE: {
			uint16_t conn_interval_us = 0;
			hci_le_enhanced_connection_complete_event_rp0 *p_enhanced_conn_complete;
			p_enhanced_conn_complete =
					(hci_le_enhanced_connection_complete_event_rp0*) p_meta_evt->data;
			conn_interval_us = p_enhanced_conn_complete->Conn_Interval * 1250;
			LOG_INFO_APP(
					">>== HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE - Connection handle: 0x%04X\n",
					p_enhanced_conn_complete->Connection_Handle);
			LOG_INFO_APP(
					"     - Connection established with @:%02x:%02x:%02x:%02x:%02x:%02x\n",
					p_enhanced_conn_complete->Peer_Address[5],
					p_enhanced_conn_complete->Peer_Address[4],
					p_enhanced_conn_complete->Peer_Address[3],
					p_enhanced_conn_complete->Peer_Address[2],
					p_enhanced_conn_complete->Peer_Address[1],
					p_enhanced_conn_complete->Peer_Address[0]);
			LOG_INFO_APP(
					"     - Connection Interval:   %d.%02d ms\n     - Connection latency:    %d\n     - Supervision Timeout:   %d ms\n",
					conn_interval_us / 1000, (conn_interval_us % 1000) / 10,
					p_enhanced_conn_complete->Conn_Latency,
					p_enhanced_conn_complete->Supervision_Timeout * 10);

			if (bleAppContext.Device_Connection_Status
					== APP_BLE_LP_CONNECTING) {
				/* Connection as client */
				bleAppContext.Device_Connection_Status =
						APP_BLE_CONNECTED_CLIENT;
			} else {
				/* Connection as server */
				bleAppContext.Device_Connection_Status =
						APP_BLE_CONNECTED_SERVER;
			}
			bleAppContext.BleApplicationContext_legacy.connectionHandle =
					p_enhanced_conn_complete->Connection_Handle;

			/* USER CODE BEGIN HCI_EVT_LE_ENHANCED_CONN_COMPLETE */

			/* USER CODE END HCI_EVT_LE_ENHANCED_CONN_COMPLETE */
			break; /* HCI_LE_ENHANCED_CONNECTION_COMPLETE_SUBEVT_CODE */
		}
		case HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE: {
			uint16_t conn_interval_us = 0;
			hci_le_connection_complete_event_rp0 *p_conn_complete;
			p_conn_complete =
					(hci_le_connection_complete_event_rp0*) p_meta_evt->data;
			conn_interval_us = p_conn_complete->Conn_Interval * 1250;
			LOG_INFO_APP(
					">>== HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE - Connection handle: 0x%04X\n",
					p_conn_complete->Connection_Handle);
			LOG_INFO_APP(
					"     - Connection established with @:%02x:%02x:%02x:%02x:%02x:%02x\n",
					p_conn_complete->Peer_Address[5],
					p_conn_complete->Peer_Address[4],
					p_conn_complete->Peer_Address[3],
					p_conn_complete->Peer_Address[2],
					p_conn_complete->Peer_Address[1],
					p_conn_complete->Peer_Address[0]);
			LOG_INFO_APP(
					"     - Connection Interval:   %d.%02d ms\n     - Connection latency:    %d\n     - Supervision Timeout:   %d ms\n",
					conn_interval_us / 1000, (conn_interval_us % 1000) / 10,
					p_conn_complete->Conn_Latency,
					p_conn_complete->Supervision_Timeout * 10);

			if (bleAppContext.Device_Connection_Status
					== APP_BLE_LP_CONNECTING) {
				/* Connection as client */
				bleAppContext.Device_Connection_Status =
						APP_BLE_CONNECTED_CLIENT;
			} else {
				/* Connection as server */
				bleAppContext.Device_Connection_Status =
						APP_BLE_CONNECTED_SERVER;
			}
			bleAppContext.BleApplicationContext_legacy.connectionHandle =
					p_conn_complete->Connection_Handle;

			hci_le_set_data_length(
					bleAppContext.BleApplicationContext_legacy.connectionHandle,
					251, 2120);
			handleNotification.Evt_Opcode = MATTER_STM_CONN_HANDLE_EVT;
			handleNotification.ConnectionHandle =
					bleAppContext.BleApplicationContext_legacy.connectionHandle;
			APP_MATTER_Notification(&handleNotification);

			/* USER CODE BEGIN HCI_EVT_LE_CONN_COMPLETE */
			
			/* USER CODE END HCI_EVT_LE_CONN_COMPLETE */
			break; /* HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE */
		}
			/* USER CODE BEGIN SUBEVENT */

			/* USER CODE END SUBEVENT */
		default: {
			/* USER CODE BEGIN SUBEVENT_DEFAULT */

			/* USER CODE END SUBEVENT_DEFAULT */
			break;
		}
		}

		/* USER CODE BEGIN META_EVT */

		/* USER CODE END META_EVT */
	}
		break; /* HCI_LE_META_EVT_CODE */

	case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE: {
		p_blecore_evt = (evt_blecore_aci*) p_event_pckt->data;
		/* USER CODE BEGIN EVT_VENDOR */

		/* USER CODE END EVT_VENDOR */
		switch (p_blecore_evt->ecode) {
		/* USER CODE BEGIN ECODE */

		/* USER CODE END ECODE */
		case ACI_L2CAP_CONNECTION_UPDATE_RESP_VSEVT_CODE: {
			aci_l2cap_connection_update_resp_event_rp0 *p_l2cap_conn_update_resp;
			p_l2cap_conn_update_resp =
					(aci_l2cap_connection_update_resp_event_rp0*) p_blecore_evt->data;
			UNUSED(p_l2cap_conn_update_resp);
			/* USER CODE BEGIN EVT_L2CAP_CONNECTION_UPDATE_RESP */

			/* USER CODE END EVT_L2CAP_CONNECTION_UPDATE_RESP */
			break;
		}
		case ACI_GAP_PROC_COMPLETE_VSEVT_CODE: {
			aci_gap_proc_complete_event_rp0 *p_gap_proc_complete;
			p_gap_proc_complete =
					(aci_gap_proc_complete_event_rp0*) p_blecore_evt->data;
			UNUSED(p_gap_proc_complete);

			LOG_INFO_APP(">>== ACI_GAP_PROC_COMPLETE_VSEVT_CODE\n");
			/* USER CODE BEGIN EVT_GAP_PROCEDURE_COMPLETE */

			/* USER CODE END EVT_GAP_PROCEDURE_COMPLETE */
			break; /* ACI_GAP_PROC_COMPLETE_VSEVT_CODE */
		}
		case ACI_HAL_END_OF_RADIO_ACTIVITY_VSEVT_CODE: {
			/* USER CODE BEGIN RADIO_ACTIVITY_EVENT*/
#if (BLE_RADIO_ACTIVITY_ON_LED_SUPPORT != 0)
			// osTimerStart(SwitchOffLedTimerId, pdMS_TO_TICKS(LED_ON_TIMEOUT));
#endif
			/* USER CODE END RADIO_ACTIVITY_EVENT*/
			break; /* ACI_HAL_END_OF_RADIO_ACTIVITY_VSEVT_CODE */
		}
		case ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE: {
			LOG_INFO_APP(">>== ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE\n");
			/* USER CODE BEGIN ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE*/

			/* USER CODE END ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE*/
			break;
		}
		case ACI_GAP_PASS_KEY_REQ_VSEVT_CODE: {
			uint32_t pin;
			LOG_INFO_APP(">>== ACI_GAP_PASS_KEY_REQ_VSEVT_CODE\n");

			pin = CFG_FIXED_PIN;
			/* USER CODE BEGIN ACI_GAP_PASS_KEY_REQ_VSEVT_CODE_0 */

			/* USER CODE END ACI_GAP_PASS_KEY_REQ_VSEVT_CODE_0 */

			ret = aci_gap_pass_key_resp(
					bleAppContext.BleApplicationContext_legacy.connectionHandle,
					pin);
			if (ret != BLE_STATUS_SUCCESS) {
				LOG_INFO_APP(
						"==>> aci_gap_pass_key_resp : Fail, reason: 0x%02X\n",
						ret);
			} else {
				LOG_INFO_APP("==>> aci_gap_pass_key_resp : Success\n");
			}
			/* USER CODE BEGIN ACI_GAP_PASS_KEY_REQ_VSEVT_CODE*/

			/* USER CODE END ACI_GAP_PASS_KEY_REQ_VSEVT_CODE*/
			break;
		}
		case ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE: {
			uint8_t confirm_value;
			LOG_INFO_APP(">>== ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE\n");
			LOG_INFO_APP("     - numeric_value = %ld\n",
					((aci_gap_numeric_comparison_value_event_rp0* )(p_blecore_evt->data))->Numeric_Value);
			LOG_INFO_APP("     - Hex_value = %lx\n",
					((aci_gap_numeric_comparison_value_event_rp0* )(p_blecore_evt->data))->Numeric_Value);

			/* Set confirm value to 1(YES) */
			confirm_value = 1;
			/* USER CODE BEGIN ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE_0*/

			/* USER CODE END ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE_0*/

			ret = aci_gap_numeric_comparison_value_confirm_yesno(
					bleAppContext.BleApplicationContext_legacy.connectionHandle,
					confirm_value);
			if (ret != BLE_STATUS_SUCCESS) {
				LOG_INFO_APP(
						"==>> aci_gap_numeric_comparison_value_confirm_yesno : Fail, reason: 0x%02X\n",
						ret);
			} else {
				LOG_INFO_APP(
						"==>> aci_gap_numeric_comparison_value_confirm_yesno : Success\n");
			}
			/* USER CODE BEGIN ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE*/

			/* USER CODE END ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE*/
			break;
		}
		case ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE: {
			LOG_INFO_APP(">>== ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE\n");
			aci_gap_pairing_complete_event_rp0 *p_pairing_complete;
			p_pairing_complete =
					(aci_gap_pairing_complete_event_rp0*) p_blecore_evt->data;

			if (p_pairing_complete->Status != 0) {
				LOG_INFO_APP(
						"     - Pairing KO\n     - Status: 0x%02X\n     - Reason: 0x%02X\n",
						p_pairing_complete->Status, p_pairing_complete->Reason);
			} else {
				LOG_INFO_APP("     - Pairing Success\n");
			}
			LOG_INFO_APP("\n");

			/* USER CODE BEGIN ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE*/

			/* USER CODE END ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE*/
			break;
		}
		case ACI_GAP_BOND_LOST_VSEVT_CODE: {
			LOG_INFO_APP(">>== ACI_GAP_BOND_LOST_EVENT\n");
			ret =
					aci_gap_allow_rebond(
							bleAppContext.BleApplicationContext_legacy.connectionHandle);
			if (ret != BLE_STATUS_SUCCESS) {
				LOG_INFO_APP(
						"==>> aci_gap_allow_rebond : Fail, reason: 0x%02X\n",
						ret);
			} else {
				LOG_INFO_APP("==>> aci_gap_allow_rebond : Success\n");
			}
			/* USER CODE BEGIN ACI_GAP_BOND_LOST_VSEVT_CODE*/

			/* USER CODE END ACI_GAP_BOND_LOST_VSEVT_CODE*/
			break;
		}
			/* USER CODE BEGIN ECODE_1 */

			/* USER CODE END ECODE_1 */
		default: {
			/* USER CODE BEGIN ECODE_DEFAULT*/

			/* USER CODE END ECODE_DEFAULT*/
			break;
		}
		}
		/* USER CODE BEGIN EVT_VENDOR_1 */

		/* USER CODE END EVT_VENDOR_1 */
	}
		break; /* HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE */
		/* USER CODE BEGIN EVENT_PCKT */

		/* USER CODE END EVENT_PCKT */
	default: {
		/* USER CODE BEGIN EVENT_PCKT_DEFAULT*/

		/* USER CODE END EVENT_PCKT_DEFAULT*/
		break;
	}
	}

	/* USER CODE BEGIN SVCCTL_App_Notification_1 */

	/* USER CODE END SVCCTL_App_Notification_1 */

	return (SVCCTL_UserEvtFlowEnable);
}

APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status(void) {
	return bleAppContext.Device_Connection_Status;
}

void APP_BLE_Procedure_Gap_General(ProcGapGeneralId_t ProcGapGeneralId) {
	tBleStatus status;
	uint8_t phy_tx, phy_rx;

	switch (ProcGapGeneralId) {
	case PROC_GAP_GEN_PHY_TOGGLE: {
		osMutexAcquire(LinkLayerMutex, osWaitForever);
		status = hci_le_read_phy(
				bleAppContext.BleApplicationContext_legacy.connectionHandle,
				&phy_tx, &phy_rx);
		osMutexRelease(LinkLayerMutex);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP("hci_le_read_phy failure: reason=0x%02X\n", status);
		} else {
			LOG_INFO_APP("==>> hci_le_read_phy - Success\n");
			LOG_INFO_APP("==>> PHY Param  TX= %d, RX= %d\n", phy_tx, phy_rx);
			if ((phy_tx == HCI_TX_PHY_LE_2M) && (phy_rx == HCI_RX_PHY_LE_2M)) {
				LOG_INFO_APP("==>> hci_le_set_phy PHY Param  TX= %d, RX= %d - ",
						HCI_TX_PHY_LE_1M, HCI_RX_PHY_LE_1M);
				status =
						hci_le_set_phy(
								bleAppContext.BleApplicationContext_legacy.connectionHandle,
								0, HCI_TX_PHYS_LE_1M_PREF,
								HCI_RX_PHYS_LE_1M_PREF, 0);
				if (status != BLE_STATUS_SUCCESS) {
					LOG_INFO_APP("Fail\n");
				} else {
					LOG_INFO_APP("Success\n");
					gap_cmd_resp_wait();/* waiting for HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE */
				}
			} else {
				LOG_INFO_APP("==>> hci_le_set_phy PHY Param  TX= %d, RX= %d - ",
						HCI_TX_PHYS_LE_2M_PREF, HCI_RX_PHYS_LE_2M_PREF);
				status =
						hci_le_set_phy(
								bleAppContext.BleApplicationContext_legacy.connectionHandle,
								0, HCI_TX_PHYS_LE_2M_PREF,
								HCI_RX_PHYS_LE_2M_PREF, 0);
				if (status != BLE_STATUS_SUCCESS) {
					LOG_INFO_APP("Fail\n");
				} else {
					LOG_INFO_APP("Success\n");
					gap_cmd_resp_wait();/* waiting for HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE */
				}
			}
		}
		break;
	}/* PROC_GAP_GEN_PHY_TOGGLE */
	case PROC_GAP_GEN_CONN_TERMINATE: {
		status = aci_gap_terminate(
				bleAppContext.BleApplicationContext_legacy.connectionHandle,
				HCI_REMOTE_USER_TERMINATED_CONNECTION_ERR_CODE);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP("aci_gap_terminate failure: reason=0x%02X\n", status);
		} else {
			LOG_INFO_APP("==>> aci_gap_terminate : Success\n");
		}
		gap_cmd_resp_wait();/* waiting for HCI_DISCONNECTION_COMPLETE_EVT_CODE */
		break;
	}/* PROC_GAP_GEN_CONN_TERMINATE */
	case PROC_GATT_EXCHANGE_CONFIG: {
		status = aci_gatt_exchange_config(
				bleAppContext.BleApplicationContext_legacy.connectionHandle);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP("aci_gatt_exchange_config failure: reason=0x%02X\n",
					status);
		} else {
			LOG_INFO_APP("==>> aci_gatt_exchange_config : Success\n");
		}
		break;
	}
		/* USER CODE BEGIN GAP_GENERAL */

		/* USER CODE END GAP_GENERAL */
	default:
		break;
	}
	return;
}

void APP_BLE_Procedure_Gap_Peripheral(ProcGapPeripheralId_t ProcGapPeripheralId) {
	tBleStatus status;
	uint32_t paramA = ADV_INTERVAL_MIN;
	uint32_t paramB = ADV_INTERVAL_MAX;
	uint32_t paramC, paramD;

	/* First set parameters before calling ACI APIs, only if needed */
	switch (ProcGapPeripheralId) {
	case PROC_GAP_PERIPH_ADVERTISE_START_FAST: {
		paramA = ADV_INTERVAL_MIN;
		paramB = ADV_INTERVAL_MAX;
		paramC = APP_BLE_ADV_FAST;

		break;
	}/* PROC_GAP_PERIPH_ADVERTISE_START_FAST */
	case PROC_GAP_PERIPH_ADVERTISE_START_LP: {
		paramA = ADV_LP_INTERVAL_MIN;
		paramB = ADV_LP_INTERVAL_MAX;
		paramC = APP_BLE_ADV_LP;

		break;
	}/* PROC_GAP_PERIPH_ADVERTISE_START_LP */
	case PROC_GAP_PERIPH_ADVERTISE_STOP: {
		paramC = APP_BLE_IDLE;

		break;
	}/* PROC_GAP_PERIPH_ADVERTISE_STOP */
	case PROC_GAP_PERIPH_CONN_PARAM_UPDATE: {
		paramA = CONN_INT_MS(1000);
		paramB = CONN_INT_MS(1000);
		paramC = 0x0000;
		paramD = 0x01F4;

		/* USER CODE BEGIN CONN_PARAM_UPDATE */
		if (bleAppContext.connIntervalFlag != 0) {
			bleAppContext.connIntervalFlag = 0;
			paramA = CONN_INT_MS(50);
			paramB = CONN_INT_MS(50);
		} else {
			bleAppContext.connIntervalFlag = 1;
			paramA = CONN_INT_MS(1000);
			paramB = CONN_INT_MS(1000);
		}
		/* USER CODE END CONN_PARAM_UPDATE */
		break;
	}/* PROC_GAP_PERIPH_CONN_PARAM_UPDATE */
	default:
		break;
	}

	/* Call ACI APIs */
	switch (ProcGapPeripheralId) {
	case PROC_GAP_PERIPH_ADVERTISE_START_FAST:
	case PROC_GAP_PERIPH_ADVERTISE_START_LP: {
		/* Start Fast or Low Power Advertising */
		status = aci_gap_set_discoverable(ADV_TYPE, paramA, paramB,
		CFG_BD_ADDRESS_TYPE,
		ADV_FILTER, 0, 0, 0, 0, 0, 0);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP(
					"==>> aci_gap_set_discoverable - fail, result: 0x%02X\n",
					status);
		} else {
			bleAppContext.Device_Connection_Status =
					(APP_BLE_ConnStatus_t) paramC;
			LOG_INFO_APP("==>> aci_gap_set_discoverable - Success\n");
		}

		status = aci_gap_delete_ad_type(AD_TYPE_TX_POWER_LEVEL);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP("==>> delete tx power level - fail, result: 0x%02X\n",
					status);
		}

		/* Update Advertising data */
		status = aci_gap_update_adv_data(sizeof(a_AdvData),
				(uint8_t*) a_AdvData);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP("==>> Start Advertising Failed, result: 0x%02X\n",
					status);
		} else {
			LOG_INFO_APP("==>> Success: Start Advertising\n");
		}
		break;
	}
	case PROC_GAP_PERIPH_ADVERTISE_STOP: {
		status = aci_gap_set_non_discoverable();
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP(
					"aci_gap_set_non_discoverable - fail, result: 0x%02X\n",
					status);
		} else {
			bleAppContext.Device_Connection_Status =
					(APP_BLE_ConnStatus_t) paramC;
			LOG_INFO_APP("==>> aci_gap_set_non_discoverable - Success\n");
		}
		break;
	}/* PROC_GAP_PERIPH_ADVERTISE_STOP */
	case PROC_GAP_PERIPH_ADVERTISE_DATA_UPDATE: {
		status = aci_gap_update_adv_data(sizeof(a_AdvData),
				(uint8_t*) a_AdvData);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP("aci_gap_update_adv_data - fail, result: 0x%02X\n",
					status);
		} else {
			LOG_INFO_APP("==>> aci_gap_update_adv_data - Success\n");
		}

		break;
	}/* PROC_GAP_PERIPH_ADVERTISE_DATA_UPDATE */
	case PROC_GAP_PERIPH_CONN_PARAM_UPDATE: {
		status = aci_l2cap_connection_parameter_update_req(
				bleAppContext.BleApplicationContext_legacy.connectionHandle,
				paramA, paramB, paramC, paramD);
		if (status != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP(
					"aci_l2cap_connection_parameter_update_req - fail, result: 0x%02X\n",
					status);
		} else {
			LOG_INFO_APP(
					"==>> aci_l2cap_connection_parameter_update_req - Success\n");
		}

		break;
	}/* PROC_GAP_PERIPH_CONN_PARAM_UPDATE */

	case PROC_GAP_PERIPH_SET_BROADCAST_MODE: {

		break;
	}/* PROC_GAP_PERIPH_SET_BROADCAST_MODE */
	default:
		break;
	}
	return;
}

/* USER CODE BEGIN FD*/
void APP_BLE_AdvStart(void) {
	osMutexAcquire(LinkLayerMutex, osWaitForever);
	APP_BLE_Procedure_Gap_Peripheral(PROC_GAP_PERIPH_ADVERTISE_START_FAST);
	osMutexRelease(LinkLayerMutex);
	
}

void APP_BLE_AdvLowPower(void) {
	osMutexAcquire(LinkLayerMutex, osWaitForever);
	APP_BLE_Procedure_Gap_Peripheral(PROC_GAP_PERIPH_ADVERTISE_STOP);
	APP_BLE_Procedure_Gap_Peripheral(PROC_GAP_PERIPH_ADVERTISE_START_LP);
	osMutexRelease(LinkLayerMutex);
}


void APP_BLE_AdvStop(void) {
	osMutexAcquire(LinkLayerMutex, osWaitForever);
	APP_BLE_Procedure_Gap_Peripheral(PROC_GAP_PERIPH_ADVERTISE_STOP);
	osMutexRelease(LinkLayerMutex);
}

/* USER CODE END FD*/

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
uint8_t HOST_BLE_Init(void) {
	tBleStatus return_status = BLE_STATUS_FAILED;

	pInitParams.numAttrRecord = CFG_BLE_NUM_GATT_ATTRIBUTES;
	pInitParams.numAttrServ = CFG_BLE_NUM_GATT_SERVICES;
	pInitParams.attrValueArrSize = CFG_BLE_ATT_VALUE_ARRAY_SIZE;
	pInitParams.prWriteListSize = CFG_BLE_ATTR_PREPARE_WRITE_VALUE_SIZE;
	pInitParams.attMtu = CFG_BLE_ATT_MTU_MAX;
	pInitParams.max_coc_nbr = CFG_BLE_COC_NBR_MAX;
	pInitParams.max_coc_mps = CFG_BLE_COC_MPS_MAX;
	pInitParams.max_coc_initiator_nbr = CFG_BLE_COC_INITIATOR_NBR_MAX;
	pInitParams.numOfLinks = CFG_BLE_NUM_LINK;
	pInitParams.mblockCount = CFG_BLE_MBLOCK_COUNT;
	pInitParams.bleStartRamAddress = (uint8_t*) buffer;
	pInitParams.total_buffer_size = BLE_DYN_ALLOC_SIZE;
	pInitParams.bleStartRamAddress_GATT = (uint8_t*) gatt_buffer;
	pInitParams.total_buffer_size_GATT = BLE_GATT_BUF_SIZE;
	pInitParams.options = CFG_BLE_OPTIONS;
	pInitParams.debug = 0U;
	/* USER CODE BEGIN HOST_BLE_Init_Params */

	/* USER CODE END HOST_BLE_Init_Params */
	return_status = BleStack_Init(&pInitParams);
	/* USER CODE BEGIN HOST_BLE_Init */

	/* USER CODE END HOST_BLE_Init */
	return ((uint8_t) return_status);
}

static void Ble_Hci_Gap_Gatt_Init(void) {
	uint8_t role;
	uint16_t gap_service_handle, gap_dev_name_char_handle,
			gap_appearance_char_handle;
	const uint8_t *p_bd_addr;
	uint16_t a_appearance[1] = { CFG_GAP_APPEARANCE };
	tBleStatus ret = BLE_STATUS_INVALID_PARAMS;

	/* USER CODE BEGIN Ble_Hci_Gap_Gatt_Init*/

	/* USER CODE END Ble_Hci_Gap_Gatt_Init*/

	LOG_INFO_APP("==>> Start Ble_Hci_Gap_Gatt_Init function\n");

	/* Write the BD Address */
	p_bd_addr = BleGetBdAddress();
	ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
	CONFIG_DATA_PUBADDR_LEN, (uint8_t*) p_bd_addr);
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP(
				"  Fail   : aci_hal_write_config_data command - CONFIG_DATA_PUBADDR_OFFSET, result: 0x%02X\n",
				ret);
	} else {
		LOG_INFO_APP(
				"  Success: aci_hal_write_config_data command - CONFIG_DATA_PUBADDR_OFFSET\n");
		LOG_INFO_APP(
				"  Public Bluetooth Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
				p_bd_addr[5], p_bd_addr[4], p_bd_addr[3], p_bd_addr[2],
				p_bd_addr[1], p_bd_addr[0]);
	}

	/* Write Identity root key used to derive IRK and DHK(Legacy) */
	ret = aci_hal_write_config_data(CONFIG_DATA_IR_OFFSET, CONFIG_DATA_IR_LEN,
			(uint8_t*) a_BLE_CfgIrValue);
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP(
				"  Fail   : aci_hal_write_config_data command - CONFIG_DATA_IR_OFFSET, result: 0x%02X\n",
				ret);
	} else {
		LOG_INFO_APP(
				"  Success: aci_hal_write_config_data command - CONFIG_DATA_IR_OFFSET\n");
	}

	/* Write Encryption root key used to derive LTK and CSRK */
	ret = aci_hal_write_config_data(CONFIG_DATA_ER_OFFSET, CONFIG_DATA_ER_LEN,
			(uint8_t*) a_BLE_CfgErValue);
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP(
				"  Fail   : aci_hal_write_config_data command - CONFIG_DATA_ER_OFFSET, result: 0x%02X\n",
				ret);
	} else {
		LOG_INFO_APP(
				"  Success: aci_hal_write_config_data command - CONFIG_DATA_ER_OFFSET\n");
	}

	/* Set Transmission RF Power. */
	ret = aci_hal_set_tx_power_level(1, CFG_TX_POWER);
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP(
				"  Fail   : aci_hal_set_tx_power_level command, result: 0x%02X\n",
				ret);
	} else {
		LOG_INFO_APP("  Success: aci_hal_set_tx_power_level command\n");
	}

	/* Initialize GATT interface */
	ret = aci_gatt_init();
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP("  Fail   : aci_gatt_init command, result: 0x%02X\n", ret);
	} else {
		LOG_INFO_APP("  Success: aci_gatt_init command\n");
	}

	/* Initialize GAP interface */
	role = 0U;
	role |= GAP_PERIPHERAL_ROLE;

	/* USER CODE BEGIN Role_Mngt*/

	/* USER CODE END Role_Mngt */

	if (role > 0) {
		ret = aci_gap_init(role,
		CFG_PRIVACY, sizeof(a_GapDeviceName), &gap_service_handle,
				&gap_dev_name_char_handle, &gap_appearance_char_handle);

		if (ret != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP("  Fail   : aci_gap_init command, result: 0x%02X\n",
					ret);
		} else {
			LOG_INFO_APP("  Success: aci_gap_init command\n");
		}

		ret = aci_gatt_update_char_value(gap_service_handle,
				gap_dev_name_char_handle, 0, sizeof(a_GapDeviceName),
				(uint8_t*) a_GapDeviceName);
		if (ret != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP(
					"  Fail   : aci_gatt_update_char_value - Device Name, result: 0x%02X\n",
					ret);
		} else {
			LOG_INFO_APP(
					"  Success: aci_gatt_update_char_value - Device Name\n");
		}

		ret = aci_gatt_update_char_value(gap_service_handle,
				gap_appearance_char_handle, 0, sizeof(a_appearance),
				(uint8_t*) &a_appearance);
		if (ret != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP(
					"  Fail   : aci_gatt_update_char_value - Appearance, result: 0x%02X\n",
					ret);
		} else {
			LOG_INFO_APP(
					"  Success: aci_gatt_update_char_value - Appearance\n");
		}
	} else {
		LOG_ERROR_APP("GAP role cannot be null\n");
	}

	/* Initialize Default PHY */
	ret = hci_le_set_default_phy(CFG_PHY_PREF, CFG_PHY_PREF_TX,
			CFG_PHY_PREF_RX);
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP(
				"  Fail   : hci_le_set_default_phy command, result: 0x%02X\n",
				ret);
	} else {
		LOG_INFO_APP("  Success: hci_le_set_default_phy command\n");
	}

	/* Initialize IO capability */
	bleAppContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability =
			CFG_IO_CAPABILITY;
	ret =
			aci_gap_set_io_capability(
					bleAppContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability);
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP(
				"  Fail   : aci_gap_set_io_capability command, result: 0x%02X\n",
				ret);
	} else {
		LOG_INFO_APP("  Success: aci_gap_set_io_capability command\n");
	}

	/* Initialize authentication */
	bleAppContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode =
			CFG_MITM_PROTECTION;
	bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin =
			CFG_ENCRYPTION_KEY_SIZE_MIN;
	bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax =
			CFG_ENCRYPTION_KEY_SIZE_MAX;
	bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin =
			CFG_USED_FIXED_PIN;
	bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin =
			CFG_FIXED_PIN;
	bleAppContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode =
			CFG_BONDING_MODE;
	/* USER CODE BEGIN Ble_Hci_Gap_Gatt_Init_1*/
	fill_advData(&a_AdvData[0], sizeof(a_AdvData), (uint8_t*) p_bd_addr);
	/* USER CODE END Ble_Hci_Gap_Gatt_Init_1*/

	ret =
			aci_gap_set_authentication_requirement(
					bleAppContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode,
					bleAppContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode,
					CFG_SC_SUPPORT,
					CFG_KEYPRESS_NOTIFICATION_SUPPORT,
					bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin,
					bleAppContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax,
					bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin,
					bleAppContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin,
					CFG_BD_ADDRESS_TYPE);
	if (ret != BLE_STATUS_SUCCESS) {
		LOG_INFO_APP(
				"  Fail   : aci_gap_set_authentication_requirement command, result: 0x%02X\n",
				ret);
	} else {
		LOG_INFO_APP(
				"  Success: aci_gap_set_authentication_requirement command\n");
	}

	/* Initialize whitelist */
	if (bleAppContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode) {
		ret = aci_gap_configure_whitelist();
		if (ret != BLE_STATUS_SUCCESS) {
			LOG_INFO_APP(
					"  Fail   : aci_gap_configure_whitelist command, result: 0x%02X\n",
					ret);
		} else {
			LOG_INFO_APP("  Success: aci_gap_configure_whitelist command\n");
		}
	}

	LOG_INFO_APP("==>> End Ble_Hci_Gap_Gatt_Init function\n");

	return;
}

static void Ble_UserEvtRx(void) {
	SVCCTL_UserEvtFlowStatus_t svctl_return_status;
	BleEvtPacket_t *phcievt;

	LST_remove_head(&BleAsynchEventQueue, (tListNode**) &phcievt);

	svctl_return_status = SVCCTL_UserEvtRx((void*) &(phcievt->evtserial));

	if (svctl_return_status != SVCCTL_UserEvtFlowDisable) {
		AMM_Free((uint32_t*) phcievt);
	} else {
		LST_insert_head(&BleAsynchEventQueue, (tListNode*) phcievt);
	}

	if ((LST_is_empty(&BleAsynchEventQueue) == FALSE)
			&& (svctl_return_status != SVCCTL_UserEvtFlowDisable)) {
		osSemaphoreRelease(HCIasyncEvtSemaphore);
	}

	/* set the BG_BleStack_Process task for scheduling */
	osSemaphoreRelease(BleStackSemaphore);

}

static const uint8_t* BleGetBdAddress(void) {
	OTP_Data_s *p_otp_addr = NULL;
	const uint8_t *p_bd_addr;
	uint32_t udn;
	uint32_t company_id;
	uint32_t device_id;

	udn = LL_FLASH_GetUDN();

	if (udn != 0xFFFFFFFF) {
		company_id = LL_FLASH_GetSTCompanyID();
		device_id = LL_FLASH_GetDeviceID();

		/**
		 * Public Address with the ST company ID
		 * bit[47:24] : 24bits (OUI) equal to the company ID
		 * bit[23:16] : Device ID.
		 * bit[15:0] : The last 16bits from the UDN
		 * Note: In order to use the Public Address in a final product, a dedicated
		 * 24bits company ID (OUI) shall be bought.
		 */
		a_BdAddrUdn[0] = (uint8_t) (udn & 0x000000FF);
		a_BdAddrUdn[1] = (uint8_t) ((udn & 0x0000FF00) >> 8);
		a_BdAddrUdn[2] = (uint8_t) device_id;
		a_BdAddrUdn[3] = (uint8_t) (company_id & 0x000000FF);
		a_BdAddrUdn[4] = (uint8_t) ((company_id & 0x0000FF00) >> 8);
		a_BdAddrUdn[5] = (uint8_t) ((company_id & 0x00FF0000) >> 16);

		p_bd_addr = (const uint8_t*) a_BdAddrUdn;
	} else {
		OTP_Read(0, &p_otp_addr);
		if (p_otp_addr) {
			p_bd_addr = (uint8_t*) (p_otp_addr->bd_address);
		} else {
			p_bd_addr = a_MBdAddr;
		}
	}

	return p_bd_addr;
}

static void BleStack_Process_BG(void) {
	uint8_t running = 0x0;

	osMutexAcquire(LinkLayerMutex, osWaitForever);
	running = BleStack_Process();
	osMutexRelease(LinkLayerMutex);
	if (running == 0x0) {
		BleStackCB_Process();
	}
}

static void gap_cmd_resp_release(void) {
	osSemaphoreRelease(ProcGapCompleteSemaphore);
	return;
}

static void gap_cmd_resp_wait(void) {
	osSemaphoreAcquire(ProcGapCompleteSemaphore, osWaitForever);
	return;
}

/**
 * @brief  Notify the LL to resume the flow process
 * @param  None
 * @retval None
 */
static void BLE_ResumeFlowProcessCallback(void) {
	/* Receive any events from the LL. */
	change_state_options_t notify_options;

	notify_options.combined_value = 0x0F;

	ll_intf_chng_evnt_hndlr_state(notify_options);
}

static void BLE_NvmCallback(SNVMA_Callback_Status_t CbkStatus) {
	if (CbkStatus != SNVMA_OPERATION_COMPLETE) {
		/* Retry the write operation */
		SNVMA_Write(APP_BLE_NvmBuffer, BLE_NvmCallback);
	}
}

/* USER CODE BEGIN FD_LOCAL_FUNCTION */

static void fill_advData(uint8_t *p_adv_data, uint8_t tab_size,
		const uint8_t *p_bd_addr) {
	uint16_t i = 0;
	uint8_t bd_addr_1, bd_addr_0;
	uint8_t ad_length, ad_type;

	while (i < tab_size) {
		ad_length = p_adv_data[i];
		ad_type = p_adv_data[i + 1];

		switch (ad_type) {
		case AD_TYPE_FLAGS:
			break;
		case AD_TYPE_TX_POWER_LEVEL:
			break;
		case AD_TYPE_COMPLETE_LOCAL_NAME: {
			if ((p_adv_data[i + ad_length] == 'X')
					&& (p_adv_data[i + ad_length - 1] == 'X')) {
				bd_addr_1 = ((p_bd_addr[0] & 0xF0) >> 4);
				bd_addr_0 = (p_bd_addr[0] & 0xF);

				/* Convert hex value into ascii */
				if (bd_addr_1 > 0x09) {
					p_adv_data[i + ad_length - 1] = bd_addr_1 + '7';
				} else {
					p_adv_data[i + ad_length - 1] = bd_addr_1 + '0';
				}

				if (bd_addr_0 > 0x09) {
					p_adv_data[i + ad_length] = bd_addr_0 + '7';
				} else {
					p_adv_data[i + ad_length] = bd_addr_0 + '0';
				}
			}
			break;
		}
		case AD_TYPE_MANUFACTURER_SPECIFIC_DATA: {
			p_adv_data[i + 2] = ST_MANUF_ID;
			p_adv_data[i + 3] = 0x00;
			p_adv_data[i + 4] = BLUESTSDK_V2; /* blueST SDK version */
			p_adv_data[i + 5] = BOARD_ID_NUCLEO_WBA; /* Board ID */
			p_adv_data[i + 6] = FW_ID_HEART_RATE; /* FW ID */
			p_adv_data[i + 7] = 0x00; /* FW data 1 */
			p_adv_data[i + 8] = 0x00; /* FW data 2 */
			p_adv_data[i + 9] = 0x00; /* FW data 3 */
			p_adv_data[i + 10] = p_bd_addr[5]; /* MSB BD address */
			p_adv_data[i + 11] = p_bd_addr[4];
			p_adv_data[i + 12] = p_bd_addr[3];
			p_adv_data[i + 13] = p_bd_addr[2];
			p_adv_data[i + 14] = p_bd_addr[1];
			p_adv_data[i + 15] = p_bd_addr[0]; /* LSB BD address */
			break;
		}
		default:
			break;
		}
		i += ad_length + 1; /* increment the iterator to go on next element*/
	}
}
/* USER CODE END FD_LOCAL_FUNCTION */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

tBleStatus BLECB_Indication(const uint8_t *data, uint16_t length,
		const uint8_t *ext_data, uint16_t ext_length) {
	uint8_t status = BLE_STATUS_FAILED;
	BleEvtPacket_t *phcievt;
	uint16_t total_length = (length + ext_length);

	UNUSED(ext_data);

	if (data[0] == HCI_EVENT_PKT_TYPE) {
		APP_BLE_ResumeFlowProcessCb.Callback = BLE_ResumeFlowProcessCallback;
		if (AMM_Alloc(CFG_AMM_VIRTUAL_APP_BLE,
				DIVC((sizeof(BleEvtPacketHeader_t) + total_length),
						sizeof(uint32_t)), (uint32_t**) &phcievt,
				&APP_BLE_ResumeFlowProcessCb) != AMM_ERROR_OK) {
			LOG_INFO_APP("Alloc failed\n");
			status = BLE_STATUS_FAILED;
		} else if (phcievt != (BleEvtPacket_t*) 0) {
			phcievt->evtserial.type = HCI_EVENT_PKT_TYPE;
			phcievt->evtserial.evt.evtcode = data[1];
			phcievt->evtserial.evt.plen = data[2];
			memcpy((void*) &phcievt->evtserial.evt.payload, &data[3], data[2]);
			LST_insert_tail(&BleAsynchEventQueue, (tListNode*) phcievt);
			osSemaphoreRelease(HCIasyncEvtSemaphore);
			status = BLE_STATUS_SUCCESS;
		}
	} else if (data[0] == HCI_ACLDATA_PKT_TYPE) {
		status = BLE_STATUS_SUCCESS;
	}
	return status;
}
static void BleUserEvtRx_Task_Entry(void *argument) {

#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif
    while (1) {
        osSemaphoreAcquire(HCIasyncEvtSemaphore, osWaitForever);

        osMutexAcquire(LinkLayerMutex, osWaitForever);
        Ble_UserEvtRx();
        osMutexRelease(LinkLayerMutex);
        osThreadYield();

#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (uxHighWaterMark != lastHighWaterMark) {
            APP_DBG("\x1b[34m" "BleUserEvtRxTaskEntry_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
            lastHighWaterMark = uxHighWaterMark;
        }
#endif
    }
}

static void BleStack_Task_Entry(void *argument) {
	UNUSED(argument);
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif // endif HIGHWATERMARK

	while (1) {
		osSemaphoreAcquire(BleStackSemaphore, osWaitForever);
		BleStack_Process_BG();
		osThreadYield();

#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (uxHighWaterMark != lastHighWaterMark) {
            APP_DBG("\x1b[34m" "BleStackTaskEntry_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
            lastHighWaterMark = uxHighWaterMark;
        }
#endif
	}
}


void NVMCB_Store(const uint32_t *ptr, uint32_t size) {
	UNUSED(ptr);
	UNUSED(size);

	/* Call SNVMA for storing - Without callback */
	SNVMA_Write(APP_BLE_NvmBuffer, BLE_NvmCallback);
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

#if (BLE_RADIO_ACTIVITY_ON_LED_SUPPORT != 0)
static void Switch_OFF_Led(void *arg) {
	return;
}
#endif

/* USER CODE END FD_WRAP_FUNCTIONS */
