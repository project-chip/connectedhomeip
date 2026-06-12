/**
 ******************************************************************************
 * @file    app_bsp.h
 * @author  MCD Application Team
 * @brief   Interface to manage BSP.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_BSP_H
#define APP_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_conf.h"
#include "stm32_rtos.h"

#ifdef CFG_BSP_ON_DISCOVERY
#ifdef STM32WBA55xx
#include "stm32wba55g_discovery.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32_lcd.h"
#include "stm32wba55g_discovery_lcd.h"
#endif /* (CFG_LCD_SUPPORTED == 1) */
#endif /* STM32WBA55xx */
#ifdef STM32WBA65xx
#include "stm32wba65i_discovery.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32_lcd.h"
#include "stm32wba65i_discovery_lcd.h"
#endif /* (CFG_LCD_SUPPORTED == 1) */
#endif /* STM32WBA65xx */
#endif /* CFG_BSP_ON_DISCOVERY */

#ifdef CFG_BSP_ON_CEB
#ifdef STM32WBA5Mxx
#include "b_wba5m_wpan.h"
#endif
#ifdef STM32WBA6Mxx
#include "b_wba6m_wpan.h"
#endif
#endif /* CFG_BSP_ON_CEB */

#ifdef CFG_BSP_ON_NUCLEO
#include "stm32wbaxx_nucleo.h"
#endif /* CFG_BSP_ON_NUCLEO */

#ifdef CFG_COAP_MSG
#include "coap.h"
#endif /* CFG_COAP_MSG */

/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#if (CFG_LCD_SUPPORTED == 1)
#define LCD1 (0u)
#endif /* (CFG_LCD_SUPPORTED == 1) */

#if (defined CFG_BSP_ON_DISCOVERY) && (defined STM32WBA65xx)
/* No Led Blue on Discovery for STM32WBA65I. Replaced by Red Led */
#define LED_BLUE LED_RED
#endif /* (defined CFG_BSP_ON_DISCOVERY) && (defined STM32WBA65xx) */

#if (CFG_JOYSTICK_SUPPORTED == 1)
#define JOYSTICK_USE_AS_JOYSTICK                                                                                                   \
    (0u) /* When Joystick is not 'none', call according 'Joystick Action' every JOYSTICK_PRESS_SAMPLE_MS. */
#define JOYSTICK_USE_AS_BUTTON (1u) /* When Joystick is pressed, call according 'Joystick Action' one time. */
#define JOYSTICK_USE_AS_BUTTON_WITH_TIME                                                                                           \
    (2u) /* When Joystick is pressed, it wait the release or the end of  JOYSTICK_LONG_PRESS_THRESHOLD_MS                          \
            before call according 'Joystick Action'. */
#define JOYSTICK_USE_AS_CHANGE                                                                                                     \
    (3u) /* When Joystcik is pressed according 'Joystick Action' is called. When the Joystick is released, 'JoystickNoneAction' is \
            called. */
#define JOYSTICK_USE_AS_MATTER                                                                                                     \
    (4u) /* When Joystick is pressed, according 'Joystick Action' is called. When the Joystick is released or the end              \
            of JOYSTICK_LONG_PRESS_THRESHOLD_MS occurs, according 'Joystick Action' is called (same as when pressed). */
#endif   /* (CFG_JOYSTICK_SUPPORTED == 1) */

/* Exported variables --------------------------------------------------------*/
#ifdef CFG_COAP_MSG
extern uint32_t APP_Thread_TransmitPeriod_ms;
extern uint32_t APP_Thread_CoapPayloadLength_byte;
extern otCoapType APP_Thread_CoapType;
#endif /* CFG_COAP_MSG */
#ifdef CFG_BLE_ADV_CHANNEL_MAP
extern uint8_t APP_BLE_AdvChannelMap;
#endif /* CFG_BLE_ADV_CHANNEL_MAP */

#if (CFG_BUTTON_SUPPORTED == 1)
#ifdef CFG_BSP_ON_THREADX
extern TX_SEMAPHORE ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
extern osSemaphoreId_t ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
#endif /* CFG_BSP_ON_FREERTOS */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

#if (CFG_JOYSTICK_SUPPORTED == 1)
#ifdef CFG_BSP_ON_THREADX
extern TX_SEMAPHORE JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore,
    JoystickSelectSemaphore, JoystickNoneSemaphore;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
extern osSemaphoreId_t JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore,
    JoystickSelectSemaphore, JoystickNoneSemaphore;
#endif /* CFG_BSP_ON_FREERTOS */
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */

/* Exported macros ------------------------------------------------------------*/
#if (CFG_LED_SUPPORTED == 1)
#define APP_BSP_LED_On(LED) BSP_LED_On(LED)
#define APP_BSP_LED_Off(LED) BSP_LED_Off(LED)
#define APP_BSP_LED_Toggle(LED) BSP_LED_Toggle(LED)
#else /* ( CFG_LED_SUPPORTED == 1 ) */
#define APP_BSP_LED_On(LED)
#define APP_BSP_LED_Off(LED)
#define APP_BSP_LED_Toggle(LED)
#endif /* ( CFG_LED_SUPPORTED == 1 ) */

/* Exported functions prototypes ---------------------------------------------*/
void APP_BSP_Init(void);
void APP_BSP_PostIdle(void);
void APP_BSP_StandbyExit(void);
uint8_t APP_BSP_SerialCmdExecute(uint8_t * pRxBuffer, uint16_t iRxBufferSize);

#if (CFG_LED_SUPPORTED == 1)
void APP_BSP_LedInit(void);

#endif /* (CFG_LED_SUPPORTED == 1) */
#if (CFG_LCD_SUPPORTED == 1)
void APP_BSP_LcdInit(void);

#endif /* (CFG_LCD_SUPPORTED == 1) */
#if (CFG_BUTTON_SUPPORTED == 1)
void APP_BSP_ButtonInit(void);

uint8_t APP_BSP_ButtonIsLongPressed(uint16_t btnIdx);
void APP_BSP_SetButtonIsLongPressed(uint16_t btnIdx);

void APP_BSP_Button1Action(void);
void APP_BSP_Button2Action(void);
void APP_BSP_Button3Action(void);

void BSP_PB_Callback(Button_TypeDef button);

#ifdef CFG_BSP_ON_NUCLEO
#ifdef STM32WBA65xx
void EXTI4_IRQHandler(void);
void EXTI5_IRQHandler(void);
#endif /* STM32WBA65xx */
#endif /* CFG_BSP_ON_NUCLEO */

#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */
#if (CFG_JOYSTICK_SUPPORTED == 1)
void APP_BSP_JoystickInit(void);
uint8_t APP_BSP_JoystickIsLongPressed(void);
uint8_t APP_BSP_JoystickIsShortReleased(void);
uint8_t APP_BSP_JoystickIsInitialPress(void);

void APP_BSP_JoystickUpAction(void);
void APP_BSP_JoystickRightAction(void);
void APP_BSP_JoystickDownAction(void);
void APP_BSP_JoystickLeftAction(void);
void APP_BSP_JoystickSelectAction(void);
void APP_BSP_JoystickNoneAction(void);

void BSP_JOY_Callback(JOY_TypeDef joyNb, JOYPin_TypeDef joyPin);

#endif /* CFG_JOYSTICK_SUPPORTED */

#if defined(CFG_COAP_MSG) || defined(CFG_BLE_ADV_CHANNEL_MAP)
void APP_BSP_CliInit(void);
#endif /* CFG_COAP_MSG || CFG_BLE_ADV_CHANNEL_MAP */
#ifdef CFG_COAP_MSG
void APP_BSP_CoapMsgRateAction(void);
#endif /* CFG_COAP_MSG */
#ifdef CFG_BLE_ADV_CHANNEL_MAP
void APP_BSP_BleAdvChannelMapAction(void);
#endif /* CFG_BLE_ADV_CHANNEL_MAP */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_BSP_H */
