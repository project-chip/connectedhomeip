/**
*********************************************************************************************************
*               Copyright(c) 2023, Realtek Semiconductor Corporation. All rights reserved.
**********************************************************************************************************
* \file     matter_ota.h
* \brief    This file provides matter ota functions.
* \details
* \author   rock ding
* \date     2024-04-18
* \version  v1.0
*********************************************************************************************************
*/

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef MATTER_OTA_H_
#define MATTER_OTA_H_

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** \defgroup Matter_OTA_Exported_Functions Matter OTA Exported Functions
  * \brief
  * \{
  */

/**
  * @brief  Prepare for an OTA image download.
  *
  * @retval 0     On success.
  * @retval other On failure.
  */
uint8_t rtk_matter_ota_prepare(void);

/**
  * @brief  Complete the OTA image download process.
  *
  * @retval 0     On success.
  * @retval other On failure.
  */
uint8_t rtk_matter_ota_finalize(void);

/**
  * @brief  Apply the OTA image.
  */
void rtk_matter_ota_apply(void);

/**
  * @brief  Abort the OTA image download process.
  *
  * @retval 0     On success.
  * @retval other On failure.
  */
uint8_t rtk_matter_ota_abort(void);

/**
  * @brief  Process a downloaded block of data.
  * @param[in]  p_data  Pointer to the image block data.
  * @param[in]  len     Length of the data.
  *
  * @retval 0     On success.
  * @retval other On failure.
  */
uint8_t rtk_matter_ota_process_block(uint8_t *p_data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /*MATTER_OTA_H*/


/******************* (C) COPYRIGHT 2023 Realtek Semiconductor Corporation *****END OF FILE****/

