/**
  ******************************************************************************
  * @file    storage_interface.h
  * @author  MCD Application Team
  * @brief   Header for storage_interface.c module
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
#ifndef STORAGE_INTERFACE_H
#define STORAGE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx.h"
#include "psa/crypto.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define ITS_ENCRYPTION_SECRET_KEY_ID  ((psa_key_id_t)0x2FFFAAAA)
/* Device Attestation PSA key ID */
#define DEVICE_ATTESTATION_PRIVATE_KEY_ID_USER   ((psa_key_id_t)0x1fff0001)



/* Exported functions ------------------------------------------------------- */
psa_status_t storage_set(uint64_t obj_uid,
                         uint32_t obj_length,
                         const void *p_obj);

psa_status_t storage_get(uint64_t obj_uid,
                         uint32_t obj_offset,
                         uint32_t obj_length,
                         void *p_obj);

psa_status_t storage_get_info(uint64_t obj_uid,
                              void *p_obj_info,
                              uint32_t obj_info_size);

psa_status_t storage_remove(uint64_t obj_uid, uint32_t obj_size);




#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* STORAGE_INTERFACE_H */

