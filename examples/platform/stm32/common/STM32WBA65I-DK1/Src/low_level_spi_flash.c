/*
 * Copyright (c) 2013-2018 Arm Limited
 * Copyright (c) 2024 STMicroelectronics
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

#include "Driver_Flash.h"
#include "low_level_spi_flash.h"
#include <string.h>
//#include "stm32_hal.h"
#include "appli_flash_layout.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

#if defined (OEMIROT_EXTERNAL_FLASH_ENABLE)
#include "stm32wba65i_discovery_conf.h"

char StatusRegRxBuffer[1] = {0xFF};

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  ((void)arg)
#endif /* ARG_UNUSED */

/* config for flash driver */
#define SPI_FLASH0_TOTAL_SIZE   SPI_FLASH_TOTAL_SIZE
#define SPI_FLASH0_SECTOR_SIZE  (uint32_t)(4  * 1024) /* Must be same as internal flash sector size */
#define SPI_FLASH0_BLOCK_SIZE   (uint32_t)(64  * 1024) /* Must be same as internal flash sector size */
#define SPI_FLASH0_PAGE_SIZE    (uint32_t)512
#define SPI_FLASH0_PROG_UNIT    0x2 /* 2 bytes for SPI DTR mode */
#define SPI_FLASH0_ERASED_VAL   0xff

/*
#define DEBUG_SPI_FLASH_ACCESS
*/
/* Driver version */
#define ARM_SPI_FLASH_DRV_VERSION   ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

static const ARM_DRIVER_VERSION DriverVersion =
{
  ARM_FLASH_API_VERSION,  /* Defined in the CMSIS Flash Driver header file */
  ARM_SPI_FLASH_DRV_VERSION
};

/**
  * \brief Flash driver capability macro definitions \ref ARM_FLASH_CAPABILITIES
  */
/* Flash Ready event generation capability values */
#define EVENT_READY_NOT_AVAILABLE   (0u)
#define EVENT_READY_AVAILABLE       (1u)
/* Data access size values */
#define DATA_WIDTH_8BIT             (0u)
#define DATA_WIDTH_16BIT            (1u)
#define DATA_WIDTH_32BIT            (2u)
/* Chip erase capability values */
#define CHIP_ERASE_NOT_SUPPORTED    (0u)
#define CHIP_ERASE_SUPPORTED        (1u)


static ReadyStatus Spi_WriteEnable(uint32_t Instance);
static ReadyStatus Spi_WaitForDeviceReady(uint32_t Instance);
/* Driver Capabilities */
static const ARM_FLASH_CAPABILITIES DriverCapabilities =
{
  EVENT_READY_NOT_AVAILABLE,
  DATA_WIDTH_16BIT, /* 16bits for DTR mode */
  CHIP_ERASE_SUPPORTED
};

/**
  * \brief Flash status macro definitions \ref ARM_FLASH_STATUS
  */
/* Busy status values of the Flash driver  */
#define DRIVER_STATUS_IDLE      (0u)
#define DRIVER_STATUS_BUSY      (1u)
/* Error status values of the Flash driver */
#define DRIVER_STATUS_NO_ERROR  (0u)
#define DRIVER_STATUS_ERROR     (1u)

/**
  * \brief Arm Flash device structure.
  */
struct arm_spi_flash_dev_t
{
  struct low_level_spi_device *dev;
  ARM_FLASH_INFO *data;       /*!< SPI FLASH memory device data */
};
/**
  * @}
  */

/**
  * \brief      Check if the Flash memory boundaries are not violated.
  * \param[in]  flash_dev  Flash device structure \ref arm_spi_flash_dev_t
  * \param[in]  offset     Highest Flash memory address which would be accessed.
  * \return     Returns true if Flash memory boundaries are not violated, false
  *             otherwise.
  */

static bool is_range_valid(struct arm_spi_flash_dev_t *flash_dev,
                           uint32_t offset)
{
  uint32_t flash_limit = 0;

  /* Calculating the highest address of the Flash memory address range */
  flash_limit = SPI_FLASH_TOTAL_SIZE - 1;

  return (offset > flash_limit) ? (false) : (true) ;
}
/**
  * \brief        Check if the parameter is an erasable page.
  * \param[in]    flash_dev  Flash device structure \ref arm_spi_flash_dev_t
  * \param[in]    param      Any number that can be checked against the
  *                          program_unit, e.g. Flash memory address or
  *                          data length in bytes.
  * \return       Returns true if param is a sector eraseable, false
  *               otherwise.
  */
static bool is_erase_allow(struct arm_spi_flash_dev_t *flash_dev,
                           uint32_t param)
{
  /*  allow erase in range provided by device info */
  struct spi_flash_vect *vect = &flash_dev->dev->erase;
  uint32_t nb;
  for (nb = 0; nb < vect->nb; nb++)
    if ((param >= vect->range[nb].base) && (param <= vect->range[nb].limit))
    {
      return true;
    }
  return false;
}
/**
  * \brief        Check if the parameter is writeable area.
  * \param[in]    flash_dev  Flash device structure \ref arm_spi_flash_dev_t
  * \param[in]    param      Any number that can be checked against the
  *                          program_unit, e.g. Flash memory address or
  *                          data length in bytes.
  * \return       Returns true if param is aligned to program_unit, false
  *               otherwise.
  */

static bool is_write_allow(struct arm_spi_flash_dev_t *flash_dev,
                           uint32_t start, uint32_t len)
{
  /*  allow write access in area provided by device info */
  struct spi_flash_vect *vect = &flash_dev->dev->write;
  uint32_t nb;
  for (nb = 0; nb < vect->nb; nb++)
    if ((start >= vect->range[nb].base) && ((start + len - 1) <= vect->range[nb].limit))
    {
      return true;
    }
  return false;
}

/**
  * \brief        Check if the parameter is aligned to program_unit.
  * \param[in]    flash_dev  Flash device structure \ref arm_spi_flash_dev_t
  * \param[in]    param      Any number that can be checked against the
  *                          program_unit, e.g. Flash memory address or
  *                          data length in bytes.
  * \return       Returns true if param is aligned to program_unit, false
  *               otherwise.
  */

static bool is_write_aligned(struct arm_spi_flash_dev_t *flash_dev,
                             uint32_t param)
{
  return ((param % flash_dev->data->program_unit) != 0) ? (false) : (true);
}
/**
  * \brief        Check if the parameter is aligned to page_unit.
  * \param[in]    flash_dev  Flash device structure \ref arm_spi_flash_dev_t
  * \param[in]    param      Any number that can be checked against the
  *                          program_unit, e.g. Flash memory address or
  *                          data length in bytes.
  * \return       Returns true if param is aligned to sector_unit, false
  *               otherwise.
  */
static bool is_erase_aligned(struct arm_spi_flash_dev_t *flash_dev,
                             uint32_t param)
{
  return ((param % (flash_dev->data->sector_size)) != 0) ? (false) : (true);
}


static ARM_FLASH_INFO ARM_SPI_FLASH0_DEV_DATA =
{
  .sector_info    = NULL,     /* Uniform sector layout */
  .sector_count   = SPI_FLASH0_TOTAL_SIZE / SPI_FLASH0_SECTOR_SIZE,
  .sector_size    = SPI_FLASH0_SECTOR_SIZE,
  .page_size      = SPI_FLASH0_PAGE_SIZE,
  .program_unit   = SPI_FLASH0_PROG_UNIT, /* Minimum write size in bytes */
  .erased_value   = SPI_FLASH0_ERASED_VAL
};

static struct arm_spi_flash_dev_t ARM_SPI_FLASH0_DEV =
{
  .dev    = &(SPI_FLASH0_DEV),
  .data   = &(ARM_SPI_FLASH0_DEV_DATA)
};

/* Flash Status */
static ARM_FLASH_STATUS ARM_SPI_FLASH0_STATUS = {0, 0, 0};

static ARM_DRIVER_VERSION Spi_Flash_GetVersion(void)
{
  return DriverVersion;
}

static ARM_FLASH_CAPABILITIES Spi_Flash_GetCapabilities(void)
{
  return DriverCapabilities;
}

static int32_t Spi_Flash_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
  ARG_UNUSED(cb_event);

  if (BSP_EEPROM_Init(BSP_EEPROM_0) != M95_OK)
  {
    return ARM_DRIVER_ERROR_SPECIFIC;
  }
  else
  {
    return ARM_DRIVER_OK;
  }
}

static int32_t Spi_Flash_Uninitialize(void)
{
  /* Still waiting BSP function definition*/
  return ARM_DRIVER_OK;
}

static int32_t Spi_Flash_PowerControl(ARM_POWER_STATE state)
{
  switch (state)
  {
    case ARM_POWER_FULL:
      /* Nothing to be done */
      return ARM_DRIVER_OK;
    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
    default:
      return ARM_DRIVER_ERROR_PARAMETER;
  }
}

static int32_t Spi_Flash_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
  int32_t err = BSP_ERROR_NONE;
  uint8_t data_tmp[2];

  ARM_SPI_FLASH0_STATUS.error = DRIVER_STATUS_NO_ERROR;

#ifdef DEBUG_SPI_FLASH_ACCESS
  printf("read spi 0x%x n=%x \r\n", (addr + SPI_FLASH_BASE_ADDRESS), cnt);
#endif /*  DEBUG_SPI_FLASH_ACCESS */

  /* Check Flash memory boundaries */
  if (!is_range_valid(&ARM_SPI_FLASH0_DEV, addr + cnt - 1))
  {
#ifdef DEBUG_SPI_FLASH_ACCESS
    printf("read spi not allowed 0x%x n=%x \r\n", (addr + SPI_FLASH_BASE_ADDRESS), cnt);
#endif
    ARM_SPI_FLASH0_STATUS.error = DRIVER_STATUS_ERROR;
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  ARM_SPI_FLASH0_STATUS.busy = DRIVER_STATUS_BUSY;

  /* ensure previous operation is finished (erase/write) : GetStatus()
     such verification is done (inside BSP driver) at the beginning of erase or write operation  but
     not for read operation ==> in order to maximise BSP driver execution timing efficiency */
  /*while (BSP_SPI_NOR_GetStatus(0) !=  BSP_ERROR_NONE)
  {
  }*/

  /* SPI DTR mode constraint: split read request to ensure read at
   * even address with even number of bytes.
   * Flash address to read is the offset from begin of external flash.
   */
  if (addr % 2)
  {
    err = BSP_EEPROM_ReadBuffer(0, (uint8_t *)data_tmp, addr - 1, 2);
    *(uint8_t*)data = data_tmp[1];

    if (cnt > 1)
    {
      if (cnt % 2)
      {
        if (err == BSP_ERROR_NONE)
        {
          err = BSP_EEPROM_ReadBuffer(0, (uint8_t *)data + 1, (addr + 1), cnt - 1);
        }
      }
      else
      {
        if (err == BSP_ERROR_NONE)
        {
          err = BSP_EEPROM_ReadBuffer(0, (uint8_t *)data + 1, (addr + 1), cnt - 2);
        }

        if (err == BSP_ERROR_NONE)
        {
          err = BSP_EEPROM_ReadBuffer(0, (uint8_t *)data_tmp, addr + cnt - 1, 2);
          *((uint8_t*)data + cnt - 1) = data_tmp[0];
        }
      }
    }
  }
  else
  {
    if (cnt % 2)
    {
      if (cnt > 1)
      {
        err = BSP_EEPROM_ReadBuffer(0, (uint8_t *)data, addr, cnt - 1);
      }

      if (err == BSP_ERROR_NONE)
      {
        err = BSP_EEPROM_ReadBuffer(0, (uint8_t *)data_tmp, addr + cnt - 1, 2);
        *((uint8_t*)data + cnt -1) = data_tmp[0];
      }
    }
    else
    {
      err = BSP_EEPROM_ReadBuffer(0, (uint8_t *)data, addr, cnt);
    }
  }

  ARM_SPI_FLASH0_STATUS.busy = DRIVER_STATUS_IDLE;

  if (err != BSP_ERROR_NONE)
  {
#ifdef DEBUG_SPI_FLASH_ACCESS
    printf("failed read spi 0x%x n=%x \r\n", (addr + SPI_FLASH_BASE_ADDRESS), cnt);
#endif /* DEBUG_SPI_FLASH_ACCESS */
    return ARM_DRIVER_ERROR;
  }

  return ARM_DRIVER_OK;
}

static int32_t Spi_Flash_ProgramData(uint32_t addr,
                                      const void *data, uint32_t cnt)
{
  int32_t err = BSP_ERROR_NONE;

  ARM_SPI_FLASH0_STATUS.error = DRIVER_STATUS_NO_ERROR;

#ifdef DEBUG_SPI_FLASH_ACCESS
  printf("write spi 0x%x n=%x \r\n", (addr + SPI_FLASH_BASE_ADDRESS), cnt);
#endif /* DEBUG_SPI_FLASH_ACCESS */
  /* Check Flash memory boundaries and alignment with minimum write size
   * (program_unit), data size also needs to be a multiple of program_unit.
   */
  if ((!is_range_valid(&ARM_SPI_FLASH0_DEV, addr + cnt - 1)) ||
      (!is_write_aligned(&ARM_SPI_FLASH0_DEV, addr))     ||
      (!is_write_aligned(&ARM_SPI_FLASH0_DEV, cnt))      ||
      (!is_write_allow(&ARM_SPI_FLASH0_DEV, addr, cnt))
     )
  {
#ifdef DEBUG_SPI_FLASH_ACCESS
    printf("write spi not allowed 0x%x n=%x \r\n", (addr + SPI_FLASH_BASE_ADDRESS), cnt);
#endif
    ARM_SPI_FLASH0_STATUS.error = DRIVER_STATUS_ERROR;
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  ARM_SPI_FLASH0_STATUS.busy = DRIVER_STATUS_BUSY;
  
  /* spi flash address to write is the offset from begin of external flash */
  if (Spi_WriteEnable(BSP_EEPROM_0) == READY)
  {
    Spi_WaitForDeviceReady(BSP_EEPROM_0);

    err = BSP_EEPROM_WriteBuffer(0, (uint8_t *) data, addr, cnt);
  }
  else
  {
    printf("ERROR: Write enable bit has not been set");
  }

  ARM_SPI_FLASH0_STATUS.busy = DRIVER_STATUS_IDLE;

  if (err != BSP_ERROR_NONE)
  {
    printf("->>> Failed to write 0x%" PRIx32 " bytes at 0x%" PRIx32 " (Error 0x%" PRIx32 ")", cnt, (uint32_t)(SPI_FLASH_BASE_ADDRESS + addr), err);
#ifdef DEBUG_SPI_FLASH_ACCESS
    printf("failed write spi 0x%x n=%x \r\n", (addr + SPI_FLASH_BASE_ADDRESS), cnt);
#endif /* DEBUG_SPI_FLASH_ACCESS */
    return ARM_DRIVER_ERROR;
  }
  return ARM_DRIVER_OK;
}

static int32_t Spi_Flash_EraseSector(uint32_t addr)
{
  int32_t err;

  ARM_SPI_FLASH0_STATUS.error = DRIVER_STATUS_NO_ERROR;

#ifdef DEBUG_SPI_FLASH_ACCESS
  printf("erase spi 0x%x\r\n", (addr + SPI_FLASH_BASE_ADDRESS));
#endif /* DEBUG_SPI_FLASH_ACCESS */
  if (!(is_range_valid(&ARM_SPI_FLASH0_DEV, addr)) ||
      !(is_erase_aligned(&ARM_SPI_FLASH0_DEV, addr)) ||
      !(is_erase_allow(&ARM_SPI_FLASH0_DEV, addr)))
  {
#ifdef DEBUG_SPI_FLASH_ACCESS
    printf("erase spi not allowed 0x%x\r\n", (addr + SPI_FLASH_BASE_ADDRESS));
#endif
    ARM_SPI_FLASH0_STATUS.error = DRIVER_STATUS_ERROR;
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  ARM_SPI_FLASH0_STATUS.busy = DRIVER_STATUS_BUSY;

  if (BSP_EEPROM_WriteEnable(BSP_EEPROM_0) == M95_OK)
  {
    if (ARM_SPI_FLASH0_DEV.data->sector_size == SPI_FLASH0_SECTOR_SIZE)
    {
      err = BSP_EEPROM_EraseSector(0, addr);
    }
    else
    {
      err = BSP_ERROR_WRONG_PARAM;
    }
  }
  else
  {
    err = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  ARM_SPI_FLASH0_STATUS.busy = DRIVER_STATUS_IDLE;

  if (err != BSP_ERROR_NONE)
  {
    printf("->>> Erase failed at 0x%" PRIx32 " (Error 0x%" PRIx32 ")", (uint32_t)(SPI_FLASH_BASE_ADDRESS + addr), err);
#ifdef DEBUG_SPI_FLASH_ACCESS
    printf("erase spi failed 0x%x\r\n", (addr + SPI_FLASH_BASE_ADDRESS));
#endif
    return ARM_DRIVER_ERROR;
  }
  return ARM_DRIVER_OK;
}

static int32_t Spi_Flash_EraseChip(void)
{
  /* Write enable is required before erasing chip */
  if (BSP_EEPROM_WriteEnable(BSP_EEPROM_0) != M95_OK)
    return ARM_DRIVER_ERROR;

  if (BSP_EEPROM_EraseChip(BSP_EEPROM_0) != BSP_ERROR_NONE)
    return ARM_DRIVER_ERROR;
  return ARM_DRIVER_OK;
}

static ARM_FLASH_STATUS Spi_Flash_GetStatus(void)
{
  return ARM_SPI_FLASH0_STATUS;
}

static ARM_FLASH_INFO *Spi_Flash_GetInfo(void)
{
  return ARM_SPI_FLASH0_DEV.data;
}

static ReadyStatus Spi_WriteEnable(uint32_t Instance)
{
  StatusRegRxBuffer[0] = 0x00U;
  if (BSP_EEPROM_WriteEnable(Instance) == M95_OK)
    {
      /* Read status register in EEPROM */
      if (BSP_EEPROM_StatusRegRead(Instance, (uint8_t*)StatusRegRxBuffer) == M95_OK)
      {
        if ((StatusRegRxBuffer[0] & WEL_Msk) == WEL_Msk)
        {
         return(READY);
        }
        else
        {
          printf("Error occurred while reading status register\r\n");
        }
      }
    }
  return(NOT_READY);
}

static ReadyStatus Spi_WaitForDeviceReady(uint32_t Instance)
{
  StatusRegRxBuffer[0] = 0xFFU;

  /* Initialize number of trials to read a ready status */
  /* trials_ nb should depend on clock frequency, the actual one should not be high enough if clock frequency increases */
  uint32_t count = 1000UL;

  while(1)
  {
    /* Read status register in EEPROM */
    if (BSP_EEPROM_StatusRegRead(BSP_EEPROM_0, (uint8_t*)StatusRegRxBuffer) == M95_OK)
    {
      if (((StatusRegRxBuffer[0] & WIP_Msk) == 0x0U))
      {
        return(READY);
      }
    }
    else
    {
      printf(" wait Error occurred while reading status register\r\n");
      return(NOT_READY);
    }

    if (count == 0U)
    {
      printf("Timeout occurred while checking status register\r\n");
      return(NOT_READY);
    }
    count--;
  }
}

ARM_DRIVER_FLASH Driver_SPI_FLASH0 =
{
  Spi_Flash_GetVersion,
  Spi_Flash_GetCapabilities,
  Spi_Flash_Initialize,
  Spi_Flash_Uninitialize,
  Spi_Flash_PowerControl,
  Spi_Flash_ReadData,
  Spi_Flash_ProgramData,
  Spi_Flash_EraseSector,
  Spi_Flash_EraseChip,
  Spi_Flash_GetStatus,
  Spi_Flash_GetInfo
};

/**
  * @brief This function configures the spi flash in execution mode.
  * @note
  * @retval execution_status
  */
int32_t Spi_Flash_Config_Exe(void)
{
#ifdef DEBUG_SPI_FLASH_ACCESS
  printf("memory mapped spi\r\n");
#endif /*  DEBUG_SPI_FLASH_ACCESS */

  ARM_SPI_FLASH0_STATUS.busy = DRIVER_STATUS_BUSY;

  /* Still waiting BSP function definition*/

  return ARM_DRIVER_OK;
}
#endif /* OEMIROT_EXTERNAL_FLASH_ENABLE */
