/**
  ******************************************************************************
  * @file    pka_ctrl.h
  * @author  MCD Application Team
  * @brief   Header for PKA client manager module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef PKA_CTRL_H
#define PKA_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"
/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Enumerated values used for the return of the functions:
 * (warning: this enum must be aligned with BLEPLAT corresponding one)
 */
enum
{
  PKACTRL_OK    =  0,
  PKACTRL_FULL  = -1,
  PKACTRL_BUSY  = -2,
  PKACTRL_EOF   = -3,
  PKACTRL_ERROR = -5
};

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Reset the PKA controller module
 * @param  None
 * @retval None
 */
void PKACTRL_Reset(void);

/**
 * @brief  Start P-256 public key generation
 * @param  local_private_key: table of 8 x 32-bit words that contains the
 *         P-256 private key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_StartP256Key(const uint32_t* local_private_key);

/**
 * @brief  Get result of P-256 public key generation
 * @param  local_public_key: table of 32 x 32-bit words that is filled by the
 *         function with the generated P-256 public key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_ReadP256Key(uint32_t* local_public_key);

/**
 * @brief  Start DH key computation
 * @param  local_private_key: table of 8 x 32-bit words that contains the
 *         local P-256 private key (Little Endian format)
 * @param  remote_public_key: table of 32 x 32-bit words that contains the
 *         remote P-256 public key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_StartDhKey(const uint32_t* local_private_key,
                       const uint32_t* remote_public_key);

/**
 * @brief  Get result of DH key computation
 * @param  dh_key: table of 8 x 32-bit words that is filled by the function
 *         with the generated DH key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_ReadDhKey(uint32_t* dh_key);

/**
 * @brief  PKA controller background task
 * @param  None
 * @retval None
 */
void PKACTRL_BG_Process(void);

/**
 * @brief  Callback to call when a PKA Process has ended
 * @param  None
 * @retval None
 */
void PKACTRL_EndOfProcessCb(void);

/* Callback used by PKA Controller to indicate the end of the processing
 */
void PKACTRL_CB_Complete(void);

/* Callback used by PKA Controller to indicate when PKA background task needs to run */
void PKACTRL_CB_Process(void);

/* Exported functions to be implemented by the user ------------------------- */
extern int PKACTRL_MutexTake(void);
extern int PKACTRL_MutexRelease(void);
extern int PKACTRL_TakeSemEndOfOperation(void);
extern int PKACTRL_ReleaseSemEndOfOperation(void);

#ifdef __cplusplus
}
#endif

#endif /* CRC_CTRL_H */
