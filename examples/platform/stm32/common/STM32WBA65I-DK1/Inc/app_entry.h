/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.h
  * @author  MCD Application Team
  * @brief   Interface to the application
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
#ifndef APP_ENTRY_H
#define APP_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include "app_common.h"
#include "cmsis_os2.h"
#include "stm32_timer.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported constants --------------------------------------------------------*/
#define WPAN_SUCCESS 0u

/* Exported types ------------------------------------------------------------*/
/* MATTER BEGIN */
/* USER CODE BEGIN ET */

/* To keep the code reusable btw project we will map joystick on Button */
#if (CFG_JOYSTICK_SUPPORTED == 1)
typedef enum
{
  B1 = 0,
  B2 = 1,
  B3 = 2
} Button_TypeDef;


typedef struct
{
  Button_TypeDef      button;
  UTIL_TIMER_Object_t longTimerId;
  uint8_t             longPressed;
  uint8_t State; //1 pushed
} ButtonDesc_t;
#endif /* CFG_JOYSTICK_SUPPORTED == 1 */

typedef void (*PushButtonCallback)(ButtonDesc_t *aMessage);
/* USER CODE END ET */


#define SOFTWARE_VERSION_KEY "softwareid"
/* MATTER END */

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported variables --------------------------------------------------------*/
extern osThreadId_t WpanTaskHandle;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void MX_APPE_Config(void);
uint32_t MX_APPE_Init(void *p_param);
void MX_APPE_LinkLayerInit(void);

/* MATTER BEGIN */
void APPE_PushButtonSetReceiveCb(PushButtonCallback aCallback);
uint8_t APPE_GetBootReason();
/* MATTER END */
/* USER CODE BEGIN EFP */


/* USER CODE END EFP */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_ENTRY_H */
