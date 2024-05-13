/*
 * Copyright (c) 2021-2022 Arm Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __EMULATED_FLASH_DRV_H__
#define __EMULATED_FLASH_DRV_H__

#include <stdbool.h>
#include <stdint.h>

/* This include is neceasary because of the ARM_FLASH_INFO structure. Since this driver is not used
 * as a standalone driver, only with tf-m, we can include it here to prevent code duplication.
 */
#include "Driver_Flash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EMULATED_FLASH_DRV_ERASE_VALUE 0xFF

/**
\brief Flash Sector information
*/
typedef struct _emulated_flash_sector_t
{
    uint32_t start; ///< Sector Start address
    uint32_t end;   ///< Sector End address (start+size-1)
} const emulated_flash_sector_t;

/*
 * Emulated flash device structure
 *
 * This driver just emulates a flash interface and behaviour on top of any
 * type of memory.
 */
struct emulated_flash_dev_t
{
    const uint32_t memory_base_s;  /*!< FLASH memory base address, secure alias */
    const uint32_t memory_base_ns; /*!< FLASH memory base address, non-secure alias */
    ARM_FLASH_INFO * data;         /*!< FLASH data */
};

enum emulated_flash_error_t
{
    EMULATED_FLASH_ERR_NONE = 0,      /*!< No error */
    EMULATED_FLASH_ERR_INVALID_PARAM, /*!< Invalid parameter error */
    EMULATED_FLASH_NOT_READY,         /*!< Not ready error */
};

/**
 * \brief Reads data from emulated flash in a blocking call
 *
 * \param[in]  dev       Emulated flash device struct \ref emulated_flash_dev_t
 * \param[in]  addr      Address to read data from the flash
 * \param[out] data      Pointer to store data read from the flash
 * \param[in]  cnt       Number of data items to read
 *
 * \return Returns error code as specified in \ref emulated_flash_error_t
 *
 * \note This function doesn't check if dev is NULL.
 * \note The whole region needs to be set to the same security to use this
 *       function.
 */
enum emulated_flash_error_t emulated_flash_read_data(struct emulated_flash_dev_t * dev, uint32_t addr, void * data, uint32_t cnt);

/**
 * \brief Writes data to the flash in a blocking call
 *
 * \param[in] dev      Emulated flash device struct \ref emulated_flash_dev_t
 * \param[in] addr     Address to write data to the flash
 * \param[in] data     Pointer to the data to be written
 * \param[in] cnt      Number of bytes to write
 *
 * \return Returns error code as specified in \ref emulated_flash_error_t
 *
 * \note Flash area needs to be pre-erased before writing to it
 * \note Addr is expected to be within the [0x0 - Flash size] range
 * \note For better performance, this function doesn't check if dev is NULL
 * \note The whole region needs to be set to the same security to use this
 *       function.
 */
enum emulated_flash_error_t emulated_flash_program_data(struct emulated_flash_dev_t * dev, uint32_t addr, const void * data,
                                                        uint32_t cnt);

/**
 * \brief Erases a sector of the flash
 *
 * \param[in] dev      Emulated flash device struct \ref emulated_flash_dev_t
 * \param[in] addr     Address of the sector to erase
 *
 * \return Returns error code as specified in \ref emulated_flash_error_t
 *
 * \note For better performance, this function doesn't check if dev is NULL
 * \note Addr is expected to be within the [0x0 - Flash size] range
 * \note The whole sector needs to be set to the same security to use this
 *       function.
 */
enum emulated_flash_error_t emulated_flash_erase_sector(struct emulated_flash_dev_t * dev, uint32_t addr);

/**
 * \brief Erases the whole flash
 *
 * \param[in] dev      Emulated flash device struct \ref emulated_flash_dev_t
 *
 * \note For better performance, this function doesn't check if dev is NULL
 * \note The whole memory needs to be set to the same security to use this
 *       function.
 */
void emulated_flash_erase_chip(struct emulated_flash_dev_t * dev);

/**
 * \brief Returns the information of the emulated flash
 *
 * \param[in] dev      Emulated flash device struct \ref emulated_flash_dev_t
 *
 * \return Returns the info  \ref ARM_FLASH_INFO
 *
 * \note For better performance, this function doesn't check if dev is NULL
 * \note The \ref ARM_FLASH_INFO is coming from the CMSIS Flash driver. This
 *       native driver is only used together with CMSIS, so instead of
 *       duplicating the structure, it is used here as well.
 */
ARM_FLASH_INFO * emulated_flash_get_info(struct emulated_flash_dev_t * dev);

#ifdef __cplusplus
}
#endif
#endif /* __EMULATED_FLASH_DRV_H__ */
