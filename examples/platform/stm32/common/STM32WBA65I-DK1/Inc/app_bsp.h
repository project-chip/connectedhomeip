/**
  ******************************************************************************
  * @file    app_bsp.h
  * @author  MCD Application Team
  * @brief   Interface to manage BSP.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_BSP_H
#define APP_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_conf.h"
#if (CFG_LCD_SUPPORTED == 1)
#define LCD1 (0)
#endif /* (CFG_LCD_SUPPORTED == 1) */

#ifdef STM32WBA55xx
#ifdef CFG_BSP_ON_DISCOVERY
#include "stm32wba55g_discovery.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32wba55g_discovery_lcd.h"
#include "stm32_lcd.h"
#endif /* (CFG_LCD_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */
#endif /* STM32WBA55xx */

#ifdef STM32WBA65xx  
#ifdef CFG_BSP_ON_DISCOVERY
#include "stm32wba65i_discovery.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32wba65i_discovery_lcd.h"
#include "stm32_lcd.h"
#endif /* (CFG_LCD_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */
#endif /* STM32WBA65xx */

#ifdef CFG_BSP_ON_CEB
#include "b_wba5m_wpan.h"
#endif /* CFG_BSP_ON_CEB */
#ifdef CFG_BSP_ON_NUCLEO
#include "stm32wbaxx_nucleo.h"
#endif /* CFG_BSP_ON_CEB */

/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported macros ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void      APP_BSP_Init                    ( void );
void      APP_BSP_PostIdle                ( void );
void      APP_BSP_StandbyExit             ( void );
uint8_t   APP_BSP_SerialCmdExecute        ( uint8_t * pRxBuffer, uint16_t iRxBufferSize );

#if (CFG_LED_SUPPORTED == 1)
void      APP_BSP_LedInit                 ( void );

#endif /* (CFG_LED_SUPPORTED == 1) */
#if (CFG_LCD_SUPPORTED == 1)
void      APP_BSP_LcdInit             ( void );

#endif /* (CFG_LCD_SUPPORTED == 1) */
#if ( CFG_BUTTON_SUPPORTED == 1 )
void      APP_BSP_ButtonInit              ( void );

uint8_t   APP_BSP_ButtonIsLongPressed     ( uint16_t btnIdx );
void      APP_BSP_SetButtonIsLongPressed  ( uint16_t btnIdx );

void      APP_BSP_Button1Action           ( void );
void      APP_BSP_Button2Action           ( void );
void      APP_BSP_Button3Action           ( void );

void      BSP_PB_Callback                 ( Button_TypeDef button );

#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */
#if ( CFG_JOYSTICK_SUPPORTED == 1 )
void      APP_BSP_JoystickInit            ( void );

void      APP_BSP_JoystickUpAction        ( void );
void      APP_BSP_JoystickRightAction     ( void );
void      APP_BSP_JoystickDownAction      ( void );
void      APP_BSP_JoystickLeftAction      ( void );
void      APP_BSP_JoystickSelectAction    ( void );

void      BSP_JOY_Callback                ( JOY_TypeDef joyNb, JOYPin_TypeDef joyPin );

#endif /* CFG_JOYSTICK_SUPPORTED */



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_BSP_H */
