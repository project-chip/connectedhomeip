/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
/**
 ******************************************************************************
 * @file    storage_interface.h
 * @author  MCD Application Team
 * @brief   Header for storage_interface.c module
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STORAGE_INTERFACE_H
#define STORAGE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "psa/crypto.h"
#include "stm32wbaxx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define ITS_ENCRYPTION_SECRET_KEY_ID ((psa_key_id_t) 0x2FFFAAAA)
/* Device Attestation PSA key ID */
#define DEVICE_ATTESTATION_PRIVATE_KEY_ID_USER ((psa_key_id_t) 0x1fff0001)

/* Exported functions ------------------------------------------------------- */
psa_status_t storage_set(uint64_t obj_uid, uint32_t obj_length, const void * p_obj);

psa_status_t storage_get(uint64_t obj_uid, uint32_t obj_offset, uint32_t obj_length, void * p_obj);

psa_status_t storage_get_info(uint64_t obj_uid, void * p_obj_info, uint32_t obj_info_size);

psa_status_t storage_remove(uint64_t obj_uid, uint32_t obj_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STORAGE_INTERFACE_H */
