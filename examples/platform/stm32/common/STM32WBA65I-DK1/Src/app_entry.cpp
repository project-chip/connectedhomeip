/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.c
  * @author  MCD Application Team
  * @brief   Entry point of the application
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
#include "app_common.h"
#include "app_conf.h"
#include "main.h"
#include "app_entry.h"
#include "cmsis_os2.h"
#include "stm32_rtos.h"
#include "task.h"
#if (CFG_LPM_LEVEL != 0)
#include "stm32_lpm.h"
#endif /* (CFG_LPM_LEVEL != 0) */
#include "stm32_timer.h"
#include "stm32_mm.h"
#if (CFG_LOG_SUPPORTED != 0)
#include "stm32_adv_trace.h"
#include "serial_cmd_interpreter.h"
#endif /* CFG_LOG_SUPPORTED */
#include "ll_sys_if.h"
#include "app_ble.h"
#include "app_thread.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "app_sys.h"
#include "otp.h"
#include "scm.h"
#include "bpka.h"
#include "ll_sys.h"
#include "advanced_memory_manager.h"
#include "flash_driver.h"
#include "flash_manager.h"
#include "simple_nvm_arbiter.h"
#include "app_debug.h"
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
#include "adc_ctrl.h"
#include "temp_measurement.h"
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
#include "timer_if.h"
extern void xPortSysTickHandler (void);
/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_bsp.h"
#include "stm32wba65i_discovery.h"
#include "stm32wba65i_discovery_bus.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32wba65i_discovery_lcd.h"
#include "stm32_lcd.h"
#endif /* CFG_LCD_SUPPORTED */
#include "stm32_lpm.h"
#include "crc_ctrl.h"
#include "psa/crypto.h"
#if (OTA_SUPPORT == 1)
#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
#include "stm_ota_spi_flash.h"
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */
#endif /* (OTA_SUPPORT == 1) */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */



/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private constants ---------------------------------------------------------*/
/* USER CODE BEGIN PC */

/* USER CODE END PC */

/* Private variables ---------------------------------------------------------*/


#if ( CFG_LPM_LEVEL != 0)
static bool system_startup_done = FALSE;
/* Holds maximum number of FreeRTOS tick periods that can be suppressed */
static uint32_t maximumPossibleSuppressedTicks = 0;
#endif /* ( CFG_LPM_LEVEL != 0) */

#if (CFG_LOG_SUPPORTED != 0)
/* Log configuration */
static Log_Module_t Log_Module_Config = { .verbose_level = APPLI_CONFIG_LOG_LEVEL, .region = LOG_REGION_ALL_REGIONS };
#endif /* (CFG_LOG_SUPPORTED != 0) */

/* AMM configuration */
static uint32_t AMM_Pool[CFG_AMM_POOL_SIZE];
static AMM_VirtualMemoryConfig_t vmConfig[CFG_AMM_VIRTUAL_MEMORY_NUMBER] =
{
  /* Virtual Memory #1 */
  {
    .Id = CFG_AMM_VIRTUAL_STACK_BLE,
    .BufferSize = CFG_AMM_VIRTUAL_STACK_BLE_BUFFER_SIZE
  },
  /* Virtual Memory #2 */
  {
    .Id = CFG_AMM_VIRTUAL_APP_BLE,
    .BufferSize = CFG_AMM_VIRTUAL_APP_BLE_BUFFER_SIZE
  },
};

static AMM_InitParameters_t ammInitConfig =
{
  .p_PoolAddr = AMM_Pool,
  .PoolSize = CFG_AMM_POOL_SIZE,
  .VirtualMemoryNumber = CFG_AMM_VIRTUAL_MEMORY_NUMBER,
  .p_VirtualMemoryConfigList = vmConfig
};


/* USER CODE BEGIN PV */


/* FreeRtos Random Process stacks attributes */
const osThreadAttr_t stRandomProcessTaskAttributes = 
{
  .name = "Hw Random Task",
  .stack_size = TASK_HW_RNG_STACK_SIZE,
  .priority = CFG_TASK_PRIO_HW_RNG
};

/* FreeRtos Amm Background Task stacks attributes */
const osThreadAttr_t stAmmBckgTaskAttributes = 
{
  .name = "AMM Background Task",
  .stack_size = TASK_AMM_BCKG_STACK_SIZE,
  .priority = CFG_TASK_PRIO_AMM_BCKG
};

/* FreeRtos BPKA Background Task stacks attributes */
const osThreadAttr_t stBPKATaskAttributes = 
{
  .name = "BPKA Task",
  .stack_size = TASK_BPKA_STACK_SIZE,
  .priority = CFG_TASK_PRIO_BPKA
};



/* FreeRtos Flash Manager Background Task stacks attributes */
const osThreadAttr_t stFlashManagerBackgroundTaskAttributes = 
{
  .name = "Flash Manager Background Task",
  .stack_size = TASK_FLASH_MANAGER_BCKGND_STACK_SIZE,
  .priority = CFG_TASK_PRIO_FLASH_MANAGER_BCKGND
};

osSemaphoreId_t       BpkaSemaphore;
osThreadId_t          BpkaTaskId;

osSemaphoreId_t       AppliInitEndSemaphore, HwRngSemaphore;
osThreadId_t          AppliInitTaskId, RandomProcessTaskId;

osSemaphoreId_t       AmmBckgSemaphore, FlashMangerReqSemaphore;
osThreadId_t          AmmBckgTaskId, FmBackgroundTaskId;



static UTIL_TIMER_Object_t  TimerOSwakeup_Id;
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/
/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/
static void Config_HSE(void);
static void RNG_Init( void );
static void System_Init( void );
static void SystemPower_Config( void );
static void TimerOSwakeupCB(void *arg);
#if ( CFG_LPM_LEVEL != 0)
static void preOSsleepProcessing(uint32_t expectedIdleTime);
static void postOSsleepProcessing(uint32_t expectedIdleTime);
static uint32_t getCurrentTime(void);
#endif /* CFG_LPM_LEVEL */

/* Callback function to handle pushbutton to AppTask */
PushButtonCallback PbCb = NULL;

void APPE_PushButtonSetReceiveCb(PushButtonCallback aCallback)
{
	PbCb = aCallback;
}

/**
 * @brief Joystick Up Action
 *
 * @param  None
 * @return None
 */
void APP_BSP_JoystickUpAction( void )
{
	ButtonDesc_t Message;
	Message.button = B1;
	Message.State = 1;
	PbCb(&Message); //call matter callback to handle push button
}

/**
 * @brief Joystick Select Action
 *
 * @param  None
 * @return None
 */
void APP_BSP_JoystickSelectAction( void )
{
	 ButtonDesc_t Message;
	 Message.button = B2;
	 Message.State = 1;
	 PbCb(&Message); //call matter callback to handle push button
}
/**
 * @brief Wrapper for init function of the MM for the AMM
 *
 * @param p_PoolAddr: Address of the pool to use - Not use -
 * @param PoolSize: Size of the pool - Not use -
 *
 * @return None
 */
static void AMM_WrapperInit (uint32_t * const p_PoolAddr, const uint32_t PoolSize);

/**
 * @brief Wrapper for allocate function of the MM for the AMM
 *
 * @param BufferSize
 *
 * @return Allocated buffer
 */
static uint32_t * AMM_WrapperAllocate (const uint32_t BufferSize);

/**
 * @brief Wrapper for free function of the MM for the AMM
 *
 * @param p_BufferAddr
 *
 * @return None
 */
static void AMM_WrapperFree (uint32_t * const p_BufferAddr);
static void BPKA_Task(void *argument);
static void AMM_Background_Task(void *argument);
static void FM_Background_Task(void *argument);
static void HW_RNG_Process_Task(void *argument);


/* USER CODE BEGIN PFP */

#if (CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE != 0)
static void LogTimestamp(uint8_t * pData, uint16_t * piSize);
#endif /*CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE */
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/
/**
 * @brief   System Initialisation.
 */
void MX_APPE_Config(void)
{
  /* Configure HSE Tuning */
  Config_HSE();
}

/**
 * @brief   Background Task for AMM.
 */
void AMM_Background_Task(void *argument)
{
  UNUSED( argument );

  while(1)
  {
    osSemaphoreAcquire(AmmBckgSemaphore, osWaitForever);
    AMM_BackgroundProcess();
    osThreadYield();
  }
}

/**
 * @brief   Background Task for Flash Manager.
 */
void FM_Background_Task(void *argument)
{
  UNUSED( argument );

  while(1)
  {
    osSemaphoreAcquire(FlashMangerReqSemaphore, osWaitForever);
    FM_BackgroundProcess();
    osThreadYield();
  }
}

/**
 * @brief   Background Task for BPKA.
 */
void BPKA_Task(void *argument)
{
  UNUSED( argument );

  while(1)
  {
    osSemaphoreAcquire(BpkaSemaphore, osWaitForever);
    osMutexAcquire(LinkLayerMutex, osWaitForever);
    BPKA_BG_Process();
    osMutexRelease(LinkLayerMutex);
    osThreadYield();
  }
}


/**
 * @brief   System Initialisation.
 */
uint32_t MX_APPE_Init(void *p_param)
{
  APP_DEBUG_SIGNAL_SET(APP_APPE_INIT);

  UNUSED(p_param);

  /* System initialization */
  System_Init();

  /* Configure the system Power Mode */
  SystemPower_Config();


  APP_DBG("FW info :\n");
  APP_DBG("**********************************************************\n");
  APP_DBG("PRODUCT NAME : " PRODUCT_NAME "\n"  );
  APP_DBG("VENDOR NAME : " VENDOR_NAME "\n" );
  #if defined(USE_STM32WBA65I_DK1) // TODO: to be checked once WBA6 DK header file released
    APP_DBG("HARDWARE : STM32WBA65MM-DK\n" );
  #elif defined(STM32WBA65xx)
   	APP_DBG("HARDWARE : STM32WBA65xx\n" );
  #else
   	APP_DBG("HARDWARE : UNSPECIFIED\n" );
  #endif
  APP_DBG("SOFTWARE : X-CUBE-MATTER release revision " X_CUBE_MATTER_VERSION "\n" );
  APP_DBG("Embedded SW components :\n");
  APP_DBG("- Matter SDK version : " MATTER_SDK_VERSION "\n" );
  APP_DBG("**********************************************************\n");

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  /* Initialize the Temperature measurement */
  TEMPMEAS_Init ();
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

  /* Initialize the Advance Memory Manager */
  AMM_Init (&ammInitConfig);

  /* Create thread for AMM background and semaphore to control it*/
  AmmBckgSemaphore = osSemaphoreNew( 1, 0, NULL );
  if ( AmmBckgSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : AMM BACKGROUND SEMAPHORE CREATION FAILED" );
    while(1);
  }
  
  AmmBckgTaskId = osThreadNew( AMM_Background_Task, NULL, &stAmmBckgTaskAttributes );
  if ( AmmBckgTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS :  AMM BACKGROUND TASK CREATION FAILED" );
    while(1);
  } 
  
  CRCCTRL_Init();


  /* Register Semaphore to trigger FM_Background_Task */
  FlashMangerReqSemaphore = osSemaphoreNew( 1, 0, NULL );
  if ( FlashMangerReqSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : FLASH MANAGER BACKGROUND SEMAPHORE CREATION FAILED" );
    Error_Handler();
  }
  
  /* Create flash manager Task over FreeRTOS */
  FmBackgroundTaskId = osThreadNew(FM_Background_Task, NULL, &stFlashManagerBackgroundTaskAttributes);
  if ( FmBackgroundTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : FLASH MANAGER BACKGROUND THREAD CREATION FAILED" );
    Error_Handler();
  }

  /* USER CODE BEGIN APPE_Init_1 */


  /* USER CODE END APPE_Init_1 */
  
  /* Create thread for BPKA and semaphore to control it*/
  BpkaSemaphore = osSemaphoreNew( 1, 0, NULL );
  if ( BpkaSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : BPKA SEMAPHORE CREATION FAILED" );
    while(1);
  }
  
  BpkaTaskId = osThreadNew( BPKA_Task, NULL, &stBPKATaskAttributes );
  if ( BpkaTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : BPKA TASK CREATION FAILED" );
    while(1);
  }

  BPKA_Reset( );

  RNG_Init();

  APP_DBG("Start init ble\n");
  APP_BLE_Init();
  
  /* create a SW timer to wakeup system from low power */
  UTIL_TIMER_Create(&TimerOSwakeup_Id,
                    0,
                    UTIL_TIMER_ONESHOT,
                    &TimerOSwakeupCB, 0);
  
  /* Disable RFTS Bypass for flash operation - Since LL has not started yet */
  FD_SetStatus (FD_FLASHACCESS_RFTS_BYPASS, LL_FLASH_DISABLE);

  ll_sys_config_params();


  /* USER CODE BEGIN APPE_Init_2 */
  /* USER CODE END APPE_Init_2 */
  APP_DEBUG_SIGNAL_RESET(APP_APPE_INIT);


  // Init nvm
  NM_Init();

#if (OTA_SUPPORT == 1)
#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
    STM_OTA_SPI_FLASH_Init();
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */
#endif /* (OTA_SUPPORT == 1) */

  APP_THREAD_Init();

#if (CONFIG_STM32_FACTORY_DATA_ENABLE == 1)
  /* Init PSA */
  psa_status_t retval;
   retval = psa_crypto_init();
  if(retval != PSA_SUCCESS)
  {
     APP_DBG("PSA Init Failed\n");
     Error_Handler();
  }
#endif /* CONFIG_STM32_FACTORY_DATA_ENABLE */

#if (CFG_LED_SUPPORTED == 1)
  APP_BSP_LedInit();
#endif /* (CFG_LED_SUPPORTED == 1) */
#if (CFG_JOYSTICK_SUPPORTED == 1)
  APP_BSP_JoystickInit();
#endif /* CFG_JOYSTICK_SUPPORTED */

  /* Init matter */
  APP_DBG("Start init matter\n");
  CHIP_ERROR err = GetAppTask().InitMatter();
  if (err != CHIP_NO_ERROR) {
      APP_DBG("Matter init failed\n");
  }

  GetAppTask().StartAppTask();

  return WPAN_SUCCESS;
}

/* USER CODE BEGIN FD */


/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Configure HSE by read this Tuning from OTP
 *
 */
static void Config_HSE(void)
{
  OTP_Data_s* otp_ptr = NULL;

  /* Read HSE_Tuning from OTP */
  if (OTP_Read(DEFAULT_OTP_IDX, &otp_ptr) != HAL_OK)
  {
    /* OTP no present in flash, apply default gain */
    HAL_RCCEx_HSESetTrimming(0x0C);
  }
  else
  {
    HAL_RCCEx_HSESetTrimming(otp_ptr->hsetune);
  }
}

/**
 *
 */
static void System_Init( void )
{

/*
 * ENABLE_SDK_BOOT_REASON_SUPPORT is supported.
 * The RCC reset flags are cleared after determining the boot reason type
 */
#if (ENABLE_SDK_BOOT_REASON_SUPPORT == 0)
  /* Clear RCC RESET flag */
  LL_RCC_ClearResetFlags();
#endif /* ENABLE_SDK_BOOT_REASON_SUPPORT == 0 */

  UTIL_TIMER_Init();

  /* Enable wakeup out of standby from RTC ( UTIL_TIMER )*/
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);

#if (CFG_LOG_SUPPORTED != 0)
  /* Initialize the logs ( using the USART ) */
  Log_Module_Init( Log_Module_Config );
#if (CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE != 0)
   Log_Module_RegisterTimeStampFunction(LogTimestamp);
#endif /*CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE */
  /* Initialize the Command Interpreter */
  Serial_CMD_Interpreter_Init();
#endif  /* (CFG_LOG_SUPPORTED != 0) */

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  ADCCTRL_Init ();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

#if ( CFG_LPM_LEVEL != 0)
  system_startup_done = TRUE;
#endif /* ( CFG_LPM_LEVEL != 0) */

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
#if (CFG_SCM_SUPPORTED == 1)
  /* Initialize System Clock Manager */
  scm_init();
#endif /* CFG_SCM_SUPPORTED */

#if (CFG_DEBUGGER_LEVEL == 0)
  /* Pins used by SerialWire Debug are now analog input */
  GPIO_InitTypeDef DbgIOsInit = {0};
  DbgIOsInit.Mode = GPIO_MODE_ANALOG;
  DbgIOsInit.Pull = GPIO_NOPULL;
  DbgIOsInit.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &DbgIOsInit);

  DbgIOsInit.Mode = GPIO_MODE_ANALOG;
  DbgIOsInit.Pull = GPIO_NOPULL;
  DbgIOsInit.Pin = GPIO_PIN_3|GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &DbgIOsInit);
#endif /* CFG_DEBUGGER_LEVEL */

  /* Configure Vcore supply */
  if ( HAL_PWREx_ConfigSupply( CFG_CORE_SUPPLY ) != HAL_OK )
  {
    Error_Handler();
  }

#if (CFG_LPM_LEVEL != 0)
  /* Initialize low Power Manager. By default enabled */
  UTIL_LPM_Init();

#if (CFG_LPM_STDBY_SUPPORTED == 1)
  /* Enable SRAM1, SRAM2 and RADIO retention*/
  LL_PWR_SetSRAM1SBRetention(LL_PWR_SRAM1_SB_FULL_RETENTION);
  LL_PWR_SetSRAM2SBRetention(LL_PWR_SRAM2_SB_FULL_RETENTION);
  LL_PWR_SetRadioSBRetention(LL_PWR_RADIO_SB_FULL_RETENTION); /* Retain sleep timer configuration */

#else /* (CFG_LPM_STDBY_SUPPORTED == 1) */
  UTIL_LPM_SetOffMode(1U << CFG_LPM_APP, UTIL_LPM_DISABLE);
#endif /* (CFG_LPM_STDBY_SUPPORTED == 1) */
#endif /* (CFG_LPM_LEVEL != 0)  */
}

static void HW_RNG_Process_Task( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire(HwRngSemaphore, osWaitForever);
    HW_RNG_Process();
    osThreadYield();
  }
}

/**
 * @brief Initialize Random Number Generator module
 */
static void RNG_Init(void)
{
  // -- Init & Start Random Generation --
  HW_RNG_Start();

  /* Register Semaphore to launch the Random Process */
  HwRngSemaphore = osSemaphoreNew( 1, 0, NULL ); 
  if ( HwRngSemaphore == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : RANDOM PROCESS SEMAPHORE CREATION FAILED" );
    while(1);
  }
  
  /* Create the Random Process Thread */
  RandomProcessTaskId = osThreadNew( HW_RNG_Process_Task, NULL, &stRandomProcessTaskAttributes );
  if ( RandomProcessTaskId == NULL )
  { 
    APP_DBG( "ERROR FREERTOS : RANDOM PROCESS TASK CREATION FAILED" );
    while(1);
  }
}

static void AMM_WrapperInit (uint32_t * const p_PoolAddr, const uint32_t PoolSize)
{
  UTIL_MM_Init ((uint8_t *)p_PoolAddr, ((size_t)PoolSize * sizeof(uint32_t)));
}

static uint32_t * AMM_WrapperAllocate (const uint32_t BufferSize)
{
  return (uint32_t *)UTIL_MM_GetBuffer (((size_t)BufferSize * sizeof(uint32_t)));
}

static void AMM_WrapperFree (uint32_t * const p_BufferAddr)
{
  UTIL_MM_ReleaseBuffer ((void *)p_BufferAddr);
}


/* OS wakeup callback */
static void TimerOSwakeupCB(void *arg)
{
  /* USER CODE BEGIN TimerOSwakeupCB */

  /* USER CODE END TimerOSwakeupCB */
  return;
}

#if ( CFG_LPM_LEVEL != 0)
static void preOSsleepProcessing(uint32_t expectedIdleTime)
{
  LL_PWR_ClearFlag_STOP();

  if ( ( system_startup_done != FALSE ) && ( UTIL_LPM_GetMode() == UTIL_LPM_OFFMODE ) )
  {
    APP_SYS_BLE_EnterDeepSleep();
  }

  LL_RCC_ClearResetFlags();

#if defined(STM32WBAXX_SI_CUT1_0)
  /* Wait until HSE is ready */
  while (LL_RCC_HSE_IsReady() == 0);

  UTILS_ENTER_LIMITED_CRITICAL_SECTION(RCC_INTR_PRIO << 4U);
  scm_hserdy_isr();
  UTILS_EXIT_LIMITED_CRITICAL_SECTION();
#endif /* STM32WBAXX_SI_CUT1_0 */

  UTIL_LPM_EnterLowPower(); /* WFI instruction call is inside this API */
}

static void postOSsleepProcessing(uint32_t expectedIdleTime)
{
  UTIL_TIMER_Stop(&TimerOSwakeup_Id);

  LL_AHB5_GRP1_EnableClock(LL_AHB5_GRP1_PERIPH_RADIO);
  ll_sys_dp_slp_exit();
}

/* return current time since boot, continue to count in standby low power mode */
static uint32_t getCurrentTime(void)
{
  return TIMER_IF_GetTimerValue();
}
#endif /* ( CFG_LPM_LEVEL != 0) */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */




/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

void BPKACB_Process( void )
{
  osSemaphoreRelease(BpkaSemaphore);  
}

/**
 * @brief Callback used by 'Random Generator' to launch Task to generate Random Numbers
 */
void HWCB_RNG_Process( void )
{
  osSemaphoreRelease(HwRngSemaphore);
}

void AMM_RegisterBasicMemoryManager (AMM_BasicMemoryManagerFunctions_t * const p_BasicMemoryManagerFunctions)
{
  /* Fulfill the function handle */
  p_BasicMemoryManagerFunctions->Init = AMM_WrapperInit;
  p_BasicMemoryManagerFunctions->Allocate = AMM_WrapperAllocate;
  p_BasicMemoryManagerFunctions->Free = AMM_WrapperFree;
}

void AMM_ProcessRequest (void)
{
  /* Ask for AMM background task scheduling */
  osSemaphoreRelease(AmmBckgSemaphore);
}

void FM_ProcessRequest (void)
{
  /* Schedule the background process */
  osSemaphoreRelease(FlashMangerReqSemaphore);
}

#if (CFG_LOG_SUPPORTED != 0)
/**
 *
 */
void RNG_KERNEL_CLK_OFF(void)
{
  /* RNG module may not switch off HSI clock when traces are used */

  /* USER CODE BEGIN RNG_KERNEL_CLK_OFF */

  /* USER CODE END RNG_KERNEL_CLK_OFF */
}

void SCM_HSI_CLK_OFF(void)
{
  /* SCM module may not switch off HSI clock when traces are used */

  /* USER CODE BEGIN SCM_HSI_CLK_OFF */

  /* USER CODE END SCM_HSI_CLK_OFF */
}

void UTIL_ADV_TRACE_PreSendHook(void)
{
#if (CFG_LPM_LEVEL != 0)
  /* Disable Stop mode before sending a LOG message over UART */
  UTIL_LPM_SetStopMode(1U << CFG_LPM_LOG, UTIL_LPM_DISABLE);
#endif /* (CFG_LPM_LEVEL != 0) */
  /* USER CODE BEGIN UTIL_ADV_TRACE_PreSendHook */

  /* USER CODE END UTIL_ADV_TRACE_PreSendHook */
}

void UTIL_ADV_TRACE_PostSendHook(void)
{
#if (CFG_LPM_LEVEL != 0)
  /* Enable Stop mode after LOG message over UART sent */
  UTIL_LPM_SetStopMode(1U << CFG_LPM_LOG, UTIL_LPM_ENABLE);
#endif /* (CFG_LPM_LEVEL != 0) */
  /* USER CODE BEGIN UTIL_ADV_TRACE_PostSendHook */

  /* USER CODE END UTIL_ADV_TRACE_PostSendHook */
}

#endif /* (CFG_LOG_SUPPORTED != 0) */

#if ( CFG_LPM_LEVEL != 0)
void vPortSuppressTicksAndSleep( uint32_t xExpectedIdleTime )
{
  uint32_t lowPowerTimeBeforeSleep, lowPowerTimeAfterSleep;
  eSleepModeStatus eSleepStatus;

  /* Stop the timer that is generating the OS tick interrupt. */
  UTIL_TIMER_Stop(&TimerOStick_Id);

  /* Make sure the SysTick reload value does not overflow the counter. */
  if( xExpectedIdleTime > maximumPossibleSuppressedTicks )
  {
    xExpectedIdleTime = maximumPossibleSuppressedTicks;
  }

  /* Enter a critical section but don't use the taskENTER_CRITICAL()
   * method as that will mask interrupts that should exit sleep mode. */
  __asm volatile ( "cpsid i" ::: "memory" );
  __asm volatile ( "dsb" );
  __asm volatile ( "isb" );

  eSleepStatus = eTaskConfirmSleepModeStatus();

  /* If a context switch is pending or a task is waiting for the scheduler
   * to be unsuspended then abandon the low power entry. */
  if( eSleepStatus == eAbortSleep )
  {
    /* Restart the timer that is generating the OS tick interrupt. */
    UTIL_TIMER_StartWithPeriod(&TimerOStick_Id, portTICK_PERIOD_MS);

    /* Re-enable interrupts - see comments above the cpsid instruction above. */
    __asm volatile ( "cpsie i" ::: "memory" );
  }
  else
  {
    if( eSleepStatus == eNoTasksWaitingTimeout )
    {
      /* It is not necessary to configure an interrupt to bring the
         microcontroller out of its low power state at a fixed time in the
         future. */
      preOSsleepProcessing(xExpectedIdleTime); /* WFI instruction call is inside this API */
      postOSsleepProcessing(xExpectedIdleTime);
    }
    else
    {
      /* Configure an interrupt to bring the microcontroller out of its low
         power state at the time the kernel next needs to execute. The
         interrupt must be generated from a source that remains operational
         when the microcontroller is in a low power state. */
      UTIL_TIMER_StartWithPeriod(&TimerOSwakeup_Id, (xExpectedIdleTime - 1) * portTICK_PERIOD_MS);

      /* Read the current time from RTC, maintainned in standby */
      lowPowerTimeBeforeSleep = getCurrentTime();

      /* Enter the low power state. */
      preOSsleepProcessing(xExpectedIdleTime); /* WFI instruction call is inside this API */
      postOSsleepProcessing(xExpectedIdleTime);

      /* Determine how long the microcontroller was actually in a low power
         state for, which will be less than xExpectedIdleTime if the
         microcontroller was brought out of low power mode by an interrupt
         other than that configured by the vSetWakeTimeInterrupt() call.
         Note that the scheduler is suspended before
         portSUPPRESS_TICKS_AND_SLEEP() is called, and resumed when
         portSUPPRESS_TICKS_AND_SLEEP() returns.  Therefore no other tasks will
         execute until this function completes. */
      lowPowerTimeAfterSleep = getCurrentTime();

      /* Correct the kernel tick count to account for the time spent in its low power state. */
      vTaskStepTick( TIMER_IF_Convert_Tick2ms(lowPowerTimeAfterSleep - lowPowerTimeBeforeSleep) / portTICK_PERIOD_MS );

    }
    /* Re-enable interrupts to allow the interrupt that brought the MCU
     * out of sleep mode to execute immediately.  See comments above
     * the cpsid instruction above. */
    __asm volatile ( "cpsie i" ::: "memory" );
    __asm volatile ( "dsb" );
    __asm volatile ( "isb" );

    /* Restart the timer that is generating the OS tick interrupt. */
    UTIL_TIMER_StartWithPeriod(&TimerOStick_Id, portTICK_PERIOD_MS);
  }
  return;
}
#else
void vPortSuppressTicksAndSleep( uint32_t xExpectedIdleTime )
{
  return;
}
#endif /* ( CFG_LPM_LEVEL != 0) */

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

#if (CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE != 0)
static void LogTimestamp(uint8_t * pData, uint16_t * piSize)
{
	uint16_t len = 0;

	len =snprintf ((char*)pData,356,"[%010lu] ",HAL_GetTick());

	if (len < 0)
	{
	    *piSize = 0;
	    return;
	}
	*piSize = len;
}
#endif /*CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE */
/* USER CODE END FD_WRAP_FUNCTIONS */
