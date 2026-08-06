/**
 ******************************************************************************
 * @file    secure_nsc.h
 * @author  MCD Application Team
 * @brief   Header for secure non-secure callable APIs list
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
#ifndef SECURE_NSC_H
#define SECURE_NSC_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/**
 * @brief  non-secure callback ID enumeration definition
 */
typedef enum
{
    SECURE_FAULT_CB_ID = 0x00U, /*!< System secure fault callback ID */
    GTZC_ERROR_CB_ID   = 0x01U  /*!< GTZC secure error callback ID */
} SECURE_CallbackIDTypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SECURE_RegisterCallback(SECURE_CallbackIDTypeDef CallbackId, void * func);
void SECURE_ConfirmSecureAppImage(void);
void SECURE_ConfirmSecureDataImage(void);
#endif /* SECURE_NSC_H */
