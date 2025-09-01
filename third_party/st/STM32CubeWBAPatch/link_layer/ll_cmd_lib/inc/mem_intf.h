/*$Id: //dwh/bluetooth/DWC_ble154combo/firmware/rel/2.00a-lca01/firmware/public_inc/mem_intf.h#1 $*/
/**
 ********************************************************************************
 * @file    mem_intf.h
 * @brief   This file contains all the functions prototypes for the mem_intf.c.
 ******************************************************************************
 * @copy
 * This Synopsys DWC Bluetooth Low Energy Combo Link Layer/MAC software and
 * associated documentation ( hereinafter the "Software") is an unsupported
 * proprietary work of Synopsys, Inc. unless otherwise expressly agreed to in
 * writing between Synopsys and you. The Software IS NOT an item of Licensed
 * Software or a Licensed Product under any End User Software License Agreement
 * or Agreement for Licensed Products with Synopsys or any supplement thereto.
 * Synopsys is a registered trademark of Synopsys, Inc. Other names included in
 * the SOFTWARE may be the trademarks of their respective owners.
 *
 * Synopsys MIT License:
 * Copyright (c) 2020-Present Synopsys, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * the Software), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING, BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE ARISING FROM,
 * OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MEM_INTF_H
#define MEM_INTF_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Defination ----------------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported types ------------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/** @addtogroup Memory_Interface_Exported_Functions
 * @{
 */

/**
 * @brief  Coping memory from position to another.
 * @param  ptr_dstntion	: pointer to the destination array where the content is to be copied.
 * @param  ptr_src	: pointer to the source of data to be copied.
 * @param  n 		: the number of bytes to be copied.
 * @retval pointer to destination.
 */
void *ble_memcpy(
	void *ptr_dstntion,
	const void *ptr_src,
	uint16_t n);

/**
 * @brief  Setting a certian block of memory with a certain value.
 * @param  ptr_mem	: pointer to the block of memory to fill.
 * @param  value	: the value to be set. The value is passed as an int.
 * @param  n 		: the number of bytes to be set to the value.
 * @retval pointer to destination.
 */
void *ble_memset(
	void *ptr_mem,
	uint8_t value,
	uint16_t n);

/**
 * @brief  comparing a certain block of memory with another.
 * @param  ptr_dstntion	: pointer to the destination array where the content is to be compared.
 * @param  ptr_src	: pointer to the source of data to be compared.
 * @param  n 		: the number of bytes to be compared.
 * @retval  	< 0 ptr_dstntion is less than ptr_src.
 * 		> 0 ptr_src is less than ptr_dstntion.
 * 		  0 ptr_dstntion is equal to ptr_src.
 */
int8_t ble_memcmp(
	const void *ptr_dstntion,
	const void *ptr_src,
	uint16_t n);

/**
 * @brief  Moving  memory from position to another.
 * @param  ptr_dstntion	: pointer to the destination array where the content is to be moved.
 * @param  ptr_src	: pointer to the source of data to be moved.
 * @param  n 		: the number of bytes to be moved.
 * @retval pointer to destination.
 */

void *ble_memmov(
	void *ptr_dstntion,
	const void *ptr_src,
	uint16_t n);
/**
 * @brief  Coping n bytes of memory from position to another.
 * @param  destination	: pointer to the destination array where the content is to be copied.
 * @param  source	    : pointer to the source of data to be copied.
 * @param  num_bytes    : the number of bytes to be copied.
 * @param  keep_endian  : flag to keep or change the endian
 * @retval pointer to destination.
 */
void ble_memcpy_n_bytes(uint8_t *destination,
		const uint8_t *source, uint8_t num_bytes, uint8_t keep_endian);
/**
 * @}
 */
#endif /* MEM_INTF_H */
