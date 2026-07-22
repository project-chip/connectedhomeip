/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/* USER CODE END Header */

#include "ll_sys_if.h"
#include "app_common.h"
#include "app_conf.h"
#include "ll_intf_cmn.h"
#include "ll_sys.h"
#include "log_module.h"
#include "main.h"
#include "platform.h"
#include "stm32_rtos.h"
#include "utilities_common.h"
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
#include "temp_measurement.h"
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */
#if (CFG_LPM_STANDBY_SUPPORTED == 0)
extern void profile_reset(void);
#endif
/* Private defines -----------------------------------------------------------*/
/* Radio event scheduling method - must be set at 1 */
#define USE_RADIO_LOW_ISR (1)
#define NEXT_EVENT_SCHEDULING_FROM_ISR (1)

#define LSI_RCO_CALIB_PERIOD_MS (15000U)   /* LSI calib period in ms */
#define LSI_RCO_CALIB_DURATION_CYCLE (24U) /* LSI calib duration in LL sleep timer clock cycles */

/* USER CODE BEGIN PD */
void ll_intf_apply_cte_degrad_change(void);
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private constants ---------------------------------------------------------*/
/* USER CODE BEGIN PC */

/* USER CODE END PC */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/

osMutexId_t LinkLayerMutex;

const osMutexAttr_t LinkLayerMutex_attributes = {
    .name = "Link Layer Mutex", .attr_bits = TASK_DEFAULT_ATTR_BITS, .cb_mem = TASK_DEFAULT_CB_MEM, .cb_size = TASK_DEFAULT_CB_SIZE
};

/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
static void ll_sys_bg_temperature_measurement_init(void);
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
static void ll_sys_sleep_clock_source_selection(void);
static uint8_t ll_sys_BLE_sleep_clock_accuracy_selection(void);
void ll_sys_reset(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Link Layer background process initialization
 * @param  None
 * @retval None
 */
void ll_sys_bg_process_init(void)
{
    /* Create Link Layer FreeRTOS objects */

    LinkLayerMutex = osMutexNew(&LinkLayerMutex_attributes);

    if (LinkLayerMutex == NULL)
    {
        LOG_ERROR_APP("Link Layer FreeRTOS objects creation FAILED");
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
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_LinkLayer);
}

/**
 * @brief  Link Layer background process next iteration scheduling from ISR
 * @param  None
 * @retval None
 */
void ll_sys_schedule_bg_process_isr(void)
{
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_LinkLayer);
}

/**
 * @brief  Link Layer configuration phase before application startup.
 * @param  None
 * @retval None
 */
void ll_sys_config_params(void)
{
    /* USER CODE BEGIN ll_sys_config_params_0 */

    /* USER CODE END ll_sys_config_params_0 */

    /* Configure link layer behavior for low ISR use and next event scheduling method:
     * - SW low ISR is used.
     * - Next event is scheduled from ISR.
     */
    ll_intf_cmn_config_ll_ctx_params(USE_RADIO_LOW_ISR, NEXT_EVENT_SCHEDULING_FROM_ISR);
    /* Apply the selected link layer sleep timer source */
    ll_sys_sleep_clock_source_selection();

    /* USER CODE BEGIN ll_sys_config_params_1 */

    /* USER CODE END ll_sys_config_params_1 */

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
    /* Initialize link layer temperature measurement background task */
    ll_sys_bg_temperature_measurement_init();

    /* Link layer IP uses temperature based calibration instead of periodic one */
    ll_intf_cmn_set_temperature_sensor_state();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

    /* Link Layer power table */
    ll_intf_cmn_select_tx_power_table(CFG_RF_TX_POWER_TABLE_ID);

#if (USE_CTE_DEGRADATION == 1u)
    /* Apply CTE degradation */
    ll_sys_apply_cte_settings();
#endif /* (USE_CTE_DEGRADATION == 1u) */

    /* USER CODE BEGIN ll_sys_config_params_2 */

    /* USER CODE END ll_sys_config_params_2 */
}

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)

/**
 * @brief  Link Layer temperature request background process initialization
 * @param  None
 * @retval None
 */
void ll_sys_bg_temperature_measurement_init(void) {}

/**
 * @brief  Request backroud task processing for temperature measurement
 * @param  None
 * @retval None
 */
void ll_sys_bg_temperature_measurement(void)
{
    static uint8_t initial_temperature_acquisition = 0;

    if (initial_temperature_acquisition == 0)
    {
        TEMPMEAS_RequestTemperatureMeasurement();
        initial_temperature_acquisition = 1;
    }
    else
    {
        osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_TempRadioCalib);
    }
}

#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

uint8_t ll_sys_BLE_sleep_clock_accuracy_selection(void)
{
    uint8_t BLE_sleep_clock_accuracy = 0;
#if (CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE == 0)
    uint32_t RevID = LL_DBGMCU_GetRevisionID();
#endif
    uint32_t linklayer_slp_clk_src = LL_RCC_RADIO_GetSleepTimerClockSource();

    if (linklayer_slp_clk_src == LL_RCC_RADIOSLEEPSOURCE_LSE)
    {
        /* LSE selected as Link Layer sleep clock source.
           Sleep clock accuracy is different regarding the WBA device ID and revision
         */
#if (CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE == 0)
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx)
        if (RevID == REV_ID_A)
        {
            BLE_sleep_clock_accuracy = STM32WBA5x_REV_ID_A_SCA_RANGE;
        }
        else if (RevID == REV_ID_B)
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
#else  /* CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE */
        BLE_sleep_clock_accuracy = CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE;
#endif /* CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE */
    }
    else
    {
        /* LSE is not the Link Layer sleep clock source, sleep clock accurcay default value is 500 ppm */
        BLE_sleep_clock_accuracy = STM32WBA5x_DEFAULT_SCA_RANGE;
    }

    return BLE_sleep_clock_accuracy;
}

void ll_sys_sleep_clock_source_selection(void)
{
    uint16_t freq_value            = 0;
    uint32_t linklayer_slp_clk_src = LL_RCC_RADIOSLEEPSOURCE_NONE;

    linklayer_slp_clk_src = LL_RCC_RADIO_GetSleepTimerClockSource();
    switch (linklayer_slp_clk_src)
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
    ll_intf_cmn_le_select_slp_clk_src((uint8_t) linklayer_slp_clk_src, &freq_value);
}

void ll_sys_reset(void)
{
    uint8_t bsca = 0;
    /* Link layer timings */
    uint8_t drift_time = DRIFT_TIME_DEFAULT;
    uint8_t exec_time  = EXEC_TIME_DEFAULT;

    /* USER CODE BEGIN ll_sys_reset_0 */
    drift_time = DRIFT_TIME_OPTIMIZED;
    exec_time  = EXEC_TIME_OPTIMIZED;
    /* USER CODE END ll_sys_reset_0 */

    /* Apply the selected link layer sleep timer source */
    ll_sys_sleep_clock_source_selection();

    /* Configure the link layer sleep clock accuracy */
    bsca = ll_sys_BLE_sleep_clock_accuracy_selection();
    ll_intf_le_set_sleep_clock_accuracy(bsca);

    if (LL_RCC_RADIO_GetSleepTimerClockSource() == LL_RCC_RADIOSLEEPSOURCE_LSI)
    {
        /* Configure RCO calibration */
        ll_intf_le_set_rco_clbr_evnt_params(LSI_RCO_CALIB_DURATION_CYCLE, LSI_RCO_CALIB_PERIOD_MS);
    }

    /* Update link layer timings depending on selected configuration */
    if (LL_RCC_RADIO_GetSleepTimerClockSource() == LL_RCC_RADIOSLEEPSOURCE_LSI)
    {
        drift_time += DRIFT_TIME_EXTRA_LSI2;
        exec_time += EXEC_TIME_EXTRA_LSI2;
    }
    else
    {
#if defined(__GNUC__) && defined(DEBUG)
        drift_time += DRIFT_TIME_EXTRA_GCC_DEBUG;
        exec_time += EXEC_TIME_EXTRA_GCC_DEBUG;
#endif
    }

    /* USER CODE BEGIN ll_sys_reset_1 */

    /* USER CODE END ll_sys_reset_1 */

    if ((drift_time != DRIFT_TIME_DEFAULT) || (exec_time != EXEC_TIME_DEFAULT))
    {
        ll_sys_config_BLE_schldr_timings(drift_time, exec_time);
    }
    /* USER CODE BEGIN ll_sys_reset_2 */

    /* USER CODE END ll_sys_reset_2 */
}
#if defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx)
void ll_sys_apply_cte_settings(void)
{
    ll_intf_apply_cte_degrad_change();
}
#endif /* defined(STM32WBA52xx) || defined(STM32WBA54xx) || defined(STM32WBA55xx) || defined(STM32WBA65xx) */

#if (CFG_LPM_STANDBY_SUPPORTED == 0)
void ll_sys_get_ble_profile_statistics(uint32_t * exec_time, uint32_t * drift_time, uint32_t * average_drift_time, uint8_t reset)
{
    ll_intf_get_profile_statistics(exec_time, drift_time, average_drift_time, reset);
}
#endif

void ll_sys_set_rtl_polling_time(uint8_t rtl_polling_time)
{
    /* first parameter otInstance *aInstance is unused */
    radio_set_rtl_polling_time(NULL, rtl_polling_time);
}
