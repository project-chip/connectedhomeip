/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_entry.c
 * Description        : Entry application source file for STM32WPAN Middleware.
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
#include "app_entry.h"
#include "AppTask.h"
#include "app_ble.h"
#include "app_common.h"
#include "app_conf.h"
#include "app_debug.h"
#include "app_thread.h"
#include "cmsis_os.h"
#include "dbg_trace.h"
#include "flash_wb.h"
#include "hw_conf.h"
#include "main.h"
#include "shci.h"
#include "shci_tl.h"
#include "ssd1315.h"
#include "stm32_lcd.h"
#include "stm32_lpm.h"
#include "stm32wb5mm_dk_lcd.h"
#include "stm_ext_flash.h"
#include "stm_logging.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
/* POOL_SIZE = 2(TL_PacketHeader_t) + 258 (3(TL_EVT_HDR_SIZE) + 255(Payload size)) */
#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH * 4U * DIVC((sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE), 4U))
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

extern RTC_HandleTypeDef hrtc; /**< RTC handler declaration */
#ifdef __cplusplus
extern "C" {
#endif

PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t EvtPool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t SystemCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t SystemSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t BleSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];
uint8_t g_ot_notification_allowed = 0U;

/* Global variables ----------------------------------------------------------*/
osMutexId_t MtxShciId;
osSemaphoreId_t SemShciId;
osThreadId_t ShciUserEvtProcessId;
osThreadId_t OsPushButtonProcessId;

const osThreadAttr_t ShciUserEvtProcess_attr = { .name       = CFG_SHCI_USER_EVT_PROCESS_NAME,
                                                 .attr_bits  = CFG_SHCI_USER_EVT_PROCESS_ATTR_BITS,
                                                 .cb_mem     = CFG_SHCI_USER_EVT_PROCESS_CB_MEM,
                                                 .cb_size    = CFG_SHCI_USER_EVT_PROCESS_CB_SIZE,
                                                 .stack_mem  = CFG_SHCI_USER_EVT_PROCESS_STACK_MEM,
                                                 .stack_size = CFG_SHCI_USER_EVT_PROCESS_STACK_SIZE,
                                                 .priority   = CFG_SHCI_USER_EVT_PROCESS_PRIORITY };

const osThreadAttr_t PushButtonProcess_attr = { .name       = CFG_PUSH_BUTTON_EVT_PROCESS_NAME,
                                                .attr_bits  = CFG_PUSH_BUTTON_EVT_PROCESS_ATTR_BITS,
                                                .cb_mem     = CFG_PUSH_BUTTON_EVT_PROCESS_CB_MEM,
                                                .cb_size    = CFG_PUSH_BUTTON_EVT_PROCESS_CB_SIZE,
                                                .stack_mem  = CFG_PUSH_BUTTON_EVT_PROCESS_STACK_MEM,
                                                .stack_size = CFG_PUSH_BUTTON_EVT_PROCESS_STACK_SIZE,
                                                .priority   = CFG_PUSH_BUTTON_EVT_PROCESS_PRIORITY };

/* Global function prototypes -----------------------------------------------*/
#if (CFG_DEBUG_TRACE != 0)
size_t DbgTraceWrite(int handle, const unsigned char * buf, size_t bufSize);
#endif

/* USER CODE BEGIN GFP */

/* USER CODE END GFP */

/* Private functions prototypes-----------------------------------------------*/
static void SystemPower_Config(void);
static void Init_Debug(void);
static void APPE_SysStatusNot(SHCI_TL_CmdStatus_t status);
static void APPE_SysUserEvtRx(void * pPayload);
static void APPE_SysEvtReadyProcessing(void);
static void APPE_SysEvtError(SCHI_SystemErrCode_t ErrorCode);
static void ShciUserEvtProcess(void * argument);
static void PushButtonEvtProcess(void * argument);

static void appe_Tl_Init(void);
/* USER CODE BEGIN PFP */
static void Led_Init(void);
static void Button_Init(void);
#if (CFG_HW_EXTPA_ENABLED == 1)
static void ExtPA_Init(void);
#endif
/* USER CODE END PFP */

// Callback function to handle pushbutton to apptask
PushButtonCallback PbCb = NULL;

void APP_ENTRY_PBSetReceiveCallback(PushButtonCallback aCallback)
{
    PbCb = aCallback;
}
/* Functions Definition ------------------------------------------------------*/
void APPE_Init(void)
{
    /* Configure the system Power Mode */
    SystemPower_Config();

    /* Initialize the TimerServer */
    HW_TS_Init(hw_ts_InitMode_Full, &hrtc);

    /* USER CODE BEGIN APPE_Init_1 */
    /* initialize debugger module if supported and debug trace if activated */
    Init_Debug();

    // Init qspi and external flash
    STM_EXT_FLASH_Init();
    // Init nvm
    NM_Init();

    APPD_Init();

    /**
     * The Standby mode should not be entered before the initialization is over
     * The default state of the Low Power Manager is to allow the Standby Mode so an request is needed here
     */
    UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);

    OsPushButtonProcessId = osThreadNew(PushButtonEvtProcess, NULL, &PushButtonProcess_attr);
    Led_Init();
    Button_Init();

    /* USER CODE END APPE_Init_1 */
    /* Initialize all transport layers and start CPU2 which will send back a ready event to CPU1 */
    appe_Tl_Init();

#if (CFG_LCD_SUPPORTED == 1)
    BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
    /* Set LCD Foreground Layer  */
    UTIL_LCD_SetFuncDriver(&LCD_Driver); /* SetFunc before setting device */
    UTIL_LCD_SetDevice(0);               /* SetDevice after funcDriver is set */
    BSP_LCD_Clear(0, SSD1315_COLOR_BLACK);
    BSP_LCD_DisplayOn(0);
    BSP_LCD_Refresh(0);
    UTIL_LCD_SetFont(&Font12);
    /* Set the LCD Text Color */
    UTIL_LCD_SetTextColor(SSD1315_COLOR_WHITE);
    UTIL_LCD_SetBackColor(SSD1315_COLOR_BLACK);
    BSP_LCD_Clear(0, SSD1315_COLOR_BLACK);
    BSP_LCD_Refresh(0);
    UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *) APP_NAME, CENTER_MODE);
    BSP_LCD_Refresh(0);
#endif
    /**
     * From now, the application is waiting for the ready event ( VS_HCI_C2_Ready )
     * received on the system channel before starting the Stack
     * This system event is received with APPE_SysUserEvtRx()
     */
    /* USER CODE BEGIN APPE_Init_2 */
#if (CFG_HW_EXTPA_ENABLED == 1)
    ExtPA_Init();
#endif

    /* USER CODE END APPE_Init_2 */
    return;
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void Init_Debug(void)
{
#if (CFG_DEBUGGER_SUPPORTED == 1)
    /**
     * Keep debugger enabled while in any low power mode
     */
    HAL_DBGMCU_EnableDBGSleepMode();

    /* Enable debugger EXTI lines */
    LL_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);
    LL_C2_EXTI_EnableIT_32_63(LL_EXTI_LINE_48);

#else
    /* Disable debugger EXTI lines */
    LL_EXTI_DisableIT_32_63(LL_EXTI_LINE_48);
    LL_C2_EXTI_DisableIT_32_63(LL_EXTI_LINE_48);

    GPIO_InitTypeDef gpio_config = { 0 };

    gpio_config.Pull = GPIO_NOPULL;
    gpio_config.Mode = GPIO_MODE_ANALOG;

    gpio_config.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &gpio_config);
    __HAL_RCC_GPIOA_CLK_DISABLE();

    gpio_config.Pin = GPIO_PIN_4 | GPIO_PIN_3;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_Init(GPIOB, &gpio_config);
    __HAL_RCC_GPIOB_CLK_DISABLE();

    /**
     * Do not keep debugger enabled while in any low power mode
     */
    HAL_DBGMCU_DisableDBGSleepMode();
    HAL_DBGMCU_DisableDBGStopMode();
    HAL_DBGMCU_DisableDBGStandbyMode();
#endif /* (CFG_DEBUGGER_SUPPORTED == 1) */

#if (CFG_DEBUG_TRACE != 0)
    DbgTraceInit();
#endif

    return;
}

/**
 * @brief  Configure the system for power optimization
 *
 * @note  This API configures the system to be ready for low power mode
 *
 * @param  None
 * @retval None
 */
static void SystemPower_Config(void)
{
    // Before going to stop or standby modes, do the settings so that system clock and IP80215.4 clock
    // start on HSI automatically
    LL_RCC_HSI_EnableAutoFromStop();

    /**
     * Select HSI as system clock source after Wake Up from Stop mode
     */
    LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);

    /* Initialize low power manager */
    UTIL_LPM_Init();

    /* Disable low power mode until INIT is complete */
    UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);
    UTIL_LPM_SetStopMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);

    /* Enable RAM1 (because OT instance.o is located here for Concurrent Mode */
    LL_C2_AHB1_GRP1_EnableClock(LL_C2_AHB1_GRP1_PERIPH_SRAM1);
    LL_C2_AHB1_GRP1_EnableClockSleep(LL_C2_AHB1_GRP1_PERIPH_SRAM1);

    return;
}

static void appe_Tl_Init(void)
{
    TL_MM_Config_t tl_mm_config;
    SHCI_TL_HciInitConf_t SHci_Tl_Init_Conf;

    /**< Reference table initialization */
    TL_Init();

    MtxShciId = osMutexNew(NULL);
    SemShciId = osSemaphoreNew(1, 0, NULL); /*< Create the semaphore and make it busy at initialization */

    /** FreeRTOS system task creation */
    ShciUserEvtProcessId = osThreadNew(ShciUserEvtProcess, NULL, &ShciUserEvtProcess_attr);

    /**< System channel initialization */
    SHci_Tl_Init_Conf.p_cmdbuffer       = (uint8_t *) &SystemCmdBuffer;
    SHci_Tl_Init_Conf.StatusNotCallBack = APPE_SysStatusNot;
    shci_init(APPE_SysUserEvtRx, (void *) &SHci_Tl_Init_Conf);

    /**< Memory Manager channel initialization */
    tl_mm_config.p_BleSpareEvtBuffer    = BleSpareEvtBuffer;
    tl_mm_config.p_SystemSpareEvtBuffer = SystemSpareEvtBuffer;
    tl_mm_config.p_AsynchEvtPool        = EvtPool;
    tl_mm_config.AsynchEvtPoolSize      = POOL_SIZE;
    TL_MM_Init(&tl_mm_config);

    TL_Enable();

    return;
}

static void APPE_SysStatusNot(SHCI_TL_CmdStatus_t status)
{
    switch (status)
    {
    case SHCI_TL_CmdBusy:
        osMutexAcquire(MtxShciId, osWaitForever);
        break;

    case SHCI_TL_CmdAvailable:
        osMutexRelease(MtxShciId);
        break;

    default:
        break;
    }
    return;
}

/**
 * The type of the payload for a system user event is tSHCI_UserEvtRxParam
 * When the system event is both :
 *    - a ready event (subevtcode = SHCI_SUB_EVT_CODE_READY)
 *    - reported by the FUS (sysevt_ready_rsp == FUS_FW_RUNNING)
 * The buffer shall not be released
 * (eg ((tSHCI_UserEvtRxParam*)pPayload)->status shall be set to SHCI_TL_UserEventFlow_Disable )
 * When the status is not filled, the buffer is released by default
 */
static void APPE_SysUserEvtRx(void * pPayload)
{
    TL_AsynchEvt_t * p_sys_event;
    p_sys_event = (TL_AsynchEvt_t *) (((tSHCI_UserEvtRxParam *) pPayload)->pckt->evtserial.evt.payload);

    switch (p_sys_event->subevtcode)
    {
    case SHCI_SUB_EVT_CODE_READY:
        APPE_SysEvtReadyProcessing();
        break;

    case SHCI_SUB_EVT_ERROR_NOTIF:
        APPE_SysEvtError((SCHI_SystemErrCode_t) (p_sys_event->payload[0]));
        break;

    default:
        break;
    }
    return;
}

/**
 * @brief Notify a system error coming from the M0 firmware
 * @param  ErrorCode  : errorCode detected by the M0 firmware
 *
 * @retval None
 */
static void APPE_SysEvtError(SCHI_SystemErrCode_t ErrorCode)
{
    switch (ErrorCode)
    {
    case ERR_THREAD_LLD_FATAL_ERROR:
        APP_DBG("** ERR_THREAD : LLD_FATAL_ERROR \n");
        break;

    case ERR_THREAD_UNKNOWN_CMD:
        APP_DBG("** ERR_THREAD : UNKNOWN_CMD \n");
        break;

    default:
        APP_DBG("** ERR_THREAD : ErroCode=%d \n", ErrorCode);
        break;
    }
    return;
}

static void APPE_SysEvtReadyProcessing(void)
{
    /* Traces channel initialization */
    APPD_EnableCPU2();

    /* Configuration to CPU2 */
    SHCI_C2_CONFIG_Cmd_Param_t config_param = { 0 };
    uint32_t Ot_NVMAddr                     = 0;
    /* In the Context of Dynamic Concurrent mode, the Init and start of each stack must be split and executed
     * in the following order :
     * APP_BLE_Init_Dyn_1()    : BLE Stack Init until it's ready to start ADV
     * APP_THREAD_Init_Dyn_1() : Thread Stack Init until it's ready to be configured (default channel, PID, etc...)
     * APP_BLE_Init_Dyn_2()    : Start ADV
     * APP_THREAD_Init_Dyn_2() : Thread Stack configuration (default channel, PID, etc...) to be able to start scanning
     *                           or joining a Thread Network
     */
    APP_DBG("1- Initialisation of BLE Stack...");
    APP_BLE_Init_Dyn_1();
    /* Set the address that will be used by OT stack for NVM data management */
    if (NM_GetOtNVMAddr(&Ot_NVMAddr) == NVM_OK)
    {
        config_param.ThreadNvmRamAddress = Ot_NVMAddr;
        (void) SHCI_C2_Config(&config_param);
    }

    APP_DBG("2- Initialisation of OpenThread Stack. FW info :");
    APP_THREAD_Init_Dyn_1();
    APP_BLE_Init_Dyn_2();
    APP_THREAD_Init_Dyn_2();
    APP_DBG("Start init matter");
    GetAppTask().StartAppTask();
#if (CFG_LPM_SUPPORTED == 1)
    /* Thread stack is initialized, low power mode can be enabled */
    UTIL_LPM_SetOffMode(1U << CFG_LPM_APP, UTIL_LPM_ENABLE);
    UTIL_LPM_SetStopMode(1U << CFG_LPM_APP, UTIL_LPM_ENABLE);
#endif
    return;
}

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
 *************************************************************/
static void ShciUserEvtProcess(void * argument)
{
    UNUSED(argument);
    for (;;)
    {
        /* USER CODE BEGIN SHCI_USER_EVT_PROCESS_1 */

        /* USER CODE END SHCI_USER_EVT_PROCESS_1 */
        osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
        shci_user_evt_proc();
        /* USER CODE BEGIN SHCI_USER_EVT_PROCESS_2 */

        /* USER CODE END SHCI_USER_EVT_PROCESS_2 */
    }
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */
static void Led_Init(void)
{
#if (CFG_LED_SUPPORTED == 1U)
    /**
     * Leds Initialization
     */

#endif

    return;
}

static void Button_Init(void)
{

#if ((CFG_BUTTON_SUPPORTED == 1U) || (CFG_HW_EXTPA_ENABLED == 1))
    /**
     * Button Initialization
     */

    BSP_PB_Init(BUTTON_USER1, BUTTON_MODE_EXTI);
    BSP_PB_Init(BUTTON_USER2, BUTTON_MODE_EXTI);

#endif

    return;
}

#if (CFG_HW_EXTPA_ENABLED == 1)
static void ExtPA_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // configure the GPIO PB0 in AF6 to be used as RF_TX_MOD_EXT_PA
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_RF_DTB0;
    GPIO_InitStruct.Pin       = GPIO_EXT_PA_TX_PIN;
    HAL_GPIO_Init(GPIO_EXT_PA_TX_PORT, &GPIO_InitStruct);

    // configure the GPIO which will be managed by M0 stack to enable Ext PA
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin   = GPIO_EXT_PA_EN_PIN;
    HAL_GPIO_Init(GPIO_EXT_PA_EN_PORT, &GPIO_InitStruct);

    // Indicate to M0 which GPIO must be managed
    SHCI_C2_ExtpaConfig((uint32_t) GPIO_EXT_PA_EN_PORT, GPIO_EXT_PA_EN_PIN, EXT_PA_ENABLED_HIGH, EXT_PA_ENABLED);
}
#endif /* CFG_HW_EXTPA_ENABLED */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
static void PushButtonEvtProcess(void * argument)
{
    UNUSED(argument);
    uint32_t ButtonPressed = 0;

    for (;;)
    {
        /* USER CODE BEGIN SHCI_USER_EVT_PROCESS_1 */
        /* USER CODE END SHCI_USER_EVT_PROCESS_1 */
        ButtonPressed = osThreadFlagsWait(3, osFlagsWaitAny, osWaitForever);
        Push_Button_st Message;
        if (1 == ButtonPressed)
        {
            Message.Pushed_Button = BUTTON_USER1;
            Message.State         = 1;
        }
        if (2 == ButtonPressed)
        {
            Message.Pushed_Button = BUTTON_USER2;
            Message.State         = 2;
        }
        PbCb(&Message); // call matter callback to handle push button
        /* USER CODE BEGIN SHCI_USER_EVT_PROCESS_2 */

        /* USER CODE END SHCI_USER_EVT_PROCESS_2 */
    }
}

void shci_notify_asynch_evt(void * pdata)
{
    UNUSED(pdata);
    osThreadFlagsSet(ShciUserEvtProcessId, 1);
    return;
}

void shci_cmd_resp_release(uint32_t flag)
{
    UNUSED(flag);
    osSemaphoreRelease(SemShciId);
    return;
}

void shci_cmd_resp_wait(uint32_t timeout)
{
    osSemaphoreAcquire(SemShciId, pdMS_TO_TICKS(timeout));
    return;
}

/* Received trace buffer from M0 */
void TL_TRACES_EvtReceived(TL_EvtPacket_t * hcievt)
{
#if (CFG_DEBUG_TRACE != 0)
    /* Call write/print function using DMA from dbg_trace */
    /* - Cast to TL_AsynchEvt_t* to get "real" payload (without Sub Evt code 2bytes),
     - (-2) to size to remove Sub Evt Code */
    DbgTraceWrite(1U, (const unsigned char *) ((TL_AsynchEvt_t *) (hcievt->evtserial.evt.payload))->payload,
                  hcievt->evtserial.evt.plen - 2U);
#endif /* CFG_DEBUG_TRACE */
    /* Release buffer */
    TL_MM_EvtDone(hcievt);
}
/**
 * @brief  Initialization of the trace mechanism
 * @param  None
 * @retval None
 */
#if (CFG_DEBUG_TRACE != 0)
void DbgOutputInit(void)
{
#if (CFG_HW_USART1_ENABLED == 1)
    HW_UART_Init(CFG_DEBUG_TRACE_UART);
#endif
    return;
}

/**
 * @brief  Management of the traces
 * @param  p_data : data
 * @param  size : size
 * @param  call-back :
 * @retval None
 */
void DbgOutputTraces(uint8_t * p_data, uint16_t size, void (*cb)(void))
{
    HW_UART_Transmit_DMA(CFG_DEBUG_TRACE_UART, p_data, size, cb);

    return;
}
#endif

/**
 * @brief This function manage the Push button action
 * @param  GPIO_Pin : GPIO pin which has been activated
 * @retval None
 */
void BSP_PB_Callback(Button_TypeDef Button)
{
    switch (Button)
    {
    case BUTTON_USER1:
        APP_DBG("BUTTON 1 PUSHED !");
        osThreadFlagsSet(OsPushButtonProcessId, 1);
        break;

    case BUTTON_USER2:
        APP_DBG("BUTTON 2 PUSHED !");
        osThreadFlagsSet(OsPushButtonProcessId, 2);
        break;

    default:
        break;
    }

    return;
}

#ifdef __cplusplus
}
#endif
