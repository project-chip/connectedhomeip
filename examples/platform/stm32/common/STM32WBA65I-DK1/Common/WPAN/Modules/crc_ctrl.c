/**
  ******************************************************************************
  * @file    crc_ctrl.c
  * @author  MCD Application Team
  * @brief   Source for CRC client controller module
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

/* Utilities */
#include "utilities_common.h"

/* Own header files */
#include "crc_ctrl.h"
#include "crc_ctrl_conf.h"

/* HAL CRC header */
#include "stm32wbaxx_hal_crc.h"

/* Private defines -----------------------------------------------------------*/
/**
 * @brief Initial value define for configuration tracking number
 *
 */
#define CRCCTRL_NO_CONFIG   (uint32_t)(0x00000000u)

/* Private typedef -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
 * @brief Tracker of the current applied configuration
 */
static uint32_t CurrentConfig = CRCCTRL_NO_CONFIG;

/**
 * @brief Higher registered handle ID
 */
static uint32_t MaxRegisteredId = CRCCTRL_NO_CONFIG;

/**
 * @brief Handle of the HAL CRC
 */
static CRC_HandleTypeDef CRCHandle =
{
  .Instance = CRCCTRL_HWADDR,
};

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  Configure CRC IP
  * @param  p_Handle: CRC handle
  * @retval State of the configuration
  */
static inline HAL_StatusTypeDef CrcConfigure (CRCCTRL_Handle_t * const p_Handle);

/* Functions Definition ------------------------------------------------------*/
__WEAK CRCCTRL_Cmd_Status_t CRCCTRL_Init (void)
{
  CRCCTRL_Cmd_Status_t error = CRCCTRL_UNKNOWN;

  /* Try to take the CRC mutex */
  error = CRCCTRL_MutexTake ();

  if (CRCCTRL_OK == error)
  {
    CurrentConfig = CRCCTRL_NO_CONFIG;

    CRCHandle.State = HAL_CRC_STATE_RESET;

    CRCHandle.Instance = CRCCTRL_HWADDR;

    /* Release the mutex */
    CRCCTRL_MutexRelease ();
  }

  return error;
}

__WEAK CRCCTRL_Cmd_Status_t CRCCTRL_RegisterHandle (CRCCTRL_Handle_t * const p_Handle)
{
  CRCCTRL_Cmd_Status_t error = CRCCTRL_UNKNOWN;

  if (NULL == p_Handle)
  {
    error = CRCCTRL_ERROR_NULL_POINTER;
  }
  else if (HANDLE_REG == p_Handle->State)
  {
    error = CRCCTRL_HANDLE_ALREADY_REGISTERED;
  }
  else
  {
    /* Try to take the CRC mutex */
    error = CRCCTRL_MutexTake ();

    if (CRCCTRL_OK == error)
    {
      /* Update the maximum registered handle */
      MaxRegisteredId = MaxRegisteredId + 1u;
      /* Update the handle UUID */
      p_Handle->Uid = MaxRegisteredId;
      /* Init the previous value */
      p_Handle->PreviousComputedValue = p_Handle->Configuration.InitValue;
      /* Set handle as initialized */
      p_Handle->State = HANDLE_REG;

      /* Release the mutex */
      CRCCTRL_MutexRelease ();
    }
  }

  return error;
}

__WEAK CRCCTRL_Cmd_Status_t CRCCTRL_Calculate (CRCCTRL_Handle_t * const p_Handle,
                                               uint32_t a_Payload[],
                                               const uint32_t PayloadSize,
                                               uint32_t * const p_ConmputedValue)
{
  CRCCTRL_Cmd_Status_t error = CRCCTRL_UNKNOWN;
  HAL_StatusTypeDef eReturn = HAL_OK;

  /* Null pointer for handle or payload */
  if ((NULL == p_Handle) || (NULL == a_Payload) || (NULL == p_ConmputedValue))
  {
    error = CRCCTRL_ERROR_NULL_POINTER;
  }
  /* Handle not init */
  else if (HANDLE_NOT_REG == p_Handle->State)
  {
    error = CRCCTRL_HANDLE_NOT_REGISTERED;
  }
  /* Handle not in the range */
  else if ((MaxRegisteredId < p_Handle->Uid) ||
           (CRCCTRL_NO_CONFIG >= p_Handle->Uid))
  {
    error = CRCCTRL_HANDLE_NOT_VALID;
  }
  else
  {
    /* Try to take the CRC mutex */
    error = CRCCTRL_MutexTake ();

    if (CRCCTRL_OK == error)
    {
      /* Is the current config IS NOT the same as the one requested ? */
      if (CurrentConfig != p_Handle->Uid)
      {
        /* Configure the CRC before use */
        eReturn = CrcConfigure (p_Handle);
      }

      if (eReturn == HAL_OK)
      {
        *p_ConmputedValue = HAL_CRC_Calculate (&CRCHandle, a_Payload, PayloadSize);

        /* Update the handle with the computed value */
        p_Handle->PreviousComputedValue = *p_ConmputedValue;
      }
      else
      {
        error = CRCCTRL_ERROR_CONFIG;
      }

      /* Release the mutex */
      CRCCTRL_MutexRelease ();
    }
  }

  return error;
}

__WEAK CRCCTRL_Cmd_Status_t CRCCTRL_Accumulate (CRCCTRL_Handle_t * const p_Handle,
                                                uint32_t a_Payload[],
                                                const uint32_t PayloadSize,
                                                uint32_t * const p_ConmputedValue)
{
  CRCCTRL_Cmd_Status_t error = CRCCTRL_UNKNOWN;

  /* Null pointer for handle or payload */
  if ((NULL == p_Handle) || (NULL == a_Payload) || (NULL == p_ConmputedValue))
  {
    error = CRCCTRL_ERROR_NULL_POINTER;
  }
  /* Handle not init */
  else if (HANDLE_NOT_REG == p_Handle->State)
  {
    error = CRCCTRL_HANDLE_NOT_REGISTERED;
  }
  /* Handle not in the range */
  else if ((MaxRegisteredId < p_Handle->Uid) ||
           (CRCCTRL_NO_CONFIG >= p_Handle->Uid))
  {
    error = CRCCTRL_HANDLE_NOT_VALID;
  }
  else
  {
    /* Try to take the CRC mutex */
    error = CRCCTRL_MutexTake ();

    if (CRCCTRL_OK == error)
    {
      /* Check if the config has to change */
      if (CurrentConfig == p_Handle->Uid)
      {
        *p_ConmputedValue = HAL_CRC_Accumulate (&CRCHandle,
                                                a_Payload,
                                                PayloadSize);

        /* Update the handle with the computed value */
        p_Handle->PreviousComputedValue = *p_ConmputedValue;
      }
      /* Configure the CRC before use */
      else if (HAL_OK == CrcConfigure (p_Handle))
      {
        /* Before starting the accumulation, the init register shall be written with the previous value */
        CRCHandle.Instance->INIT = p_Handle->PreviousComputedValue;

        *p_ConmputedValue = HAL_CRC_Calculate (&CRCHandle,
                                               a_Payload,
                                               PayloadSize);

        /* Update the handle with the computed value */
        p_Handle->PreviousComputedValue = *p_ConmputedValue;
      }
      else
      {
        error = CRCCTRL_ERROR_CONFIG;
      }

      /* Release the mutex */
      CRCCTRL_MutexRelease ();
    }
  }

  return error;
}

/* Private function Definition -----------------------------------------------*/
HAL_StatusTypeDef CrcConfigure (CRCCTRL_Handle_t * const p_Handle)
{
  HAL_StatusTypeDef error = HAL_OK;

  /* No need to DeInit if the CRC if it is not yet initialized */
  if (HAL_CRC_STATE_RESET != CRCHandle.State)
  {
    /* DeInit the CRC module */
    error = HAL_CRC_DeInit(&CRCHandle);
  }

  /* All OK ? */
  if (HAL_OK == error)
  {
    /* Fulfill the configuration part */
    CRCHandle.Init.CRCLength = p_Handle->Configuration.CRCLength;
    CRCHandle.Init.DefaultInitValueUse = p_Handle->Configuration.DefaultInitValueUse;
    CRCHandle.Init.DefaultPolynomialUse = p_Handle->Configuration.DefaultPolynomialUse;
    CRCHandle.Init.GeneratingPolynomial = p_Handle->Configuration.GeneratingPolynomial;
    CRCHandle.Init.InitValue = p_Handle->Configuration.InitValue;
    CRCHandle.Init.InputDataInversionMode = p_Handle->Configuration.InputDataInversionMode;
    CRCHandle.Init.OutputDataInversionMode = p_Handle->Configuration.OutputDataInversionMode;
    CRCHandle.InputDataFormat = p_Handle->Configuration.InputDataFormat;

    /* Apply the requested CRC configuration */
    error = HAL_CRC_Init(&CRCHandle);

    if (HAL_OK == error)
    {
      /* Update the current configuration */
      CurrentConfig = p_Handle->Uid;
    }
    else
    {
      /* There must be an issue with configuration, clean the configuration */
      memset ((void *)(&CRCHandle.Init),
              0x00,
              sizeof (CRC_InitTypeDef));

      CRCHandle.InputDataFormat = 0x00u;
    }
  }

  return error;
}

/* Weak function Definition --------------------------------------------------*/
__WEAK CRCCTRL_Cmd_Status_t CRCCTRL_MutexTake (void)
{
  return CRCCTRL_OK;
}

__WEAK CRCCTRL_Cmd_Status_t CRCCTRL_MutexRelease (void)
{
  return CRCCTRL_OK;
}
