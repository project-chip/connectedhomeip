/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ll_sys_if.c
  * @author  MCD Application Team
  * @brief   Source file for initiating system
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

#include "app_common.h"
#include "main.h"
#include "app_conf.h"
#include "ll_intf.h"
#include "ll_sys.h"
#include "ll_sys_if.h"
#include "ll_intf_cmn.h"
#include "cmsis_os2.h"
#include "stm32_rtos.h"
#include "utilities_common.h"
#include "linklayer_plat.h"
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
#include "temp_measurement.h"
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

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
/* LINK_LAYER_TEMP_MEAS_TASK related resources */
osSemaphoreId_t         LinkLayerMeasSemaphore;
osThreadId_t            LinkLayerMeasThread;

/* LINK_LAYER_TASK related resources */
osSemaphoreId_t         LinkLayerSemaphore;
osThreadId_t            LinkLayerThread;
osMutexId_t             LinkLayerMutex;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
static void ll_sys_bg_temperature_measurement_init(void);
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
static void ll_sys_sleep_clock_source_selection(void);
#if (CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE == 0)
static uint8_t ll_sys_BLE_sleep_clock_accuracy_selection(void);
#endif /* CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE */
void ll_sys_reset(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Link Layer Task for FreeRTOS
 * @param  void *argument
 * @retval None
 */
static void LinkLayerSys_Task_Entry(void *argument)
{
  UNUSED( argument );
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif

  for(;;)
  {
    osSemaphoreAcquire(LinkLayerSemaphore, osWaitForever);
    osMutexAcquire(LinkLayerMutex, osWaitForever);
    ll_sys_bg_process();
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

/**
  * @brief  Link Layer background process initialization
  * @param  None
  * @retval None
  */
void ll_sys_bg_process_init(void)
{
  /* Tasks creation */
  const osSemaphoreAttr_t LinkLayerSemaphore_attributes = {
    .name = "LinkLayerSemaphore"
  };
  LinkLayerSemaphore = osSemaphoreNew(1U, 0U, &LinkLayerSemaphore_attributes);
  if (LinkLayerSemaphore == NULL)
  {
    Error_Handler();
  }
  const osMutexAttr_t LinkLayerMutex_attributes = {
    .name = "LinkLayer Mutex"
  };
  LinkLayerMutex = osMutexNew(&LinkLayerMutex_attributes);
  if (LinkLayerMutex == NULL)
  {
    Error_Handler();
  }
  const osThreadAttr_t LinkLayerTask_attributes = {
    .name = "Link Layer Task",
    .priority = CFG_TASK_PRIO_LINK_LAYER,
    .stack_size = TASK_LINK_LAYER_STACK_SIZE
  };
  LinkLayerThread = osThreadNew(LinkLayerSys_Task_Entry, NULL, &LinkLayerTask_attributes);
  if (LinkLayerThread == NULL)
  {
    Error_Handler();
  }
}

/**
  * @brief  Link Layer background process next iteration scheduling
  * @param  None
  * @retval None
  */
void ll_sys_schedule_bg_process(void)
{
   osSemaphoreRelease(LinkLayerSemaphore);
}

/**
  * @brief  Link Layer background process next iteration scheduling from ISR
  * @param  None
  * @retval None
  */
void ll_sys_schedule_bg_process_isr(void)
{

  LINKLAYER_PLAT_DisableIRQ(); // New entrer in critical section
  osSemaphoreRelease(LinkLayerSemaphore);

  LINKLAYER_PLAT_EnableIRQ();
}

/**
  * @brief  Link Layer configuration phase before application startup.
  * @param  None
  * @retval None
  */
void ll_sys_config_params(void)
{
  /* Configure link layer behavior for low ISR use and next event scheduling method:
   * - SW low ISR is used.
   * - Next event is scheduled from ISR.
   */
  ll_intf_cmn_config_ll_ctx_params(USE_RADIO_LOW_ISR, NEXT_EVENT_SCHEDULING_FROM_ISR);
  
  /* Apply the selected link layer sleep timer source */
  ll_sys_sleep_clock_source_selection();
  
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  /* Initialize link layer temperature measurement background task */
  ll_sys_bg_temperature_measurement_init();

  /* Link layer IP uses temperature based calibration instead of periodic one */
  ll_intf_cmn_set_temperature_sensor_state();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

  /* Link Layer power table */
  ll_intf_cmn_select_tx_power_table(CFG_RF_TX_POWER_TABLE_ID);
}

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
/**
 * @brief  Link Layer Temperature Measurement Task for FreeRTOS
 * @param
 * @retval None
 */
 static void LinkLayerTempMeasure_Task_Entry( void *argument )
{
  UNUSED( argument );
#if HIGHWATERMARK
    UBaseType_t uxHighWaterMark;
    UBaseType_t lastHighWaterMark = 0;
#endif // endif HIGHWATERMARK

  for(;;)
  {
    osSemaphoreAcquire(LinkLayerMeasSemaphore, osWaitForever);
    osMutexAcquire(LinkLayerMutex, osWaitForever);
    request_temperature_measurement();
    osMutexRelease(LinkLayerMutex);
    osThreadYield();
#if HIGHWATERMARK
        uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        if (uxHighWaterMark != lastHighWaterMark) {
            APP_DBG("\x1b[34m" "LinkLayerTempMeasureTaskEntry_stack_HighWaterMark %lu \n" "\x1b[0m", uxHighWaterMark);
            lastHighWaterMark = uxHighWaterMark;
        }
#endif
  }
}

/**
  * @brief  Link Layer temperature request background process initialization
  * @param  None
  * @retval None
  */
void ll_sys_bg_temperature_measurement_init(void)
{
  /* Tasks creation */
  const osSemaphoreAttr_t LinkLayerMeasSemaphore_attributes = {
    .name = "LinkLayerMeasSemaphore"
  };
  LinkLayerMeasSemaphore = osSemaphoreNew(1U, 0U, &LinkLayerMeasSemaphore_attributes);
  if (LinkLayerMeasSemaphore == NULL)
  {
    Error_Handler();
  }

  const osThreadAttr_t LinkLayerMeasTask_attributes = {
    .name = "LinkLayer Temp Measurement Task",
    .priority = (osPriority_t) LINK_LAYER_TEMP_MEAS_TASK_PRIO,
    .stack_size = TASK_LINK_LAYER_TEMP_MEAS_STACK_SIZE
  };
  LinkLayerMeasThread = osThreadNew(LinkLayerTempMeasure_Task_Entry, NULL, &LinkLayerMeasTask_attributes);
  if (LinkLayerMeasThread == NULL)
  {
    Error_Handler();
  }
}

/**
  * @brief  Request backroud task processing for temperature measurement
  * @param  None
  * @retval None
  */
void ll_sys_bg_temperature_measurement(void)
{
  static uint8_t initial_temperature_acquisition = 0;

  if(initial_temperature_acquisition == 0)
  {
    TEMPMEAS_RequestTemperatureMeasurement();
    initial_temperature_acquisition = 1;
  }
  else
  {
  	osSemaphoreRelease(LinkLayerMeasSemaphore);
  }
}

#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

#if (CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE == 0)
uint8_t ll_sys_BLE_sleep_clock_accuracy_selection(void)
{
  uint8_t BLE_sleep_clock_accuracy = 0;
  uint32_t RevID = LL_DBGMCU_GetRevisionID();
  uint32_t linklayer_slp_clk_src = LL_RCC_RADIO_GetSleepTimerClockSource();
  
  if(linklayer_slp_clk_src == LL_RCC_RADIOSLEEPSOURCE_LSE)
  {
    /* LSE selected as Link Layer sleep clock source. 
       Sleep clock accuracy is different regarding the WBA device ID and revision 
     */
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
    if(RevID == REV_ID_A)
    {
      BLE_sleep_clock_accuracy = STM32WBA5x_REV_ID_A_SCA_RANGE;
    } 
    else if(RevID == REV_ID_B)
    {
      BLE_sleep_clock_accuracy = STM32WBA5x_REV_ID_B_SCA_RANGE;
    } 
    else
    {
      /* Revision ID not supported, default value of 500ppm applied */
      BLE_sleep_clock_accuracy = STM32WBA5x_DEFAULT_SCA_RANGE;
    }
#elif defined(STM32WBA65xx)
    BLE_sleep_clock_accuracy = STM32WBA6x_SCA_RANGE;
    UNUSED(RevID);
#else
    UNUSED(RevID);
#endif /* defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) */
  }
  else
  {
    /* LSE is not the Link Layer sleep clock source, sleep clock accurcay default value is 500 ppm */
    BLE_sleep_clock_accuracy = STM32WBA5x_DEFAULT_SCA_RANGE;
  }
  
  return BLE_sleep_clock_accuracy;
}
#endif /* CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE */

void ll_sys_sleep_clock_source_selection(void)
{
  uint16_t freq_value = 0;
  uint32_t linklayer_slp_clk_src = LL_RCC_RADIOSLEEPSOURCE_NONE;
  
  linklayer_slp_clk_src = LL_RCC_RADIO_GetSleepTimerClockSource();
  switch(linklayer_slp_clk_src)
  {
    case LL_RCC_RADIOSLEEPSOURCE_LSE:
      linklayer_slp_clk_src = RTC_SLPTMR;
      break;

    case LL_RCC_RADIOSLEEPSOURCE_LSI:
      linklayer_slp_clk_src = RCO_SLPTMR;
      break;

    case LL_RCC_RADIOSLEEPSOURCE_HSE_DIV1000:
      linklayer_slp_clk_src = CRYSTAL_OSCILLATOR_SLPTMR;
      break;

    case LL_RCC_RADIOSLEEPSOURCE_NONE:
      /* No Link Layer sleep clock source selected */
      assert_param(0);
      break;
  }
  ll_intf_le_select_slp_clk_src((uint8_t)linklayer_slp_clk_src, &freq_value);
}

void ll_sys_reset(void)
{
#if (CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE == 0)
  uint8_t bsca = 0;
#endif /* CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE */  
  
  /* Apply the selected link layer sleep timer source */
  ll_sys_sleep_clock_source_selection();
  
  /* Configure the link layer sleep clock accuracy if different from the default one */
#if (CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE != 0)
  ll_intf_le_set_sleep_clock_accuracy(CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE);
#else
  bsca = ll_sys_BLE_sleep_clock_accuracy_selection();
  if(bsca != STM32WBA5x_DEFAULT_SCA_RANGE)
  {
    ll_intf_le_set_sleep_clock_accuracy(bsca);
  }
#endif /* CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE */
}
