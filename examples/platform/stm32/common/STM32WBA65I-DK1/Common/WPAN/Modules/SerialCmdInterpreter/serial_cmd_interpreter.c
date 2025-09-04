/**
  ******************************************************************************
  * @file    serial_cmd_interpreter.c
  * @author  MCD Application Team
  * @brief   Source file for the serial commands interpreter module.
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

/* Includes ------------------------------------------------------------------*/
#include "log_module.h"
#include "app_conf.h"
#include "stm32_adv_trace.h"
#include "serial_cmd_interpreter.h"

/* Private includes -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define RX_BUFF_SIZE       (256U)

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t  RxBuffer[RX_BUFF_SIZE];
static uint16_t indexRxBuffer = 0;

/* Global variables ----------------------------------------------------------*/

/* Private functions prototypes ----------------------------------------------*/
static void UART_Rx_Callback(uint8_t *PData, uint16_t Size, uint8_t Error);

/* External variables --------------------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
void Serial_CMD_Interpreter_Init(void)
{
  /* Init Communication reception. Need that Log/Traces are activated */
  UTIL_ADV_TRACE_StartRxProcess(UART_Rx_Callback);
}

__weak void Serial_CMD_Interpreter_CmdExecute( uint8_t * pRxBuffer, uint16_t iRxBufferSize )
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the enter_standby_notification could be implemented in the user file
  */
   
  /*
    This user function is in charge of interpreting the received data.
    
    Here is an example of how to use them.
    In this simple case, we'll generate a SW IT on GPIO14 if the received data is a string "TEST".
    Add the following code into the user code section :

    // Extended line handler on which we will generate the IT
    EXTI_HandleTypeDef exti_handle;

    // Check RxBuffer's content to know if we're matching our case
    if( strcmp( (char const*)pRxBuffer, "TEST" ) == 0 )
    {
      LOG_INFO_APP("TEST has been received in Uart_Cmd_Execute.\n");
      exti_handle.Line = EXTI_LINE_14;
      HAL_EXTI_GenerateSWI(&exti_handle);
    }
  */
}


/* Private functions definition ----------------------------------------------*/
static void UART_Rx_Callback( uint8_t * pData, uint16_t Size, uint8_t Error )
{
  /* Filling buffer and wait for '\r' charactere to execute actions */
  if ( indexRxBuffer < RX_BUFF_SIZE )
  {
    if ( *pData == '\r' )
    {
      Serial_CMD_Interpreter_CmdExecute( RxBuffer, indexRxBuffer );

      /* Clear receive buffer and character counter*/
      indexRxBuffer = 0;
      memset( &RxBuffer[0], 0, RX_BUFF_SIZE );
    }
    else
    {
      if ( ( * pData == '\n' ) && ( indexRxBuffer == 0 ) )
      {
        /* discard this first charactere if it's a delimiter  */
      }
      else
      {
        RxBuffer[indexRxBuffer++] = *pData;
      }
    }
  }
  else
  {
    indexRxBuffer = 0;
    memset(&RxBuffer[0], 0, RX_BUFF_SIZE);
  }
  return;
}



