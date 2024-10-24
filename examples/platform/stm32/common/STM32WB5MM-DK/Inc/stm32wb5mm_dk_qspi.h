/**
 ******************************************************************************
 * @file    stm32wb5mm_dk_qspi.h
 * @author  MCD Application Team
 * @brief   This file contains the common defines and functions prototypes for
 *          the stm32wb5mm_dk_qspi.c driver.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WB5MM_DK_QSPI_H
#define STM32WB5MM_DK_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "../Components/s25fl128s/s25fl128s.h"
#include "stm32wb5mm_dk_conf.h"
#include "stm32wb5mm_dk_errno.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup STM32WB5MM_DK
 * @{
 */

/** @addtogroup STM32WB5MM_DK_QSPI
 * @{
 */
/* Exported types ------------------------------------------------------------*/
/** @defgroup STM32WB5MM_DK_QSPI_Exported_Types Exported Types
 * @{
 */
#define BSP_QSPI_Info_t S25FL128S_Info_t
#define BSP_QSPI_Interface_t S25FL128S_Interface_t
#define BSP_QSPI_Transfer_t S25FL128S_Transfer_t
#define BSP_QSPI_DualFlash_t S25FL128S_DualFlash_t
#define BSP_QSPI_Erase_t S25FL128S_Erase_t

typedef enum
{
    QSPI_ACCESS_NONE = 0, /*!<  Instance not initialized,             */
    QSPI_ACCESS_INDIRECT, /*!<  Instance use indirect mode access     */
    QSPI_ACCESS_MMP       /*!<  Instance use Memory Mapped Mode read  */
} BSP_QSPI_Access_t;

typedef struct
{
    BSP_QSPI_Access_t IsInitialized;    /*!<  Instance access Flash method     */
    BSP_QSPI_Interface_t InterfaceMode; /*!<  Flash Interface mode of Instance */
    BSP_QSPI_Transfer_t TransferRate;   /*!<  Flash Transfer mode of Instance  */
    uint32_t DualFlashMode;             /*!<  Flash dual mode                  */
    uint32_t IsMspCallbacksValid;
} BSP_QSPI_Ctx_t;

typedef struct
{
    BSP_QSPI_Interface_t InterfaceMode; /*!<  Current Flash Interface mode */
    BSP_QSPI_Transfer_t TransferRate;   /*!<  Current Flash Transfer mode  */
    BSP_QSPI_DualFlash_t DualFlashMode; /*!<  Dual Flash mode              */
} BSP_QSPI_Init_t;

typedef struct
{
    uint32_t FlashSize;
    uint32_t ClockPrescaler;
    uint32_t SampleShifting;
    uint32_t DualFlashMode;
} MX_QSPI_Init_t;
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
typedef struct
{
    void (*pMspInitCb)(pQSPI_CallbackTypeDef);
    void (*pMspDeInitCb)(pQSPI_CallbackTypeDef);
} BSP_QSPI_Cb_t;
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */

/**
 * @}
 */

/* Exported constants --------------------------------------------------------*/
/** @defgroup STM32WB5MM_DK_QSPI_Exported_Constants Exported Constants
 * @{
 */
/* QSPI instances number */
#define QSPI_INSTANCES_NUMBER 1U

/* Definition for QSPI modes */
#define BSP_QSPI_SPI_MODE (BSP_QSPI_Interface_t) S25FL128S_SPI_MODE           /* 1 Cmd Line, 1 Address Line and 1 Data Line    */
#define BSP_QSPI_SPI_1I2O_MODE (BSP_QSPI_Interface_t) S25FL128S_SPI_1I2O_MODE /* 1 Cmd Line, 1 Address Line and 2 Data Lines   */
#define BSP_QSPI_SPI_2IO_MODE (BSP_QSPI_Interface_t) S25FL128S_SPI_2IO_MODE   /* 1 Cmd Line, 2 Address Lines and 2 Data Lines  */
#define BSP_QSPI_SPI_1I4O_MODE (BSP_QSPI_Interface_t) S25FL128S_SPI_1I4O_MODE /* 1 Cmd Line, 1 Address Line and 4 Data Lines   */
#define BSP_QSPI_SPI_4IO_MODE (BSP_QSPI_Interface_t) S25FL128S_SPI_4IO_MODE   /* 1 Cmd Line, 4 Address Lines and 4 Data Lines  */
#define BSP_QSPI_QPI_MODE (BSP_QSPI_Interface_t) S25FL128S_QPI_MODE           /* 4 Cmd Lines, 4 Address Lines and 4 Data Lines */

/* Definition for QSPI transfer rates */
#define BSP_QSPI_STR_TRANSFER (BSP_QSPI_Transfer_t) S25FL128S_STR_TRANSFER /* Single Transfer Rate */

/* Definition for QSPI dual flash mode */
#define BSP_QSPI_DUALFLASH_DISABLE (BSP_QSPI_DualFlash_t) S25FL128S_DUALFLASH_DISABLE /* Single flash mode */

/* QSPI erase types */
#define BSP_QSPI_ERASE_4K (BSP_QSPI_Erase_t) S25FL128S_ERASE_4K
#define BSP_QSPI_ERASE_64K (BSP_QSPI_Erase_t) S25FL128S_ERASE_64K
#define BSP_QSPI_ERASE_CHIP (BSP_QSPI_Erase_t) S25FL128S_ERASE_CHIP

/* QSPI block sizes */
#define BSP_QSPI_BLOCK_4K S25FL128S_SECTOR_4K
#define BSP_QSPI_BLOCK_64K S25FL128S_BLOCK_64K

/* Definition for QSPI clock resources */
#define QSPI_CLK_ENABLE() __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE() __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CS_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define QSPI_D0_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_D1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_D2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_D3_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()

#define QSPI_FORCE_RESET() __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET() __HAL_RCC_QSPI_RELEASE_RESET()

/* Definition for QSPI Pins */
#define QSPI_CS_PIN GPIO_PIN_3
#define QSPI_CS_GPIO_PORT GPIOD
#define QSPI_CLK_PIN GPIO_PIN_3
#define QSPI_CLK_GPIO_PORT GPIOA
#define QSPI_D0_PIN GPIO_PIN_9
#define QSPI_D0_GPIO_PORT GPIOB
#define QSPI_D1_PIN GPIO_PIN_5
#define QSPI_D1_GPIO_PORT GPIOD
#define QSPI_D2_PIN GPIO_PIN_6
#define QSPI_D2_GPIO_PORT GPIOD
#define QSPI_D3_PIN GPIO_PIN_7
#define QSPI_D3_GPIO_PORT GPIOD

/* S25FL128S memory */
/* Size of the flash */
#define QSPI_FLASH_SIZE 26
#define QSPI_PAGE_SIZE 256

/**
 * @}
 */

/** @addtogroup STM32WB5MM_DK_QSPI_Exported_Variables
 * @{
 */
extern QSPI_HandleTypeDef hqspi;
extern BSP_QSPI_Ctx_t QSPI_Ctx[QSPI_INSTANCES_NUMBER];
/**
 * @}
 */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup STM32WB5MM_DK_QSPI_Exported_Functions
 * @{
 */
int32_t BSP_QSPI_Init(uint32_t Instance, BSP_QSPI_Init_t * Init);
int32_t BSP_QSPI_DeInit(uint32_t Instance);
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
int32_t BSP_QSPI_RegisterMspCallbacks(uint32_t Instance, BSP_QSPI_Cb_t * CallBacks);
int32_t BSP_QSPI_RegisterDefaultMspCallbacks(uint32_t Instance);
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */
int32_t BSP_QSPI_Read(uint32_t Instance, uint8_t * pData, uint32_t ReadAddr, uint32_t Size);
int32_t BSP_QSPI_Write(uint32_t Instance, uint8_t * pData, uint32_t WriteAddr, uint32_t Size);
int32_t BSP_QSPI_EraseBlock(uint32_t Instance, uint32_t BlockAddress, BSP_QSPI_Erase_t BlockSize);
int32_t BSP_QSPI_EraseChip(uint32_t Instance);
int32_t BSP_QSPI_GetStatus(uint32_t Instance);
int32_t BSP_QSPI_GetInfo(uint32_t Instance, BSP_QSPI_Info_t * pInfo);
int32_t BSP_QSPI_EnableMemoryMappedMode(uint32_t Instance);
int32_t BSP_QSPI_DisableMemoryMappedMode(uint32_t Instance);
int32_t BSP_QSPI_ReadID(uint32_t Instance, uint8_t * Id);

/* These functions can be modified in case the current settings
   need to be changed for specific application needs */
HAL_StatusTypeDef MX_QSPI_Init(QSPI_HandleTypeDef * hQspi, MX_QSPI_Init_t * Config);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* STM32WB5MM_DK_QSPI_H */
