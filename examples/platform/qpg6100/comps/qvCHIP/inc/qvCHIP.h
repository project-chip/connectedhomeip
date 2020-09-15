/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * Copyright (c) 2020, Qorvo Inc
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 *
 * $Change: 155597 $
 * $DateTime: 2020/09/09 13:12:11 $
 */

/** @file "qvCHIP.h"
 *
 *  CHIP wrapper API
 *
 *  Declarations of the public functions and enumerations of qvCHIP.
*/

#ifndef _QVCHIP_H_
#define _QVCHIP_H_

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************
 *                    Enum Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Functional Macro Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Type Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Public Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//Requests
/** @brief Initialize Qorvo needed components for CHIP.
*   @return result                   0 if init was succesfull. -1 when failed
*/
int qvCHIP_init(void);

/** @brief Printf that outputs on Qorvo platforms.
*
*   @param module                    Module id for module producing the logging.
*   @param formattedMsg              Char buffer with formatted string message.
*/
void qvCHIP_Printf(uint8_t module, const char* formattedMsg);

/** @brief Initialization of NVM memory used for CHIP
*/
void qvCHIP_Nvm_Init();

/** @brief Get maximum length of data identified by keyId
 *
 *  @param key NVM identifier to fetch maximum length for.
 *  @param maxLength Maximum length for given key.
*/
uint16_t qvCHIP_Nvm_GetMaxKeyLen(uint16_t key);

/** @brief Backup data to NVM for a given key.
 *
 *  @param key             Identifier for NVM area to backup.
 *  @param pRamLocation    Pointer to data to backup.
 *  @param length          Length of data to backup. Cannot exceed maximum length for the key.
*/
void qvCHIP_Nvm_Backup(uint16_t key, uint8_t* pRamLocation, uint16_t length);

/** @brief Restore data from NVM for a given key.
 *
 *  @param key            Identifier for NVM area to restore.
 *  @param pRamLocation   Pointer to the RAM location to restore data to.
 *  @param length         Length of data to restore. Cannot exceed maximum length for the key.
                          If smaller then the maximum length a partial restore will be executed.
*/
bool qvCHIP_Nvm_Restore(uint16_t key, uint8_t* pRamLocation, uint16_t* length);

/** @brief Remove data from NVM for a given key.
 *
 *  @param key             Identifier for NVM data to remove.
*/
void qvCHIP_Nvm_ClearValue(uint16_t key);

/** @brief Returns if data for a given key exists in NVM.
 *
 *  @param key             Identifier for NVM data to check for existence.
 *  @param length          Returns length of the data stored for the key.
 *  @return exists 		   Returns true if data exists for given key.
*/
bool qvCHIP_Nvm_ValueExists(uint16_t key, uint16_t* length);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_QVCHIP_H_
