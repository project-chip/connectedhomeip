/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : App/app_thread.c
 * Description        : Thread Application.
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

/* Includes ------------------------------------------------------------------*/
#include "app_thread.h"
#include "app_common.h"
#include "app_conf.h"
#include "app_entry.h"
#include "cmsis_os.h"
#include "dbg_trace.h"
#include "openthread_api_wb.h"
#include "queue.h"
#include "shci.h"
#include "stm32_lpm.h"
#include "stm32wbxx_core_interface_def.h"
#include "stm_logging.h"
#include "utilities_common.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define C_SIZE_CMD_STRING 256U
#define MO_NOTIF_QUEUE_SIZE 10

/* USER CODE BEGIN PD */

/* USER CODE END PD */

static osSemaphoreId_t TransferToM0Semaphore;
static osMutexId_t MtxThreadId;

/* FreeRtos stacks attributes */
const osThreadAttr_t ThreadMsgM0ToM4Process_attr = { .name       = CFG_THREAD_MSG_M0_TO_M4_PROCESS_NAME,
                                                     .attr_bits  = CFG_THREAD_MSG_M0_TO_M4_PROCESS_ATTR_BITS,
                                                     .cb_mem     = CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_MEM,
                                                     .cb_size    = CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_SIZE,
                                                     .stack_mem  = CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_MEM,
                                                     .priority   = CFG_THREAD_MSG_M0_TO_M4_PROCESS_PRIORITY,
                                                     .stack_size = CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_SIZE };

const osThreadAttr_t ThreadCliProcess_attr = { .name       = CFG_THREAD_CLI_PROCESS_NAME,
                                               .attr_bits  = CFG_THREAD_CLI_PROCESS_ATTR_BITS,
                                               .cb_mem     = CFG_THREAD_CLI_PROCESS_CB_MEM,
                                               .cb_size    = CFG_THREAD_CLI_PROCESS_CB_SIZE,
                                               .stack_mem  = CFG_THREAD_CLI_PROCESS_STACK_MEM,
                                               .priority   = CFG_THREAD_CLI_PROCESS_PRIORITY,
                                               .stack_size = CFG_THREAD_CLI_PROCESS_STACK_SIZE };

static volatile int FlagReceiveAckFromM0 = 0;
/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
static void APP_THREAD_CheckWirelessFirmwareInfo(void);
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode);
#if (CFG_FULL_LOW_POWER == 0)
static void Send_CLI_To_M0(void);
#endif /* (CFG_FULL_LOW_POWER == 0) */
static void Send_CLI_Ack_For_OT(void);
static void HostTxCb(void);
static void Wait_Getting_Ack_From_M0(void);
static void Receive_Ack_From_M0(void);
static void Receive_Notification_From_M0(void);
static void APP_THREAD_FreeRTOSProcessMsgM0ToM4Task(void * argument);
static void Ot_Cmd_Transfer_Common(void);
#if (CFG_FULL_LOW_POWER == 0)
static void APP_THREAD_FreeRTOSSendCLIToM0Task(void * argument);
#endif /* (CFG_FULL_LOW_POWER == 0) */
#if (CFG_FULL_LOW_POWER == 0)
static void RxCpltCallback(void);
#endif /* (CFG_FULL_LOW_POWER == 0) */

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
#if (CFG_FULL_LOW_POWER == 0)
static uint8_t aRxBuffer[C_SIZE_CMD_STRING];
#endif /* (CFG_FULL_LOW_POWER == 0) */

#if (CFG_FULL_LOW_POWER == 0)
static uint8_t CommandString[C_SIZE_CMD_STRING];
#endif /* (CFG_FULL_LOW_POWER == 0) */
static __IO uint16_t indexReceiveChar      = 0;
static __IO uint16_t CptReceiveCmdFromUser = 0;

static TL_CmdPacket_t * p_thread_otcmdbuffer;
static TL_EvtPacket_t * p_thread_notif_M0_to_M4;
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_TH_Config_t ThreadConfigBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadOtCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ThreadNotifRspEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadCliCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadCliNotBuffer;
extern uint8_t g_ot_notification_allowed;

/* USER CODE BEGIN PV */
static QueueHandle_t MoNotifQueue;
static osThreadId_t OsTaskMsgM0ToM4Id; /* Task managing the M0 to M4 messaging        */
#if (CFG_FULL_LOW_POWER == 0)
static osThreadId_t OsTaskCliId; /* Task used to manage CLI command             */
#endif                           /* (CFG_FULL_LOW_POWER == 0) */
/* Debug */
/* USER CODE END PV */

/* Functions Definition ------------------------------------------------------*/

void APP_THREAD_Init(void)
{
    /* USER CODE BEGIN APP_THREAD_INIT_1 */
    /* Do not allow stop mode before Thread is initialized */
    UTIL_LPM_SetStopMode(1 << CFG_LPM_APP_THREAD, UTIL_LPM_DISABLE);
    /* USER CODE END APP_THREAD_INIT_1 */

    SHCI_CmdStatus_t ThreadInitStatus;

    /* Check the compatibility with the Coprocessor Wireless Firmware loaded */
    APP_THREAD_CheckWirelessFirmwareInfo();

    /* Register cmdbuffer */
    APP_THREAD_RegisterCmdBuffer(&ThreadOtCmdBuffer);

    /**
     * Do not allow standby in the application
     */
    UTIL_LPM_SetOffMode(1 << CFG_LPM_APP_THREAD, UTIL_LPM_DISABLE);

    /* Init config buffer and call TL_THREAD_Init */
    APP_THREAD_TL_THREAD_INIT();

    /* Configure UART for sending CLI command from M4 */
    //  APP_THREAD_Init_UART_CLI();  Conflict with qspi gpio
    /* Send Thread start system cmd to M0 */
    ThreadInitStatus = SHCI_C2_THREAD_Init();

    /* Prevent unused argument(s) compilation warning */
    UNUSED(ThreadInitStatus);

    /* Semaphore */
    TransferToM0Semaphore = osSemaphoreNew(1, 0, NULL);
    /* Initialize the mutex */
    MtxThreadId = osMutexNew(NULL);

    MoNotifQueue = xQueueCreate(MO_NOTIF_QUEUE_SIZE, sizeof(uint8_t));
    if (MoNotifQueue == NULL)
    {
        APP_DBG("Failed to allocate M0 notification queue");
    }

    /* Create the different FreeRTOS tasks requested to run this Thread application*/
    OsTaskMsgM0ToM4Id = osThreadNew(APP_THREAD_FreeRTOSProcessMsgM0ToM4Task, NULL, &ThreadMsgM0ToM4Process_attr);

    /* USER CODE BEGIN APP_THREAD_INIT_FREERTOS */
    /* USER CODE END APP_THREAD_INIT_FREERTOS */
    /* USER CODE BEGIN APP_THREAD_INIT_2 */
    /* USER CODE END APP_THREAD_INIT_2 */
}

/**
 * @brief  Trace the error or the warning reported.
 * @param  ErrId :
 * @param  ErrCode
 * @retval None
 */
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode)
{
    /* USER CODE BEGIN APP_THREAD_Error_1 */

    /* USER CODE END APP_THREAD_Error_1 */
    switch (ErrId)
    {
    case ERR_REC_MULTI_MSG_FROM_M0:
        APP_THREAD_TraceError("ERROR : ERR_REC_MULTI_MSG_FROM_M0 ", ErrCode);
        break;
    case ERR_THREAD_SET_STATE_CB:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_STATE_CB ", ErrCode);
        break;
    case ERR_THREAD_SET_CHANNEL:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_CHANNEL ", ErrCode);
        break;
    case ERR_THREAD_SET_PANID:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_PANID ", ErrCode);
        break;
    case ERR_THREAD_IPV6_ENABLE:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_IPV6_ENABLE ", ErrCode);
        break;
    case ERR_THREAD_START:
        APP_THREAD_TraceError("ERROR: ERR_THREAD_START ", ErrCode);
        break;
    case ERR_THREAD_ERASE_PERSISTENT_INFO:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_ERASE_PERSISTENT_INFO ", ErrCode);
        break;
    case ERR_THREAD_CHECK_WIRELESS:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_CHECK_WIRELESS ", ErrCode);
        break;
        /* USER CODE BEGIN APP_THREAD_Error_2 */
    case ERR_THREAD_COAP_START:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_START ", ErrCode);
        break;
    case ERR_THREAD_COAP_ADD_RESSOURCE:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_ADD_RESSOURCE ", ErrCode);
        break;
    case ERR_THREAD_MESSAGE_READ:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_MESSAGE_READ ", ErrCode);
        break;
    case ERR_THREAD_COAP_SEND_RESPONSE:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_RESPONSE ", ErrCode);
        break;
    case ERR_THREAD_COAP_APPEND:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_APPEND ", ErrCode);
        break;
    case ERR_THREAD_COAP_SEND_REQUEST:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_REQUEST ", ErrCode);
        break;
    case ERR_TIMER_INIT:
        APP_THREAD_TraceError("ERROR : ERR_TIMER_INIT ", ErrCode);
        break;
    case ERR_TIMER_START:
        APP_THREAD_TraceError("ERROR : ERR_TIMER_START ", ErrCode);
        break;
        /* USER CODE END APP_THREAD_Error_2 */
    default:
        APP_THREAD_TraceError("ERROR Unknown ", 0);
        break;
    }
}

/**
 * @brief Perform initialization of CLI UART interface.
 * @param  None
 * @retval None
 */
void APP_THREAD_Init_UART_CLI(void)
{
#if (CFG_FULL_LOW_POWER == 0)
    OsTaskCliId = osThreadNew(APP_THREAD_FreeRTOSSendCLIToM0Task, NULL, &ThreadCliProcess_attr);
#endif /* (CFG_FULL_LOW_POWER == 0) */

#if (CFG_FULL_LOW_POWER == 0)
    HW_UART_Init(CFG_CLI_UART);
    HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1, RxCpltCallback);
#endif /* (CFG_FULL_LOW_POWER == 0) */
}

/**
 * @brief Perform initialization of TL for THREAD.
 * @param  None
 * @retval None
 */
void APP_THREAD_TL_THREAD_INIT(void)
{
    ThreadConfigBuffer.p_ThreadOtCmdRspBuffer = (uint8_t *) &ThreadOtCmdBuffer;
    ThreadConfigBuffer.p_ThreadNotAckBuffer   = (uint8_t *) ThreadNotifRspEvtBuffer;
    ThreadConfigBuffer.p_ThreadCliRspBuffer   = (uint8_t *) &ThreadCliCmdBuffer;
    ThreadConfigBuffer.p_ThreadCliNotBuffer   = (uint8_t *) &ThreadCliNotBuffer;

    TL_THREAD_Init(&ThreadConfigBuffer);
}

/**
 * @brief  This function is used to transfer the Ot commands from the
 *         M4 to the M0.
 *
 * @param   None
 * @return  None
 */
void Ot_Cmd_Transfer(void)
{
    Ot_Cmd_Transfer_Common();
}

/**
 * @brief  This function is used to transfer the Ot commands from the
 *         M4 to the M0 with Notification M0 to M4 allowed.
 *
 * @param   None
 * @return  None
 */
void Ot_Cmd_TransferWithNotif(void)
{
    /* Flag to specify to UTIL_SEQ_EvtIdle that M0 to M4 notifications are allowed */
    g_ot_notification_allowed = 1U;

    Ot_Cmd_Transfer_Common();
}

/**
 * @brief  This function is called when acknowledge from OT command is received from the M0+.
 *
 * @param   Otbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_OT_CmdEvtReceived(TL_EvtPacket_t * Otbuffer)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(Otbuffer);

    Receive_Ack_From_M0();

    /* Does not allow OpenThread M0 to M4 notification */
    g_ot_notification_allowed = 0U;
}

/**
 * @brief  This function is called when notification from M0+ is received.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_THREAD_NotReceived(TL_EvtPacket_t * Notbuffer)
{
    p_thread_notif_M0_to_M4 = Notbuffer;

    Receive_Notification_From_M0();
}

/**
 * @brief  This function is called when notification on CLI TL Channel from M0+ is received.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_THREAD_CliNotReceived(TL_EvtPacket_t * Notbuffer)
{
    TL_CmdPacket_t * l_CliBuffer = (TL_CmdPacket_t *) Notbuffer;
    uint8_t l_size               = l_CliBuffer->cmdserial.cmd.plen;

    /* WORKAROUND: if string to output is "> " then respond directly to M0 and do not output it */
    if (strcmp((const char *) l_CliBuffer->cmdserial.cmd.payload, "> ") != 0)
    {
        /* Write to CLI UART */
        HW_UART_Transmit_IT(CFG_CLI_UART, l_CliBuffer->cmdserial.cmd.payload, l_size, HostTxCb);
    }
    else
    {
        Send_CLI_Ack_For_OT();
    }
}

/**
 * @brief  This function is called before sending any ot command to the M0
 *         core. The purpose of this function is to be able to check if
 *         there are no notifications coming from the M0 core which are
 *         pending before sending a new ot command.
 * @param  None
 * @retval None
 */
void Pre_OtCmdProcessing(void)
{
    osMutexAcquire(MtxThreadId, osWaitForever);
}

void APP_THREAD_RegisterCmdBuffer(TL_CmdPacket_t * p_buffer)
{
    p_thread_otcmdbuffer = p_buffer;
}

Thread_OT_Cmd_Request_t * THREAD_Get_OTCmdPayloadBuffer(void)
{
    return (Thread_OT_Cmd_Request_t *) p_thread_otcmdbuffer->cmdserial.cmd.payload;
}

Thread_OT_Cmd_Request_t * THREAD_Get_OTCmdRspPayloadBuffer(void)
{
    return (Thread_OT_Cmd_Request_t *) ((TL_EvtPacket_t *) p_thread_otcmdbuffer)->evtserial.evt.payload;
}

Thread_OT_Cmd_Request_t * THREAD_Get_NotificationPayloadBuffer(void)
{
    return (Thread_OT_Cmd_Request_t *) (p_thread_notif_M0_to_M4)->evtserial.evt.payload;
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief  Warn the user that an error has occurred.In this case,
 *         the LEDs on the Board will start blinking.
 *
 * @param  pMess  : Message associated to the error.
 * @param  ErrCode: Error code associated to the module (OpenThread or other module if any)
 * @retval None
 */
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode)
{
    /* USER CODE BEGIN TRACE_ERROR */
    APP_DBG("**** Fatal error = %s (Err = %d)", pMess, ErrCode);
    while (1U == 1U)
    {
        HAL_Delay(500U);
    }
    /* USER CODE END TRACE_ERROR */
}

/**
 * @brief Check if the Coprocessor Wireless Firmware loaded supports Thread
 *        and display associated information
 * @param  None
 * @retval None
 */
static void APP_THREAD_CheckWirelessFirmwareInfo(void)
{
    WirelessFwInfo_t wireless_info_instance;
    WirelessFwInfo_t * p_wireless_info = &wireless_info_instance;

    if (SHCI_GetWirelessFwInfo(p_wireless_info) != SHCI_Success)
    {
        APP_THREAD_Error((uint32_t) ERR_THREAD_CHECK_WIRELESS, (uint32_t) ERR_INTERFACE_FATAL);
    }
    else
    {
        APP_DBG("**********************************************************");
        APP_DBG("WIRELESS COPROCESSOR FW:");
        /* Print version */
        APP_DBG("VERSION ID = %d.%d.%d", p_wireless_info->VersionMajor, p_wireless_info->VersionMinor, p_wireless_info->VersionSub);

        switch (p_wireless_info->StackType)
        {
        case INFO_STACK_TYPE_THREAD_FTD:
            APP_DBG("FW Type : Thread FTD");
            break;
        case INFO_STACK_TYPE_THREAD_MTD:
            APP_DBG("FW Type : Thread MTD");
            break;
        case INFO_STACK_TYPE_BLE_THREAD_FTD_DYAMIC:
            APP_DBG("FW Type : Dynamic Concurrent Mode BLE/Thread");
            break;
            //		case INFO_STACK_TYPE_BLE_THREAD_FOR_MATTER:
            //			APP_DBG("FW Type : Dynamic Concurrent Mode BLE/Thread for Matter ")
            //			;
            //			break;
        default:
            /* No Thread device supported ! */
            APP_THREAD_Error((uint32_t) ERR_THREAD_CHECK_WIRELESS, (uint32_t) ERR_INTERFACE_FATAL);
            break;
        }
        APP_DBG("**********************************************************");
    }
}

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
 *************************************************************/
static void APP_THREAD_FreeRTOSProcessMsgM0ToM4Task(void * argument)
{
    UNUSED(argument);
    uint8_t NotUsed = 0;
    for (;;)
    {
        /* USER CODE BEGIN APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_1 */

        /* USER END END APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_1 */
        xQueueReceive(MoNotifQueue, &NotUsed, portMAX_DELAY);

        if (uxQueueMessagesWaiting(MoNotifQueue) > 1U)
        {
            APP_THREAD_Error(ERR_REC_MULTI_MSG_FROM_M0, 0);
        }
        else
        {
            OpenThread_CallBack_Processing();
        }
        /* USER CODE BEGIN APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_2 */

        /* USER END END APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_2 */
    }
}

#if (CFG_FULL_LOW_POWER == 0)
static void APP_THREAD_FreeRTOSSendCLIToM0Task(void * argument)
{
    UNUSED(argument);
    for (;;)
    {
        /* USER CODE BEGIN APP_THREAD_FREERTOS_SEND_CLI_TO_M0_1 */

        /* USER END END APP_THREAD_FREERTOS_SEND_CLI_TO_M0_1 */
        osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
        Send_CLI_To_M0();
        /* USER CODE BEGIN APP_THREAD_FREERTOS_SEND_CLI_TO_M0_2 */

        /* USER END END APP_THREAD_FREERTOS_SEND_CLI_TO_M0_2 */
    }
}
#endif /* (CFG_FULL_LOW_POWER == 0) */

/* USER CODE BEGIN FREERTOS_WRAPPER_FUNCTIONS */
/* USER CODE END FREERTOS_WRAPPER_FUNCTIONS */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
static void Ot_Cmd_Transfer_Common(void)
{
    /* OpenThread OT command cmdcode range 0x280 .. 0x3DF = 352 */
    p_thread_otcmdbuffer->cmdserial.cmd.cmdcode = 0x280U;
    /* Size = otCmdBuffer->Size (Number of OT cmd arguments : 1 arg = 32bits so multiply by 4 to get size in bytes)
     * + ID (4 bytes) + Size (4 bytes) */
    uint32_t l_size = ((Thread_OT_Cmd_Request_t *) (p_thread_otcmdbuffer->cmdserial.cmd.payload))->Size * 4U + 8U;
    p_thread_otcmdbuffer->cmdserial.cmd.plen = l_size;

    TL_OT_SendCmd();

    /* Wait completion of cmd */
    Wait_Getting_Ack_From_M0();
}

/**
 * @brief  This function waits for getting an acknowledgment from the M0.
 *
 * @param  None
 * @retval None
 */
static void Wait_Getting_Ack_From_M0(void)
{
    while (FlagReceiveAckFromM0 == 0)
    {
    }
    FlagReceiveAckFromM0 = 0;
    osMutexRelease(MtxThreadId);
    //    osSemaphoreAcquire( TransferToM0Semaphore, osWaitForever );
}

/**
 * @brief  Receive an acknowledgment from the M0+ core.
 *         Each command send by the M4 to the M0 are acknowledged.
 *         This function is called under interrupt.
 * @param  None
 * @retval None
 */
static void Receive_Ack_From_M0(void)
{
    FlagReceiveAckFromM0 = 1;
    // osSemaphoreRelease( TransferToM0Semaphore);
}

/**
 * @brief  Receive a notification from the M0+ through the IPCC.
 *         This function is called under interrupt.
 * @param  None
 * @retval None
 */
static void Receive_Notification_From_M0(void)
{
    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
     it will get set to pdTRUE inside the interrupt safe API function if a
     context switch is required. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint8_t NotUsed = 0;
    xQueueSendToFrontFromISR(MoNotifQueue, &NotUsed, &xHigherPriorityTaskWoken);

    /* Pass the xHigherPriorityTaskWoken value into portEND_SWITCHING_ISR(). If
     xHigherPriorityTaskWoken was set to pdTRUE inside xSemaphoreGiveFromISR()
     then calling portEND_SWITCHING_ISR() will request a context switch. If
     xHigherPriorityTaskWoken is still pdFALSE then calling
     portEND_SWITCHING_ISR() will have no effect */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

#if (CFG_FULL_LOW_POWER == 0)
static void RxCpltCallback(void)
{
    /* Filling buffer and wait for '\r' char */
    if (indexReceiveChar < C_SIZE_CMD_STRING)
    {
        CommandString[indexReceiveChar++] = aRxBuffer[0];
        if (aRxBuffer[0] == '\r')
        {
            CptReceiveCmdFromUser = 1U;

            /* UART task scheduling*/
            osThreadFlagsSet(OsTaskCliId, 1);
        }
    }

    /* Once a character has been sent, put back the device in reception mode */
    HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1U, RxCpltCallback);
}
#endif /* (CFG_FULL_LOW_POWER == 0) */

#if (CFG_FULL_LOW_POWER == 0)
/**
 * @brief Process sends receive CLI command to M0.
 * @param  None
 * @retval None
 */
static void Send_CLI_To_M0(void)
{
    memset(ThreadCliCmdBuffer.cmdserial.cmd.payload, 0x0U, 255U);
    memcpy(ThreadCliCmdBuffer.cmdserial.cmd.payload, CommandString, indexReceiveChar);
    ThreadCliCmdBuffer.cmdserial.cmd.plen    = indexReceiveChar;
    ThreadCliCmdBuffer.cmdserial.cmd.cmdcode = 0x0;

    /* Clear receive buffer, character counter and command complete */
    CptReceiveCmdFromUser = 0;
    indexReceiveChar      = 0;
    memset(CommandString, 0, C_SIZE_CMD_STRING);

    TL_CLI_SendCmd();
}
#endif /* (CFG_FULL_LOW_POWER == 0) */

/**
 * @brief Send notification for CLI TL Channel.
 * @param  None
 * @retval None
 */
static void Send_CLI_Ack_For_OT(void)
{

    /* Notify M0 that characters have been sent to UART */
    TL_THREAD_CliSendAck();
}

/**
 * @brief  End of transfer callback for CLI UART sending.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
static void HostTxCb(void)
{
    Send_CLI_Ack_For_OT();
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

/* USER CODE END FD_WRAP_FUNCTIONS */
