/**
 ******************************************************************************
 * @file    stm_ota.c
 * @author  MCD Application Team
 * @brief   Write new image in external flash
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm_ext_flash.h"
#include "cmsis_os.h"
#include "stm32wb5mm_dk_qspi.h"

/* Private defines -----------------------------------------------------------*/
#define ERASE_BLOC_SIZE 0x10000U /*!< 64 Kbytes */

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
osSemaphoreId_t SemExtFlashId;

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static STM_OTA_StatusTypeDef check_addr(uint32_t Address, uint32_t Length);
static void QSPI_Pin_LP(void);
static void QSPI_Pin_WakeUP(void);
/* Public functions ----------------------------------------------------------*/

STM_OTA_StatusTypeDef STM_EXT_FLASH_Init(void)
{
    BSP_QSPI_Init_t init;

    SemExtFlashId = osSemaphoreNew(1, 1, NULL); /*< Create the semaphore and make it available at initialization */

    init.TransferRate  = BSP_QSPI_STR_TRANSFER;
    init.DualFlashMode = BSP_QSPI_DUALFLASH_DISABLE;
    init.InterfaceMode = S25FL128S_QPI_MODE;
    if (BSP_QSPI_Init(0, &init) != BSP_ERROR_NONE)
    {
        return STM_EXT_FLASH_INIT_FAILED;
    }
    else
    {
#if CFG_FULL_LOW_POWER == 1
        QSPI_Pin_LP();
#endif
        return STM_EXT_FLASH_OK;
    }
}

STM_OTA_StatusTypeDef STM_EXT_FLASH_Delete_Image(uint32_t Address, uint32_t Length)
{
    uint32_t loop_flash;

    // check if the address is in the external flash and if the length is < flash size
    if (check_addr(Address, Length) != STM_EXT_FLASH_OK)
    {
        return STM_EXT_FLASH_INVALID_PARAM;
    }

    /* Do nothing if Length equal to 0 */
    if (Length == 0U)
    {
        return STM_EXT_FLASH_OK;
    }

    /* flash address to erase is the offset from begin of external flash */
    Address -= EXTERNAL_FLASH_ADDRESS;

    osSemaphoreAcquire(SemExtFlashId, osWaitForever);
#if CFG_FULL_LOW_POWER == 1
    QSPI_Pin_WakeUP();
#endif
    /* Loop on 64KBytes block */
    for (loop_flash = 0U; loop_flash < (((Length - 1U) / ERASE_BLOC_SIZE) + 1U); loop_flash++)
    {
        if (BSP_QSPI_EraseBlock(0, Address, BSP_QSPI_ERASE_64K) != BSP_ERROR_NONE)
        {
#if CFG_FULL_LOW_POWER == 1
            QSPI_Pin_LP();
#endif
            osSemaphoreRelease(SemExtFlashId);
            return STM_EXT_FLASH_DELETE_FAILED;
        }

        /* next 64KBytes block */
        Address += ERASE_BLOC_SIZE;
    }
#if CFG_FULL_LOW_POWER == 1
    QSPI_Pin_LP();
#endif
    osSemaphoreRelease(SemExtFlashId);
    return STM_EXT_FLASH_OK;
}

STM_OTA_StatusTypeDef STM_EXT_FLASH_WriteChunk(uint32_t DestAddress, uint8_t * pSrcBuffer, uint32_t Length)
{
    int32_t error = 0;
    if (pSrcBuffer == NULL)
    {
        return STM_EXT_FLASH_INVALID_PARAM;
    }
    // check if the address is in the external flash and if the length is < flash size
    if (check_addr(DestAddress, Length) != STM_EXT_FLASH_OK)
    {
        return STM_EXT_FLASH_INVALID_PARAM;
    }
    /* Do nothing if Length equal to 0 */
    if (Length == 0U)
    {
        return STM_EXT_FLASH_OK;
    }
    osSemaphoreAcquire(SemExtFlashId, osWaitForever);
#if CFG_FULL_LOW_POWER == 1
    QSPI_Pin_WakeUP();
#endif
    error = BSP_QSPI_Write(0, pSrcBuffer, DestAddress - EXTERNAL_FLASH_ADDRESS, Length);
    if (error != BSP_ERROR_NONE)
    {
#if CFG_FULL_LOW_POWER == 1
        QSPI_Pin_LP();
#endif
        osSemaphoreRelease(SemExtFlashId);
        return STM_EXT_FLASH_WRITE_FAILED;
    }
    else
    {
#if CFG_FULL_LOW_POWER == 1
        QSPI_Pin_LP();
#endif
        osSemaphoreRelease(SemExtFlashId);
        return STM_EXT_FLASH_OK;
    }
}

STM_OTA_StatusTypeDef STM_EXT_FLASH_ReadChunk(uint32_t DestAddress, uint8_t * pSrcBuffer, uint32_t Length)
{
    int32_t error = 0;
    if (pSrcBuffer == NULL)
    {
        return STM_EXT_FLASH_INVALID_PARAM;
    }
    // check if the address is in the external flash and if the length is < flash size
    if (check_addr(DestAddress, Length) != STM_EXT_FLASH_OK)
    {
        return STM_EXT_FLASH_INVALID_PARAM;
    }

    /* Do nothing if Length equal to 0 */
    if (Length == 0U)
    {
        return STM_EXT_FLASH_OK;
    }
    osSemaphoreAcquire(SemExtFlashId, osWaitForever);
#if CFG_FULL_LOW_POWER == 1
    QSPI_Pin_WakeUP();
#endif
    error = BSP_QSPI_Read(0, pSrcBuffer, DestAddress - EXTERNAL_FLASH_ADDRESS, Length);
    if (error != BSP_ERROR_NONE)
    {
#if CFG_FULL_LOW_POWER == 1
        QSPI_Pin_LP();
#endif
        osSemaphoreRelease(SemExtFlashId);
        return STM_EXT_FLASH_READ_FAILED;
    }
    else
    {
#if CFG_FULL_LOW_POWER == 1
        QSPI_Pin_LP();
#endif
        osSemaphoreRelease(SemExtFlashId);
        return STM_EXT_FLASH_OK;
    }
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static STM_OTA_StatusTypeDef check_addr(uint32_t Address, uint32_t Length)
{
    // check if the address is in the external flash and if the length is < flash size
    if ((Address < EXTERNAL_FLASH_ADDRESS) || (S25FL128S_FLASH_SIZE < Length) ||
        (Address + Length > EXTERNAL_FLASH_ADDRESS + S25FL128S_FLASH_SIZE))
    {
        return STM_EXT_FLASH_INVALID_PARAM;
    }
    else
    {
        return STM_EXT_FLASH_OK;
    }
}

static void QSPI_Pin_WakeUP(void)
{

    GPIO_InitTypeDef gpio_init_structure;

    /*##-1- Configure peripheral GPIO ##########################################*/
    /* QSPI CS GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_CS_PIN;
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &gpio_init_structure);

    /* QSPI CLK GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_CLK_PIN;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &gpio_init_structure);

    /* QSPI D0 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D0_PIN;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_D0_GPIO_PORT, &gpio_init_structure);

    /* QSPI D1 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D1_PIN;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_D1_GPIO_PORT, &gpio_init_structure);

    /* QSPI D2 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D2_PIN;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_D2_GPIO_PORT, &gpio_init_structure);

    /* QSPI D3 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D3_PIN;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_D3_GPIO_PORT, &gpio_init_structure);
}

static void QSPI_Pin_LP(void)
{
    /*##-1- Disable peripherals ################################*/
    /* De-Configure QSPI pins */
    HAL_GPIO_DeInit(QSPI_CS_GPIO_PORT, QSPI_CS_PIN);
    HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
    HAL_GPIO_DeInit(QSPI_D0_GPIO_PORT, QSPI_D0_PIN);
    HAL_GPIO_DeInit(QSPI_D1_GPIO_PORT, QSPI_D1_PIN);
    HAL_GPIO_DeInit(QSPI_D2_GPIO_PORT, QSPI_D2_PIN);
    HAL_GPIO_DeInit(QSPI_D3_GPIO_PORT, QSPI_D3_PIN);
}
