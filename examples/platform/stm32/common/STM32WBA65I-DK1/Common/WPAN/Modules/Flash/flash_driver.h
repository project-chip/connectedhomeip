/**
  ******************************************************************************
  * @file    flash_driver.h
  * @author  MCD Application Team
  * @brief   Header for flash_driver.c module
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
#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

/* Exported types ------------------------------------------------------------*/

/* Bit mask to modify Flash Control status */
typedef uint32_t  FD_Flash_ctrl_bm_t;

/* Flash operation status */
typedef enum
{
  FD_FLASHOP_SUCCESS,
  FD_FLASHOP_FAILURE
} FD_FlashOp_Status_t;

/* Flash Driver commands to enable or disable flash access */
typedef enum
{
  LL_FLASH_ENABLE,
  LL_FLASH_DISABLE,
} FD_FLASH_Status_t;

/**
 * @brief Bit mask to modify Flash Control status
 *
 * @details Those bitmasks are used to enable/disable access to the flash:
 *   - System:
 *     -# FD_FLASHACCESS_SYSTEM: Determine whether or not the flash access is allowed from a system POV.
 *                               This bit has a predominance over all the other bit masks, ie: No flash operation can
 *                               be achieved without this to be set to 0, ie: LL_FLASH_ENABLE.
 *   - RFTS:
 *     -# FD_FLASHACCESS_RFTS: Determine whether or not the RF Timing Synchro allows flash access. This bit is set
 *                             once a window has been allowed by the BLE LL, ie: set to 0.
 *                             This bit has no impact when FD_FLASHACCESS_RFTS_BYPASS is set, ie: set to 0.
 *     -# FD_FLASHACCESS_RFTS_BYPASS: Nullify the impact of FD_FLASHACCESS_RFTS when enabled, ie: set to 0. Its role is
 *                                    to allow flash operation without the need to request a timing window to the RFTS,
 *                                    ie: Executing flash operation without the BLE LL.
 *
 */
typedef enum FD_FlashAccess_bm
{
  /* System flash access bitfield */
  FD_FLASHACCESS_SYSTEM,
  /* RF Timing Synchro flash access bitfield */
  FD_FLASHACCESS_RFTS,
  /* Bypass of RF Timing Synchro flash access bitfield */
  FD_FLASHACCESS_RFTS_BYPASS,
}FD_FlashAccess_bm_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void FD_SetStatus(FD_Flash_ctrl_bm_t Flags_bm, FD_FLASH_Status_t Status);
FD_FlashOp_Status_t FD_WriteData(uint32_t Dest, uint32_t Payload);
FD_FlashOp_Status_t FD_EraseSectors(uint32_t Sect);

#ifdef __cplusplus
}
#endif

#endif /*FLASH_DRIVER_H */
