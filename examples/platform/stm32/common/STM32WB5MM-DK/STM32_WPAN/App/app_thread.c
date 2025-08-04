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
#define THREAD_LINK_POLL_PERIOD (5 * 1000 * 1000 / CFG_TS_TICK_VAL) /**< 5s */
#define MO_NOTIF_QUEUE_SIZE 10
/* FreeRtos stacks attributes */
const osThreadAttr_t ThreadMsgM0ToM4Process_attr = { .name       = CFG_THREAD_MSG_M0_TO_M4_PROCESS_NAME,
                                                     .attr_bits  = CFG_THREAD_MSG_M0_TO_M4_PROCESS_ATTR_BITS,
                                                     .cb_mem     = CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_MEM,
                                                     .cb_size    = CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_SIZE,
                                                     .stack_mem  = CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_MEM,
                                                     .priority   = CFG_THREAD_MSG_M0_TO_M4_PROCESS_PRIORITY,
                                                     .stack_size = CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_SIZE };

static osSemaphoreId_t OtCmdProcessSem;
static osSemaphoreId_t OtCmdAckSem;

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
static void APP_THREAD_CheckWirelessFirmwareInfo(void);
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void * pContext);
static void APP_THREAD_DeviceConfig(void);
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode);
static void Wait_Getting_Ack_From_M0(void);
static void Receive_Ack_From_M0(void);
static void Receive_Notification_From_M0(void);

/* FreeRTos wrapper functions */
static void APP_THREAD_FreeRTOSProcessMsgM0ToM4Task(void * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
static TL_CmdPacket_t * p_thread_otcmdbuffer;
static TL_EvtPacket_t * p_thread_notif_M0_to_M4;
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_TH_Config_t ThreadConfigBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadOtCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ThreadNotifRspEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadCliCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadCliNotBuffer;
extern uint8_t g_ot_notification_allowed;

static QueueHandle_t MoNotifQueue;
static osThreadId_t OsTaskMsgM0ToM4Id; /* Task managing the M0 to M4 messaging        */
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief Main entry point for the Thread Application
 * @param  none
 * @retval None
 */
void APP_THREAD_Init_Dyn_1(void)
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

    /* Send Thread start system cmd to M0 */
    ThreadInitStatus = SHCI_C2_THREAD_Init();

    /* Prevent unused argument(s) compilation warning */
    UNUSED(ThreadInitStatus);

    /* Create the different FreeRTOS tasks requested to run this Thread application*/
    OsTaskMsgM0ToM4Id = osThreadNew(APP_THREAD_FreeRTOSProcessMsgM0ToM4Task, NULL, &ThreadMsgM0ToM4Process_attr);

    /* Create binary semaphores for OT command handling */
    OtCmdProcessSem = osSemaphoreNew(1, 1, NULL);
    OtCmdAckSem     = osSemaphoreNew(1, 0, NULL);

    /* USER CODE BEGIN APP_THREAD_INIT_FREERTOS */
    MoNotifQueue = xQueueCreate(MO_NOTIF_QUEUE_SIZE, sizeof(uint8_t));
    if (MoNotifQueue == NULL)
    {
        APP_DBG("Failed to allocate M0 notification queue");
    }
    /* USER CODE END APP_THREAD_INIT_FREERTOS */
}

void APP_THREAD_Init_Dyn_2(void)
{
    /* Initialize and configure the Thread device*/
#if (CFG_LPM_SUPPORTED == 1)
    APP_THREAD_DeviceConfig();

    /* Allow the 800_15_4 IP to enter in low power mode */
    SHCI_C2_RADIO_AllowLowPower(THREAD_IP, TRUE);

    /* Allow stop mode after Thread initialization*/
    UTIL_LPM_SetStopMode(1 << CFG_LPM_APP_THREAD, UTIL_LPM_ENABLE);
#endif
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
    case ERR_THREAD_SET_NETWORK_KEY:
        APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_NETWORK_KEY ", ErrCode);
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

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
static void APP_THREAD_DeviceConfig(void)
{
    otError error                = OT_ERROR_NONE;
    otLinkModeConfig OT_LinkMode = { 0 };
    /* Set the pool period to 5 sec. It means that when the device will enter
     * in 'sleepy end device' mode, it will send an ACK_Request every 5 sec.
     * This message will act as keep alive message.
     */
    error = otLinkSetPollPeriod(NULL, THREAD_LINK_POLL_PERIOD);
    /* Set the sleepy end device mode */
    OT_LinkMode.mRxOnWhenIdle = 0;
    OT_LinkMode.mDeviceType   = 0;
    OT_LinkMode.mNetworkData  = 1U;

    error = otThreadSetLinkMode(NULL, OT_LinkMode);
    if (error != OT_ERROR_NONE)
    {
        APP_THREAD_Error(ERR_THREAD_LINK_MODE, error);
    }
    error = otIp6SetEnabled(NULL, true);
    if (error != OT_ERROR_NONE)
    {
        APP_THREAD_Error(ERR_THREAD_IPV6_ENABLE, error);
    }
    error = otThreadSetEnabled(NULL, true);
    if (error != OT_ERROR_NONE)
    {
        APP_THREAD_Error(ERR_THREAD_START, error);
    }
}
/* @brief Thread notification when the state changes.
 * @param  aFlags  : Define the item that has been modified
 *         aContext: Context
 * @retval None
 */
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void * pContext)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(pContext);

    /* USER CODE BEGIN APP_THREAD_STATENOTIF */

    /* USER CODE END APP_THREAD_STATENOTIF */

    if ((NotifFlags & (uint32_t) OT_CHANGED_THREAD_ROLE) == (uint32_t) OT_CHANGED_THREAD_ROLE)
    {
        switch (otThreadGetDeviceRole(NULL))
        {
        case OT_DEVICE_ROLE_DISABLED:
            /* USER CODE BEGIN OT_DEVICE_ROLE_DISABLED */
            /* USER CODE END OT_DEVICE_ROLE_DISABLED */
            break;
        case OT_DEVICE_ROLE_DETACHED:
            /* USER CODE BEGIN OT_DEVICE_ROLE_DETACHED */
            /* USER CODE END OT_DEVICE_ROLE_DETACHED */
            break;
        case OT_DEVICE_ROLE_CHILD:
            /* USER CODE BEGIN OT_DEVICE_ROLE_CHILD */
            /* USER CODE END OT_DEVICE_ROLE_CHILD */
            break;
        case OT_DEVICE_ROLE_ROUTER:
            /* USER CODE BEGIN OT_DEVICE_ROLE_ROUTER */
            /* USER CODE END OT_DEVICE_ROLE_ROUTER */
            break;
        case OT_DEVICE_ROLE_LEADER:
            /* USER CODE BEGIN OT_DEVICE_ROLE_LEADER */
            /* USER CODE END OT_DEVICE_ROLE_LEADER */
            break;
        default:
            /* USER CODE BEGIN DEFAULT */
            /* USER CODE END DEFAULT */
            break;
        }
    }
}

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
        APP_DBG("PRODUCT NAME : " PRODUCT_NAME);
        APP_DBG("VENDOR NAME : " VENDOR_NAME);
        APP_DBG("HARDWARE : " HARDWARE_VERSION);
        APP_DBG("SOFTWARE : X-CUBE-MATTER release revision " X_CUBE_MATTER_VERSION);
        APP_DBG("Embedded SW components :");
        APP_DBG("- Matter SDK version : " MATTER_SDK_VERSION);
        APP_DBG("- STM32WB Cube Firmware Version : %d.%d.%d.%d", p_wireless_info->VersionMajor, p_wireless_info->VersionMinor,
                p_wireless_info->VersionSub, p_wireless_info->VersionBranch);
        APP_DBG("**********************************************************");

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

/* USER CODE BEGIN FREERTOS_WRAPPER_FUNCTIONS */

/* USER CODE END FREERTOS_WRAPPER_FUNCTIONS */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

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
        /* Do nothing  CLI is not enable */
    }
    else
    {
        /* Notify M0 that characters have been sent to UART */
        TL_THREAD_CliSendAck();
    }
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
 * @brief  This function is called before sending any ot command to the M0
 *         core. The purpose of this function is to be able to check if
 *         there are no notifications coming from the M0 core which are
 *         pending before sending a new ot command.
 * @param  None
 * @retval None
 */
void Pre_OtCmdProcessing(void)
{
    osSemaphoreAcquire(OtCmdProcessSem, osWaitForever);
}

/**
 * @brief  This function waits for getting an acknowledgment from the M0.
 *
 * @param  None
 * @retval None
 */
static void Wait_Getting_Ack_From_M0(void)
{
    osSemaphoreAcquire(OtCmdAckSem, osWaitForever);
    osSemaphoreRelease(OtCmdProcessSem);
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
    osSemaphoreRelease(OtCmdAckSem);
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

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

/* USER CODE END FD_WRAP_FUNCTIONS */
