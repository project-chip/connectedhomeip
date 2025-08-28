/**
  ******************************************************************************
  * @file    serial_cmd_interpreter.h
  * @author  MCD Application Team
  * @brief   Header file for the serial commands interpreter module.
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
#ifndef SERIAL_CMD_INTERPRETER_H
#define SERIAL_CMD_INTERPRETER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void Serial_CMD_Interpreter_Init(void);
void Serial_CMD_Interpreter_CmdExecute( uint8_t * pRxBuffer, uint16_t iRxBufferSize );

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SERIAL_CMD_INTERPRETER_H */
