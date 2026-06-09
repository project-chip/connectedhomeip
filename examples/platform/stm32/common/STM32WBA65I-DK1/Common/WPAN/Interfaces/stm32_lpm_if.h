/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32_lpm_if.h
  * @author  MCD Application Team
  * @brief   Header for stm32_lpm_if.c module (device specific LP management)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef STM32_TINY_LPM_IF_H
#define STM32_TINY_LPM_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx_ll_cortex.h"
#include "stm32wbaxx_hal.h"
#include "stm32wbaxx_ll_pwr.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  /* USER CODE BEGIN MT1 */
  /* USER CODE END MT1 */
  UTIL_LPM_IDLE_MODE,            /**< Idle mode */
  /* USER CODE BEGIN MT2 */
  /* USER CODE END MT2 */
  UTIL_LPM_SLEEP_MODE,           /**< Sleep mode */
  /* USER CODE BEGIN MT3 */
  /* USER CODE END MT3 */
#if (CFG_LPM_STOP1_SUPPORTED == 1)
  UTIL_LPM_STOP1_MODE,           /**< Stop 1 mode */
  /* USER CODE BEGIN MT4 */
  /* USER CODE END MT4 */
#endif
#if (CFG_LPM_STOP2_SUPPORTED == 1)
  UTIL_LPM_STOP2_MODE,           /**< Stop 2 mode */
  /* USER CODE BEGIN MT5 */
  /* USER CODE END MT5 */
#endif
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
  UTIL_LPM_STANDBY_MODE,         /**< Standby mode */
  /* USER CODE BEGIN MT6 */
  /* USER CODE END MT6 */
#endif
  UTIL_LPM_NUM_MODES             /**< Number of supported modes */
} UTIL_LPM_Mode_t;

#define UTIL_LPM_MAX_MODE (UTIL_LPM_NUM_MODES - 1) /**< Highest (most efficient )supported modes */

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief Handle enter and exit of Idle mode
  */
void LPM_IdleMode(uint32_t param);

/**
  * @brief Handle enter and exit of Sleep mode
  */
void LPM_SleepMode(uint32_t param);

#if (CFG_LPM_STOP1_SUPPORTED == 1)
/**
  * @brief Handle enter and exit of Stop1 mode
  */
void LPM_Stop1Mode(uint32_t param);

#endif
#if (CFG_LPM_STOP2_SUPPORTED == 1)
/**
  * @brief Handle enter and exit of Stop2 mode
  */
void LPM_Stop2Mode(uint32_t param);

#endif
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
/**
  * @brief Handle enter and exit of Standby mode
  */
void LPM_StandbyMode(uint32_t param);

#endif

/**
  * @brief Check if the system is waking-up from standby low power mode.
  */
uint32_t is_boot_from_standby(void);

#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
/**
  * @brief returns 0 if wakeup time profiling is not done.
  */
uint32_t LPM_is_wakeup_time_profiling_done(void);
#endif /* CFG_LPM_STANDBY_SUPPORTED */
#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */

/**
  * @brief will save MCU context if before standby entry
  */
void CPUcontextSave(void);

/**
  * @brief Backup CPU peripheral registers selected in @ref register_backup_table
  * @note Implemented in stm32wbaxx_ResetHandler.s
  */
void backup_system_register(void);

/**
  * @brief Restore CPU peripheral registers selected in @ref register_backup_table
  * @note Implemented in stm32wbaxx_ResetHandler.s
  */
void restore_system_register(void);

/**
  * @brief Restore GPIOs configuration at standby exit
  */
void Standby_Restore_GPIO(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* STM32_TINY_LPM_IF_H */

