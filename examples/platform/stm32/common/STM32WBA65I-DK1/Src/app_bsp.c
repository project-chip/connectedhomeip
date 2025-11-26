/**
  ******************************************************************************
  * @file    app_bsp.c
  * @author  MCD Application Team
  * @brief   Application to manage BSP.
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

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "log_module.h"
#include "app_conf.h"
#include "app_bsp.h"
#include "main.h"

#include "stm32_rtos.h"
#include "stm32_timer.h"

/* Private includes -----------------------------------------------------------*/
#include "serial_cmd_interpreter.h"

/* Private typedef -----------------------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
typedef struct
{
  Button_TypeDef      button;
  UTIL_TIMER_Object_t longTimerId;
  uint8_t             longPressed;
  uint32_t            waitingTime;
} ButtonDesc_t;
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

/* Private defines -----------------------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
#define BUTTON_LONG_PRESS_SAMPLE_MS           (50u)     /* Sample button level rate in milli seconds. */
#define BUTTON_LONG_PRESS_THRESHOLD_MS        (500u)    /* Long pression time threshold in milli seconds. */
#ifdef CFG_BSP_ON_CEB
#define BUTTON_NB_MAX                         (B2 + 1u)
#else /* CFG_BSP_ON_CEB */
#define BUTTON_NB_MAX                         (B3 + 1u)
#endif /* CFG_BSP_ON_CEB */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

#ifndef CFG_BSP_ON_SEQUENCER
#if (CFG_BUTTON_SUPPORTED == 1)
/* Push Button B1 Task related defines */
#define TASK_STACK_SIZE_BUTTON_B1             TASK_STACK_SIZE_BUTTON_Bx
#define TASK_PRIO_BUTTON_B1                   TASK_PRIO_BUTTON_Bx
#define TASK_PREEMP_BUTTON_B1                 TASK_PREEMP_BUTTON_Bx

/* Push Button B2 Task related defines */
#define TASK_STACK_SIZE_BUTTON_B2             TASK_STACK_SIZE_BUTTON_Bx
#define TASK_PRIO_BUTTON_B2                   TASK_PRIO_BUTTON_Bx
#define TASK_PREEMP_BUTTON_B2                 TASK_PREEMP_BUTTON_Bx

/* Push Button B3 Task related defines */
#define TASK_STACK_SIZE_BUTTON_B3             TASK_STACK_SIZE_BUTTON_Bx
#define TASK_PRIO_BUTTON_B3                   TASK_PRIO_BUTTON_Bx
#define TASK_PREEMP_BUTTON_B3                 TASK_PREEMP_BUTTON_Bx

#endif /* (CFG_BUTTON_SUPPORTED == 1) */
#if (CFG_JOYSTICK_SUPPORTED == 1)
/* Push Joystick Up Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_UP           TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_UP                 TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_UP               TASK_PREEMP_JOYSTICK_x

/* Push Joystick Right Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_RIGHT        TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_RIGHT              TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_RIGHT            TASK_PREEMP_JOYSTICK_x

/* Push Joystick Down Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_DOWN         TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_DOWN               TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_DOWN             TASK_PREEMP_JOYSTICK_x

/* Push Joystick Left Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_LEFT         TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_LEFT               TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_LEFT             TASK_PREEMP_JOYSTICK_x

/* Push Joystick Select Task related defines */
#define TASK_STACK_SIZE_JOYSTICK_SELECT       TASK_STACK_SIZE_JOYSTICK_x
#define TASK_PRIO_JOYSTICK_SELECT             TASK_PRIO_JOYSTICK_x
#define TASK_PREEMP_JOYSTICK_SELECT           TASK_PREEMP_JOYSTICK_x

#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_SEQUENCER */

#if (CFG_JOYSTICK_SUPPORTED == 1)
#define JOYSTICK_PRESS_SAMPLE_MS              (100u)     /* Sample Joystick level rate in milli seconds. */
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */

/* Private macros ------------------------------------------------------------*/

/* Private constants ---------------------------------------------------------*/
#ifdef CFG_BSP_ON_FREERTOS
#if (CFG_JOYSTICK_SUPPORTED == 1)
/* FreeRtos Joystick Up stack attributes */
const osThreadAttr_t JoystickUpThreadAttributes =
{
  .name         = "Joystick Up Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_UP,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_UP
};

/* FreeRtos Joystick Right stack attributes */
const osThreadAttr_t JoystickRightThreadAttributes =
{
  .name         = "Joystick Up Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_RIGHT,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_RIGHT
};

/* FreeRtos Joystick Down stack attributes */
const osThreadAttr_t JoystickDownThreadAttributes =
{
  .name         = "Joystick Down Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_DOWN,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_DOWN
};

/* FreeRtos Joystick Left stack attributes */
const osThreadAttr_t JoystickLeftThreadAttributes =
{
  .name         = "Joystick Left Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_LEFT,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_LEFT
};

/* FreeRtos Joystick Select stack attributes */
const osThreadAttr_t JoystickSelectThreadAttributes =
{
  .name         = "Joystick Up Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_JOYSTICK_SELECT,
  .stack_size   = TASK_STACK_SIZE_JOYSTICK_SELECT
};

#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#if (CFG_BUTTON_SUPPORTED == 1)

/* FreeRtos PushButton B1 stacks attributes */
const osThreadAttr_t ButtonB1ThreadAttributes =
{
  .name         = "PushButton B1 Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_BUTTON_B1,
  .stack_size   = TASK_STACK_SIZE_BUTTON_B1
};

/* FreeRtos PushButton B2 stacks attributes */
const osThreadAttr_t ButtonB2ThreadAttributes =
{
  .name         = "PushButton B2 Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_BUTTON_B2,
  .stack_size   = TASK_STACK_SIZE_BUTTON_B2
};

/* FreeRtos PushButton B3 stacks attributes */
const osThreadAttr_t ButtonB3ThreadAttributes =
{
  .name         = "PushButton B3 Thread",
  .attr_bits    = TASK_DEFAULT_ATTR_BITS,
  .cb_mem       = TASK_DEFAULT_CB_MEM,
  .cb_size      = TASK_DEFAULT_CB_SIZE,
  .stack_mem    = TASK_DEFAULT_STACK_MEM,
  .priority     = TASK_PRIO_BUTTON_B3,
  .stack_size   = TASK_STACK_SIZE_BUTTON_B3
};
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_FREERTOS */

/* Private variables ---------------------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
/* Button management */
#ifdef CFG_BSP_ON_THREADX
TX_SEMAPHORE         ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
TX_THREAD            ButtonB1Thread, ButtonB2Thread, ButtonB3Thread;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
osSemaphoreId_t      ButtonB1Semaphore, ButtonB2Semaphore, ButtonB3Semaphore;
osThreadId_t         ButtonB1Thread, ButtonB2Thread, ButtonB3Thread;
#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_CEB
static ButtonDesc_t         buttonDesc[BUTTON_NB_MAX] = { { B2, { 0 } , 0, 0 } };
#endif /* CFG_BSP_ON_CEB */
#ifdef CFG_BSP_ON_NUCLEO
static ButtonDesc_t         buttonDesc[BUTTON_NB_MAX] = { { B1, { 0 }, 0, 0 } , { B2, { 0 } , 0, 0 }, { B3, { 0 }, 0, 0 } };
#endif /* CFG_BSP_ON_NUCLEO */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

#if (CFG_JOYSTICK_SUPPORTED == 1)
/* Joystick management */
static UTIL_TIMER_Object_t  joystickTimer;
#ifdef CFG_BSP_ON_THREADX
TX_SEMAPHORE         JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore, JoystickSelectSemaphore;
TX_THREAD            JoystickUpThread, JoystickRightThread, JoystickDownThread, JoystickLeftThread, JoystickSelectThread;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_FREERTOS
osSemaphoreId_t      JoystickUpSemaphore, JoystickRightSemaphore, JoystickDownSemaphore, JoystickLeftSemaphore, JoystickSelectSemaphore;
osThreadId_t         JoystickUpThread, JoystickRightThread, JoystickDownThread, JoystickLeftThread, JoystickSelectThread;
#endif /* CFG_BSP_ON_FREERTOS */
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */

/* Global variables ----------------------------------------------------------*/

/* Private functions prototypes-----------------------------------------------*/
#if (CFG_BUTTON_SUPPORTED == 1)
static void Button_TriggerActions         ( void * arg );
#endif /* (CFG_BUTTON_SUPPORTED == 1) */

/* External variables --------------------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
/**
 * @brief   Initialisation of all used BSP and their Task if needed.
 */
void APP_BSP_Init( void )
{
#if (CFG_LED_SUPPORTED == 1)
  APP_BSP_LedInit();
#endif /* (CFG_LED_SUPPORTED == 1) */

#ifdef CFG_BSP_ON_DISCOVERY
#if (CFG_LCD_SUPPORTED == 1)
  APP_BSP_LcdInit();
#endif /* (CFG_LCD_SUPPORTED == 1) */

#if (CFG_JOYSTICK_SUPPORTED == 1)
  APP_BSP_JoystickInit();
#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */

#if (CFG_BUTTON_SUPPORTED == 1)
  APP_BSP_ButtonInit();
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
}

/**
 * @brief   Verify if Wakeup is not done by a button (or Joystick)
 */
void APP_BSP_PostIdle( void )
{
#if (CFG_LPM_STDBY_SUPPORTED != 0)
#if (CFG_BUTTON_SUPPORTED == 1)
  /* Treatment of WakeUp Button */
  if ( ( PWR->WUSR & PWR_WAKEUP_PIN2 ) != 0 )
  {
    PWR->WUSCR = PWR_WAKEUP_PIN2;
    BSP_PB_Callback( B1 );
  }
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
#if (CFG_JOYSTICK_SUPPORTED == 1)
  if ( JOY_StandbyExitFlag == 1u )
  {
    /* Could reconfigure Joystick here */
    JOY_StandbyExitFlag = 0;
  }
#endif /* CFG_JOYSTICK_SUPPORTED */
#endif /* (CFG_LPM_STDBY_SUPPORTED != 0) */
}

/**
 * @brief   Re-Initialisation of all used BSP after a StandBy.
 */
void APP_BSP_StandbyExit( void )
{
#if (CFG_LED_SUPPORTED == 1)
  /* Leds Initialization */
  //BSP_LED_Init(LED_BLUE);
#ifdef CFG_BSP_ON_NUCLEO
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
#endif /* CFG_BSP_ON_NUCLEO */
#endif /* (CFG_LED_SUPPORTED == 1) */

#if (CFG_BUTTON_SUPPORTED == 1)
#ifdef CFG_BSP_ON_DISCOVERY
  /* Joystick HW Initialization */
  BSP_JOY_Init( JOY1, JOY_MODE_EXTI, JOY_ALL );
#endif /* CFG_BSP_ON_DISCOVERY */

#ifdef CFG_BSP_ON_CEB
  /* Button HW Initialization */
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
#endif /* CFG_BSP_ON_CEB */

#ifdef CFG_BSP_ON_NUCLEO
  /* Buttons HW Initialization */
  BSP_PB_Init( B1, BUTTON_MODE_EXTI );
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
  BSP_PB_Init( B3, BUTTON_MODE_EXTI );
#endif /* CFG_BSP_ON_NUCLEO */
#endif /* (CFG_BUTTON_SUPPORTED == 1) */
}

#if ( CFG_BUTTON_SUPPORTED == 1 )

/**
 * @brief   Indicate if the selected button was pressedn during a 'long time' or not.
 *
 * @param   btnIdx    Button to test, listed in enum Button_TypeDef
 * @return  '1' if pressed during a 'long time', else '0'.
 */
uint8_t APP_BSP_ButtonIsLongPressed( uint16_t btnIdx )
{
  uint8_t pressStatus = 0;

  if ( btnIdx < BUTTON_NB_MAX )
  {
    pressStatus = buttonDesc[btnIdx].longPressed;
    buttonDesc[btnIdx].longPressed = 0;
  }

  return pressStatus;
}

/**
 * @brief   'Manually' set the selected button as pressed during a 'long time'.
 *
 * @param   btnIdx    Button to test, listed in enum Button_TypeDef
 */
void APP_BSP_SetButtonIsLongPressed( uint16_t btnIdx )
{
  buttonDesc[btnIdx].longPressed = 1;
}

/**
 * @brief  Action of button 1 when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_Button1Action( void )
{
}

/**
 * @brief  Action of button 2 when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_Button2Action( void )
{
}

/**
 * @brief  Action of button 3 when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_Button3Action( void )
{
}

#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */
#if ( CFG_JOYSTICK_SUPPORTED == 1 )

/**
 * @brief  Action of Joystick UP when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickUpAction( void )
{
}

/**
 * @brief  Action of Joystick RIGHT when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickRightAction( void )
{
}

/**
 * @brief  Action of Joystick DOWN when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickDownAction( void )
{
}

/**
 * @brief  Action of Joystick LEFT when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickLeftAction( void )
{
}

/**
 * @brief  Action of Joystick SELECT when pressed, to be implemented by user.
 * @param  None
 * @retval None
 */
__WEAK void APP_BSP_JoystickSelectAction( void )
{
}

#endif /* ( CFG_JOYSTICK_SUPPORTED == 1 ) */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

#if ( CFG_LED_SUPPORTED == 1 )

/**
 * @brief  Initialisation of all LED on used boards.
 */
void APP_BSP_LedInit( void )
{
  /* Leds Initialization */
#ifdef CFG_BSP_ON_DISCOVERY
#ifdef STM32WBA65xx  
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
#else // STM32WBA65xx
  BSP_LED_Init(LED_BLUE);
#endif // STM32WBA65xx
#endif // CFG_BSP_ON_DISCOVERY
  
#if CFG_BSP_ON_NUCLEO
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
#endif /* CFG_BSP_ON_NUCLEO */
  
#if CFG_BSP_ON_NUCLEO
  BSP_LED_Init(LED_BLUE);
#endif // CFG_BSP_ON_CEB
}

#endif /* (CFG_LED_SUPPORTED == 1) */
#ifdef CFG_BSP_ON_DISCOVERY
#if (CFG_LCD_SUPPORTED == 1)

/**
 * @brief  Initialisation of the LCD screen on used Discovery board.
 */
void APP_BSP_LcdInit( void )
{
  int32_t   iStatus;

  /* LCD Initialisation */
  iStatus = BSP_LCD_Init( LCD1, LCD_ORIENTATION_LANDSCAPE );
  if ( iStatus == BSP_ERROR_NONE )
  {
    iStatus = BSP_LCD_DisplayOn( LCD1 );
  }

  if ( iStatus == BSP_ERROR_NONE )
  {
    /* LCD Management Initialisation */
    UTIL_LCD_SetFuncDriver( &LCD_Driver );

    /* Clear the Background Layer */
    UTIL_LCD_Clear( LCD_COLOR_BLACK );

    /* Select font and Color */
    UTIL_LCD_SetFont( &Font12 );
    UTIL_LCD_SetBackColor( LCD_COLOR_BLACK );
    UTIL_LCD_SetTextColor( LCD_COLOR_WHITE );
  }
}

#endif /* (CFG_LCD_SUPPORTED == 1) */
#if (CFG_JOYSTICK_SUPPORTED == 1)

/**
 * @brief  Launch the Task selected by the joystick.
 *
 * @param  joystickState  position of joystick.
 * @retval None
 */
static void Joystick_LaunchActionTask( JOYPin_TypeDef joystickState )
{
  switch ( joystickState )
  {
#ifdef CFG_BSP_ON_FREERTOS
    case JOY_UP:
        osSemaphoreRelease( JoystickUpSemaphore );
        break;

    case JOY_RIGHT:
        osSemaphoreRelease( JoystickRightSemaphore );
        break;

    case JOY_DOWN:
        osSemaphoreRelease( JoystickDownSemaphore );
        break;

    case JOY_LEFT:
        osSemaphoreRelease( JoystickLeftSemaphore );
        break;

    case JOY_SEL:
        osSemaphoreRelease( JoystickSelectSemaphore );
        break;
#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_THREADX
    case JOY_UP:
        tx_semaphore_put( &JoystickUpSemaphore );
        break;

    case JOY_RIGHT:
        tx_semaphore_put( &JoystickRightSemaphore );
        break;

    case JOY_DOWN:
        tx_semaphore_put( &JoystickDownSemaphore );
        break;

    case JOY_LEFT:
        tx_semaphore_put( &JoystickLeftSemaphore );
        break;

    case JOY_SEL:
        tx_semaphore_put( &JoystickSelectSemaphore );
        break;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_SEQUENCER
    case JOY_UP:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_JOY_UP, CFG_SEQ_PRIO_0 );
        break;

    case JOY_RIGHT:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_JOY_RIGHT, CFG_SEQ_PRIO_0 );
        break;

    case JOY_DOWN:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_JOY_DOWN, CFG_SEQ_PRIO_0 );
        break;

    case JOY_LEFT:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_JOY_LEFT, CFG_SEQ_PRIO_0 );
        break;

    case JOY_SEL:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_JOY_SELECT, CFG_SEQ_PRIO_0 );
        break;
#endif /* CFG_BSP_ON_SEQUENCER */
    default :   /* No Action */
                break;
  }
}

/**
 *
 */
static void APP_BSP_JoystickTimerCallback(void *arg)
{
  static int32_t  joystickPreviousState = JOY_NONE;
  int32_t         joystickState;

  /* Init, Sample & DeInit Joystick */
  BSP_JOY_Init( JOY1, JOY_MODE_EXTI, JOY_ALL );

  /* Wait first conversion */
  HAL_ADC_PollForConversion( &hjoy_adc[JOY1], ( JOYSTICK_PRESS_SAMPLE_MS / 2u ) );
  joystickState = BSP_JOY_GetState( JOY1 );

  BSP_JOY_DeInit(JOY1, JOY_ALL);

  /* Process Joystick information */
  if ( ( joystickState != JOY_NONE ) && ( joystickState != joystickPreviousState ) )
  {
    Joystick_LaunchActionTask( (JOYPin_TypeDef) joystickState );
  }

  joystickPreviousState = joystickState;
}

#ifdef CFG_BSP_ON_FREERTOS

/**
 * @brief  Management of the Joystick Up task
 * @param  argument  Not used.
 * @retval None
 */
static void JoystickUpTask( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( JoystickUpSemaphore, osWaitForever );
    APP_BSP_JoystickUpAction();
  }
}

/**
 * @brief  Management of the Joystick Right task
 * @param  argument  Not used.
 * @retval None
 */
static void JoystickRightTask( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( JoystickRightSemaphore, osWaitForever );
    APP_BSP_JoystickRightAction();
  }
}

/**
 * @brief  Management of the Joystick Down task
 * @param  argument  Not used.
 * @retval None
 */
static void JoystickDownTask( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( JoystickDownSemaphore, osWaitForever );
    APP_BSP_JoystickDownAction();
  }
}

/**
 * @brief  Management of the Joystick Left task
 * @param  argument  Not used.
 * @retval None
 */
static void JoystickLeftTask( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( JoystickLeftSemaphore, osWaitForever );
    APP_BSP_JoystickLeftAction();
  }
}

/**
 * @brief  Management of the Joystick Select task
 * @param  argument  Not used.
 * @retval None
 */
static void JoystickSelectTask( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( JoystickSelectSemaphore, osWaitForever );
    APP_BSP_JoystickSelectAction();
  }
}

/**
 * @brief  Initialisation of the Joystick Tasks & Semaphores
 */
static void Joystick_InitTask( void )
{
  /* Register Semaphore & Task for the Joystick Up Semaphore */
  JoystickUpSemaphore = osSemaphoreNew( 1, 0, NULL );
  JoystickUpThread = osThreadNew( JoystickUpTask, NULL, &JoystickUpThreadAttributes );

  /* Register Semaphore & Task for the Joystick Up Semaphore */
  JoystickRightSemaphore = osSemaphoreNew( 1, 0, NULL );
  JoystickRightThread = osThreadNew( JoystickRightTask, NULL, &JoystickRightThreadAttributes );

  /* Register Semaphore & Task for the Joystick Up Semaphore */
  JoystickDownSemaphore = osSemaphoreNew( 1, 0, NULL );
  JoystickDownThread = osThreadNew( JoystickDownTask, NULL, &JoystickDownThreadAttributes );

  /* Register Semaphore & Task for the Joystick Up Semaphore */
  JoystickLeftSemaphore = osSemaphoreNew( 1, 0, NULL );
  JoystickLeftThread = osThreadNew( JoystickLeftTask, NULL, &JoystickLeftThreadAttributes );

  /* Register Semaphore & Task for the Joystick Up Semaphore */
  JoystickSelectSemaphore = osSemaphoreNew( 1, 0, NULL );
  JoystickSelectThread = osThreadNew( JoystickSelectTask, NULL, &JoystickSelectThreadAttributes );

  if ( ( JoystickUpSemaphore == NULL ) || ( JoystickRightSemaphore == NULL ) || ( JoystickDownSemaphore == NULL )
      || ( JoystickLeftSemaphore == NULL ) || ( JoystickSelectSemaphore == NULL ) )
  {
    LOG_ERROR_APP( "FreeRtos : Error during creation of Semaphore for Joystick" );
    while(1);
  }

  if ( ( JoystickUpThread == NULL ) || ( JoystickRightThread == NULL ) || ( JoystickDownThread == NULL )
      || ( JoystickLeftThread == NULL ) || ( JoystickSelectThread == NULL ) )
  {
    LOG_ERROR_APP( "FreeRtos : Error during creation of Task for Joystick" );
    while(1);
  }
}

#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_THREADX

/**
 * @brief  Management of the Joystick Up task
 * @param  lArgument  Not used.
 * @retval None
 */
static void JoystickUpTask( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &JoystickUpSemaphore, TX_WAIT_FOREVER );
    APP_BSP_JoystickUpAction();
  }
}

/**
 * @brief  Management of the Joystick Right task
 * @param  lArgument  Not used.
 * @retval None
 */
static void JoystickRightTask( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &JoystickRightSemaphore, TX_WAIT_FOREVER );
    APP_BSP_JoystickRigthAction();
  }
}

/**
 * @brief  Management of the Joystick Down task
 * @param  lArgument  Not used.
 * @retval None
 */
static void JoystickDownTask( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &JoystickDownSemaphore, TX_WAIT_FOREVER );
    APP_BSP_JoystickDownAction();
  }
}

/**
 * @brief  Management of the Joystick Left task
 * @param  lArgument  Not used.
 * @retval None
 */
static void JoystickLeftTask( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &JoystickLeftSemaphore, TX_WAIT_FOREVER );
    APP_BSP_JoystickLeftAction();
  }
}

/**
 * @brief  Management of the Joystick Select task
 * @param  lArgument  Not used.
 * @retval None
 */
static void JoystickSelectTask( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &JoystickSelectSemaphore, TX_WAIT_FOREVER );
    APP_BSP_JoystickSelectAction();
  }
}

/**
 * @brief  Initialisation of the Joystick Tasks & Semaphores
 */
static void Joystick_InitTask( void )
{
  UINT    ThreadXStatus;
  CHAR    *pStack;

  /* Register Semaphore to launch the Joystick Up Task */
  ThreadXStatus = tx_semaphore_create( &JoystickUpSemaphore, "JoystickUp Semaphore", 0 );
  ThreadXStatus |= tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_JOYSTICK_UP, TX_NO_WAIT);
  if ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_thread_create( &JoystickUpThread, "JoystickUp Thread", JoystickUpTask, 0, pStack,
                                      TASK_STACK_SIZE_JOYSTICK_UP, TASK_PRIO_JOYSTICK_UP, TASK_PREEMP_JOYSTICK_UP,
                                      TX_NO_TIME_SLICE, TX_AUTO_START );
  }

  /* Register Semaphore to launch the Joystick Right Task */
  f ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_semaphore_create( &JoystickRightSemaphore, "JoystickRight Semaphore", 0 );
    ThreadXStatus |= tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_JOYSTICK_RIGHT, TX_NO_WAIT);
    if ( ThreadXStatus == TX_SUCCESS )
    {
      ThreadXStatus = tx_thread_create( &JoystickRightThread, "JoystickRight Thread", JoystickRightTask, 0, pStack,
                                        TASK_STACK_SIZE_JOYSTICK_RIGHT, TASK_PRIO_JOYSTICK_RIGHT, TASK_PREEMP_JOYSTICK_RIGHT,
                                        TX_NO_TIME_SLICE, TX_AUTO_START );
    }
  }

  /* Register Semaphore to launch the Joystick Down Task */
  f ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_semaphore_create( &JoystickDownSemaphore, "JoystickDown Semaphore", 0 );
    ThreadXStatus |= tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_JOYSTICK_DOWN, TX_NO_WAIT);
    if ( ThreadXStatus == TX_SUCCESS )
    {
      ThreadXStatus = tx_thread_create( &JoystickDownThread, "JoystickDown Thread", JoystickDownTask, 0, pStack,
                                        TASK_STACK_SIZE_JOYSTICK_DOWN, TASK_PRIO_JOYSTICK_DOWN, TASK_PREEMP_JOYSTICK_DOWN,
                                        TX_NO_TIME_SLICE, TX_AUTO_START );
    }
  }

  /* Register Semaphore to launch the Joystick Left Task */
  if ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_semaphore_create( &JoystickLeftSemaphore, "JoystickLeft Semaphore", 0 );
    ThreadXStatus |= tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_JOYSTICK_LEFT, TX_NO_WAIT);
    if ( ThreadXStatus == TX_SUCCESS )
    {
      ThreadXStatus = tx_thread_create( &JoystickLeftThread, "JoystickLeft Thread", JoystickLeftTask, 0, pStack,
                                        TASK_STACK_SIZE_JOYSTICK_LEFT, TASK_PRIO_JOYSTICK_LEFT, TASK_PREEMP_JOYSTICK_LEFT,
                                        TX_NO_TIME_SLICE, TX_AUTO_START );
    }
  }

    /* Register Semaphore to launch the Joystick Select Task */
  if ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_semaphore_create( &JoystickLeftSemaphore, "JoystickSelect Semaphore", 0 );
    ThreadXStatus |= tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_JOYSTICK_SELECT, TX_NO_WAIT);
    if ( ThreadXStatus == TX_SUCCESS )
    {
      ThreadXStatus = tx_thread_create( &JoystickSelectThread, "JoystickSelect Thread", JoystickSelectTask, 0, pStack,
                                        TASK_STACK_SIZE_JOYSTICK_SELECT, TASK_PRIO_JOYSTICK_SELECT, TASK_PREEMP_JOYSTICK_SELECT,
                                        TX_NO_TIME_SLICE, TX_AUTO_START );
    }
  }

  /* Verify if it's OK */
  if ( ThreadXStatus != TX_SUCCESS )
  {
    LOG_ERROR_APP( "ThreadX : Error during the creation of Semaphore & Task for Joystick (0x%04X)", ThreadXStatus );
    while(1);
  }
}

#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_SEQUENCER

/**
 * @brief  Initialisation of the Joystick Tasks & Semaphores
 */
static void Joystick_InitTask( void)
{
  /* Task associated with Joystick */
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_JOY_UP, UTIL_SEQ_RFU, APP_BSP_JoystickUpAction );
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_JOY_RIGHT, UTIL_SEQ_RFU, APP_BSP_JoystickRightAction );
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_JOY_DOWN, UTIL_SEQ_RFU, APP_BSP_JoystickDownAction );
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_JOY_LEFT, UTIL_SEQ_RFU, APP_BSP_JoystickLeftAction );
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_JOY_SELECT, UTIL_SEQ_RFU, APP_BSP_JoystickSelectAction );
}

#endif /* CFG_BSP_ON_SEQUENCER */

/**
 * @brief  Initialisation of the Joystick
 */
void APP_BSP_JoystickInit( void )
{
  /* Button task initialisation */
  Joystick_InitTask();

  /* StandBy WakeUp via Joystick */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3_HIGH_1);              /* JOY-PA1. */

  /* Create periodic timer for joystick position reading */
  UTIL_TIMER_Create(&joystickTimer, JOYSTICK_PRESS_SAMPLE_MS, UTIL_TIMER_PERIODIC, &APP_BSP_JoystickTimerCallback, 0);
  UTIL_TIMER_Start(&joystickTimer);
}

#endif /* (CFG_JOYSTICK_SUPPORTED == 1) */
#endif /* CFG_BSP_ON_DISCOVERY */
#if ( CFG_BUTTON_SUPPORTED == 1 )
#ifdef CFG_BSP_ON_FREERTOS

/**
 * @brief  Management of the B1 pushbutton task
 * @param  argument  Not used.
 * @retval None
 */
static void ButtonB1Task( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( ButtonB1Semaphore, osWaitForever );
    APP_BSP_Button1Action();
  }
}

/**
 * @brief  Management of the B2 pushbutton task
 * @param  argument  Not used.
 * @retval None
 */
static void ButtonB2Task( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( ButtonB2Semaphore, osWaitForever );
    APP_BSP_Button2Action();
  }
}

/**
 * @brief  Management of the B3 pushbutton task
 * @param  argument  Not used.
 * @retval None
 */
static void ButtonB3Task( void * argument )
{
  UNUSED( argument );

  for(;;)
  {
    osSemaphoreAcquire( ButtonB3Semaphore, osWaitForever );
    APP_BSP_Button3Action();
  }
}

static void Button_InitTask( void )
{
  /* Register Semaphore & Thread to launch the pushbutton B1 Task */
  ButtonB1Semaphore = osSemaphoreNew( 1, 0, NULL );
  ButtonB1Thread = osThreadNew( ButtonB1Task, NULL, &ButtonB1ThreadAttributes );

  /* Register Semaphore & Thread to launch the pushbutton B2 Task */
  ButtonB2Semaphore = osSemaphoreNew( 1, 0, NULL );
  ButtonB2Thread = osThreadNew( ButtonB2Task, NULL, &ButtonB2ThreadAttributes );

  /* Register Semaphore & Thread to launch the pushbutton B3 Task */
  ButtonB3Semaphore = osSemaphoreNew( 1, 0, NULL );
  ButtonB3Thread = osThreadNew( ButtonB3Task, NULL, &ButtonB3ThreadAttributes );

  if ( ( ButtonB1Semaphore == NULL ) || ( ButtonB2Semaphore == NULL ) || ( ButtonB3Semaphore == NULL ) )
  {
    LOG_ERROR_APP( "FreeRtos : Error during creation of Semaphore for Buttons" );
    while(1);
  }

  if ( ( ButtonB1Thread == NULL ) || ( ButtonB2Thread == NULL ) || ( ButtonB3Thread == NULL ) )
  {
    LOG_ERROR_APP( "FreeRtos : Error during creation of Thread for Buttons" );
    while(1);
  }
}

#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_THREADX

/**
 * @brief  Management of the B1 pushbutton task
 * @param  lArgument  Not used.
 * @retval None
 */
static void ButtonB1Task( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &ButtonB1Semaphore, TX_WAIT_FOREVER );
    APP_BSP_Button1Action();
  }
}

/**
 * @brief  Management of the B2 pushbutton task
 * @param  lArgument  Not used.
 * @retval None
 */
static void ButtonB2Task( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &ButtonB2Semaphore, TX_WAIT_FOREVER );
    APP_BSP_Button2Action();
  }
}

/**
 * @brief  Management of the B3 pushbutton task
 * @param  lArgument  Not used.
 * @retval None
 */
static void ButtonB3Task( ULONG lArgument )
{
  UNUSED( lArgument );

  for(;;)
  {
    tx_semaphore_get( &ButtonB3Semaphore, TX_WAIT_FOREVER );
    APP_BSP_Button3Action();
  }
}

static void Button_InitTask( void )
{
  UINT    ThreadXStatus;
  CHAR    *pStack;

  /* Register Semaphore to launch the pushbutton B1 Task */
  ThreadXStatus = tx_semaphore_create( &ButtonB1Semaphore, "ButtonB1 Semaphore", 0 );
  ThreadXStatus |= tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_BUTTON_B1, TX_NO_WAIT);
  if ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_thread_create( &ButtonB1Thread, "ButtonB1 Thread", ButtonB1Task, 0, pStack,
                                      TASK_STACK_SIZE_BUTTON_B1, TASK_PRIO_BUTTON_B1, TASK_PREEMP_BUTTON_B1,
                                      TX_NO_TIME_SLICE, TX_AUTO_START );
  }

  /* Register Semaphore to launch the pushbutton B2 Task */
  if ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_semaphore_create( &ButtonB2Semaphore, "ButtonB2 Semaphore", 0 );
    ThreadXStatus |= tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_BUTTON_B2, TX_NO_WAIT);
    if ( ThreadXStatus == TX_SUCCESS )
    {
      ThreadXStatus = tx_thread_create( &ButtonB2Thread, "ButtonB2 Thread", ButtonB2Task, 0, pStack,
                                        TASK_STACK_SIZE_BUTTON_B2, TASK_PRIO_BUTTON_B2, TASK_PREEMP_BUTTON_B2,
                                        TX_NO_TIME_SLICE, TX_AUTO_START );
    }
  }

  /* Register Semaphore to launch the pushbutton B3 Task */
  if ( ThreadXStatus == TX_SUCCESS )
  {
    ThreadXStatus = tx_semaphore_create( &ButtonB3Semaphore, "ButtonB3 Semaphore", 0 );
    ThreadXStatus = tx_byte_allocate( pBytePool, (VOID**) &pStack, TASK_STACK_SIZE_BUTTON_B3, TX_NO_WAIT);
    if ( ThreadXStatus == TX_SUCCESS )
    {
      ThreadXStatus = tx_thread_create( &ButtonB3Thread, "ButtonB3 Thread", ButtonB3Task, 0, pStack,
                                        TASK_STACK_SIZE_BUTTON_B3, TASK_PRIO_BUTTON_B3, TASK_PREEMP_BUTTON_B3,
                                        TX_NO_TIME_SLICE, TX_AUTO_START );
    }
  }

  /* Verify if it's OK */
  if ( ThreadXStatus != TX_SUCCESS )
  {
    LOG_ERROR_APP( "ThreadX : Error during the creation of Semaphore & Task for Buttons (0x%04X)", ThreadXStatus );
    while(1);
  }
}

#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_SEQUENCER

/**
 * @brief  Initialisation of Buttons Tasks & Semaphores
 */
static void Button_InitTask( void )
{
#ifdef CFG_BSP_ON_CEB
  /* Task associated with push button B2 */
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_BUTTON_B2, UTIL_SEQ_RFU, APP_BSP_Button2Action );
#else /* CFG_BSP_ON_CEB */
  /* Task associated with push button B1 */
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_BUTTON_B1, UTIL_SEQ_RFU, APP_BSP_Button1Action );

  /* Task associated with push button B2 */
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_BUTTON_B2, UTIL_SEQ_RFU, APP_BSP_Button2Action );

  /* Task associated with push button B3 */
  UTIL_SEQ_RegTask( 1U << CFG_TASK_BSP_BUTTON_B3, UTIL_SEQ_RFU, APP_BSP_Button3Action );
#endif /* CFG_BSP_ON_CEB */
}

#endif /* CFG_BSP_ON_SEQUENCER */

/**
 * @brief  Initialisation of Buttons
 */
void APP_BSP_ButtonInit( void )
{
  Button_TypeDef  buttonIndex;

  /* Buttons HW Initialization */
#ifdef CFG_BSP_ON_CEB
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
#endif /* CFG_BSP_ON_CEB */
#ifdef CFG_BSP_ON_NUCLEO
  BSP_PB_Init( B1, BUTTON_MODE_EXTI );
  BSP_PB_Init( B2, BUTTON_MODE_EXTI );
  BSP_PB_Init( B3, BUTTON_MODE_EXTI );

#if (CFG_LPM_STDBY_SUPPORTED != 0)
  /* StandBy WakeUp via buttons */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_LOW_1);   /* WakeUp on B1 - GPIO PC13. */
#ifdef STM32WBA65xx
  SET_BIT(PWR->IORETENRC, ( B1_PIN | B2_PIN ) );    /* Retention for B1 & B2 */       
  SET_BIT(PWR->IORETENRB, B3_PIN);                  /* Retention for B3 */
#else /* STM32WBA65xx */
  SET_BIT(PWR->IORETENRC, B1_PIN );                 /* Retention for B1 */       
  SET_BIT(PWR->IORETENRB, ( B2_PIN | B3_PIN ) );    /* Retention for B2 & B3 */
#endif /* STM32WBA65xx  */
#endif /* (CFG_LPM_STDBY_SUPPORTED != 0)   */
#endif /* CFG_BSP_ON_NUCLEO */

  /* Button task initialisation */
  Button_InitTask();

  /* StandBy WakeUp via buttons */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH_1);              /* GPIO PC13. */

  /* Button timers initialisation (one for each button) */
#ifdef CFG_BSP_ON_CEB
  buttonIndex = B2;
  UTIL_TIMER_Create( &buttonDesc[buttonIndex].longTimerId, 0, UTIL_TIMER_PERIODIC, &Button_TriggerActions, &buttonDesc[buttonIndex] );
#else /* CFG_BSP_ON_CEB */
  for ( buttonIndex = B1; buttonIndex < BUTTON_NB_MAX; buttonIndex++ )
  {
    UTIL_TIMER_Create( &buttonDesc[buttonIndex].longTimerId, 0, UTIL_TIMER_PERIODIC, &Button_TriggerActions, &buttonDesc[buttonIndex] );
  }
#endif /* CFG_BSP_ON_CEB */
}

/**
 * @brief  Launch the Task selected by the button.
 *
 * @param  button  ID of pressed button.
 * @retval None
 */
static void Button_LaunchActionTask( Button_TypeDef button )
{
#ifdef CFG_BSP_ON_CEB
  if ( button == B2 )
  {
#ifdef CFG_BSP_ON_FREERTOS
    osSemaphoreRelease( ButtonB2Semaphore );
#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_THREADX
    tx_semaphore_put( &ButtonB2Semaphore );
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_SEQUENCER
    UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B2, CFG_SEQ_PRIO_0 );
#endif /* CFG_BSP_ON_SEQUENCER */
  }
#endif /* CFG_BSP_ON_CEB */
#ifdef CFG_BSP_ON_NUCLEO
  switch ( button )
  {
#ifdef CFG_BSP_ON_FREERTOS
    case B1:
        osSemaphoreRelease( ButtonB1Semaphore );
        break;

    case B2:
        osSemaphoreRelease( ButtonB2Semaphore );
        break;

    case B3:
        osSemaphoreRelease( ButtonB3Semaphore );
        break;
#endif /* CFG_BSP_ON_FREERTOS */
#ifdef CFG_BSP_ON_THREADX
    case B1:
        tx_semaphore_put( &ButtonB1Semaphore );
        break;

    case B2:
        tx_semaphore_put( &ButtonB2Semaphore );
        break;

    case B3:
        tx_semaphore_put( &ButtonB3Semaphore );
        break;
#endif /* CFG_BSP_ON_THREADX */
#ifdef CFG_BSP_ON_SEQUENCER
    case B1:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B1, CFG_SEQ_PRIO_0 );
        break;

    case B2:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B2, CFG_SEQ_PRIO_0 );
        break;

    case B3:
        UTIL_SEQ_SetTask( 1U << CFG_TASK_BSP_BUTTON_B3, CFG_SEQ_PRIO_0 );
        break;
#endif /* CFG_BSP_ON_SEQUENCER */

    default:
        break;
  }
#endif /* CFG_BSP_ON_NUCLEO */
}

/**
 *
 */
static void Button_TriggerActions( void * arg )
{
  ButtonDesc_t    * p_buttonDesc = arg;
  int32_t         buttonState = 0;

  buttonState = BSP_PB_GetState( p_buttonDesc->button );

  /* If Button pressed and Threshold time not finish, continue waiting */
  p_buttonDesc->waitingTime += BUTTON_LONG_PRESS_SAMPLE_MS;
  if ( ( buttonState != 0u ) && ( p_buttonDesc->waitingTime < BUTTON_LONG_PRESS_THRESHOLD_MS ) )
  {
    return;
  }

  /* Save button state */
  if ( buttonState != 0u )
  {
    APP_BSP_SetButtonIsLongPressed( p_buttonDesc->button );
  }

  /* Stop Timer */
  UTIL_TIMER_Stop( &p_buttonDesc->longTimerId );

  /* Launch Task */
  Button_LaunchActionTask( p_buttonDesc->button );
}

/**
 *
 */
void BSP_PB_Callback( Button_TypeDef button )
{
  buttonDesc[button].waitingTime = 0;
  UTIL_TIMER_StartWithPeriod( &buttonDesc[button].longTimerId, BUTTON_LONG_PRESS_SAMPLE_MS );
}

#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */

/**
 * @brief  Treat USART commands to simulate button press for instance.
 *
 * @param  pRxBuffer      Pointer on received data from USART.
 * @param  iRxBufferSize  Number of received data.
 * @retval 1 if function can be execute a command else 0.
 */
uint8_t APP_BSP_SerialCmdExecute( uint8_t * pRxBuffer, uint16_t iRxBufferSize )
{
  uint8_t   cReturn = 0;
  uint16_t  iUserChoice = UINT16_MAX;

  /* Parse received frame */
#if ( CFG_BUTTON_SUPPORTED == 1 )
#ifdef CFG_BSP_ON_CEB
  /* For Board B_WBA5M_WPAN, the only button (same as B1 on other board) is named B2. */
  if ( (strcmp( (char const*)pRxBuffer, "B2" ) == 0) ||
       (strcmp( (char const*)pRxBuffer, "SW2" ) == 0) )
  {
    iUserChoice = B2;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B2L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW2L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B2);
    iUserChoice = B2;
  }
#else /* CFG_BSP_ON_CEB */
  if ( (strcmp( (char const*)pRxBuffer, "B1" ) == 0) ||
       (strcmp( (char const*)pRxBuffer, "SW1" ) == 0) )
  {
    iUserChoice = B1;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B1L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW1L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B1);
    iUserChoice = B1;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B2" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW2" ) == 0) )
  {
    iUserChoice = B2;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B2L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW2L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B2);
    iUserChoice = B2;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B3" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW3" ) == 0) )
  {
    iUserChoice = B3;
  }
  else if ( (strcmp( (char const*)pRxBuffer, "B3L" ) == 0) ||
            (strcmp( (char const*)pRxBuffer, "SW3L" ) == 0) )
  {
    APP_BSP_SetButtonIsLongPressed(B3);
    iUserChoice = B3;
  }
#endif /* CFG_BSP_ON_CEB */
#endif /* ( CFG_BUTTON_SUPPORTED == 1 )  */
#if ( CFG_JOYSTICK_SUPPORTED == 1 )
  if (strcmp( (char const*)pRxBuffer, "UP" ) == 0)
  {
    iUserChoice = JOY_UP;
  }
  else if (strcmp( (char const*)pRxBuffer, "RIGHT" ) == 0)
  {
    iUserChoice = JOY_RIGHT;
  }
  else if (strcmp( (char const*)pRxBuffer, "DOWN" ) == 0 )
  {
    iUserChoice = JOY_DOWN;
  }
  else if (strcmp( (char const*)pRxBuffer, "LEFT" ) == 0)
  {
    iUserChoice = JOY_LEFT;
  }
  else if (strcmp( (char const*)pRxBuffer, "SELECT" ) == 0)
  {
    iUserChoice = JOY_SEL;
  }
#endif /* ( CFG_JOYSTICK_SUPPORTED == 1 )  */

  if ( iUserChoice != UINT16_MAX )
  {
    /* Launch Command */
    LOG_INFO_APP( "%s pressed by Serial Command.", pRxBuffer );
#if ( CFG_BUTTON_SUPPORTED == 1 )
    Button_LaunchActionTask( (Button_TypeDef)iUserChoice );
#endif /* ( CFG_BUTTON_SUPPORTED == 1 ) */
#if ( CFG_JOYSTICK_SUPPORTED == 1 )
    Joystick_LaunchActionTask( (JOYPin_TypeDef)iUserChoice );
#endif /* ( CFG_JOYSTICK_SUPPORTED == 1 ) */
    cReturn = 1;
  }

  return cReturn;
}

