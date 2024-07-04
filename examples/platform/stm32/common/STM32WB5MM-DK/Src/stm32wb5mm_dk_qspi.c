/**
  ******************************************************************************
  * @file    stm32wb5mm_dk_qspi.c
  * @author  MCD Application Team
  * @brief   This file includes a standard driver for the S25FL128S QSPI
  *          memory mounted on STM32WB5MM-DK board.
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
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
  [..]
   (#) This driver is used to drive the S25FL128S QSPI external
       memory mounted on STM32WB5MM-DK board.

   (#) This driver needs a specific component driver (S25FL128S) to be included with.

   (#) Initialization steps:
       (++) Initialize the QPSI external memory using the BSP_QSPI_Init() function. This
            function includes the MSP layer hardware resources initialization and the
            QSPI interface with the external memory.

   (#) QSPI memory operations
       (++) QSPI memory can be accessed with read/write operations once it is
            initialized.
            Read/write operation can be performed with AHB access using the functions
            BSP_QSPI_Read()/BSP_QSPI_Write().
       (++) The function BSP_QSPI_GetInfo() returns the configuration of the QSPI memory.
            (see the QSPI memory data sheet)
       (++) Perform erase block operation using the function BSP_QSPI_Erase_Block() and by
            specifying the block address. You can perform an erase operation of the whole
            chip by calling the function BSP_QSPI_Erase_Chip().
       (++) The function BSP_QSPI_GetStatus() returns the current status of the QSPI memory.
            (see the QSPI memory data sheet)
       (++) The function BSP_QSPI_EnableMemoryMappedMode enables the QSPI memory mapped mode.
       (++) The function BSP_QSPI_DisableMemoryMappedMode disables the QSPI memory mapped mode.
       (++) The function BSP_QSPI_ReadID() returns 3 bytes memory IDs: Manufacturer ID,
             Memory type, Memory density.
  @endverbatim
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32wb5mm_dk_qspi.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup STM32WB5MM_DK
 * @{
 */

/** @defgroup STM32WB5MM_DK_QSPI STM32WB5MM_DK QSPI
 * @{
 */

/** @defgroup STM32WB5MM_DK_QSPI_Exported_Variables Exported Variables
 * @{
 */
QSPI_HandleTypeDef hqspi;
BSP_QSPI_Ctx_t QSPI_Ctx[QSPI_INSTANCES_NUMBER];
/**
 * @}
 */

/* Private functions ---------------------------------------------------------*/

/** @defgroup STM32WB5MM_DK_QSPI_Private_Functions Private Functions
 * @{
 */
static void QSPI_MspInit(QSPI_HandleTypeDef * hQspi);
static void QSPI_MspDeInit(QSPI_HandleTypeDef * hSspi);
static int32_t QSPI_ResetMemory(uint32_t Instance);
static int32_t QSPI_DummyCyclesCfg(uint32_t Instance);

/**
 * @}
 */

/** @defgroup STM32WB5MM_DK_QSPI_Exported_Functions Exported Functions
 * @{
 */

/**
 * @brief  Initializes the QSPI interface.
 * @param  Instance   QSPI Instance
 * @param  Init QSPI  Init structure
 * @retval BSP status
 */
int32_t BSP_QSPI_Init(uint32_t Instance, BSP_QSPI_Init_t * Init)
{
    int32_t ret = BSP_ERROR_NONE;
    BSP_QSPI_Info_t pInfo;
    MX_QSPI_Init_t qspi_init;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        /* Check if instance is already initialized */
        if (QSPI_Ctx[Instance].IsInitialized == QSPI_ACCESS_NONE)
        {
#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
            /* Register the QSPI MSP Callbacks */
            if (QSPI_Ctx[Instance].IsMspCallbacksValid == 0UL)
            {
                if (BSP_QSPI_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
                {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                }
            }
#else
            /* Msp QSPI initialization */
            QSPI_MspInit(&hqspi);
#endif /* USE_HAL_QSPI_REGISTER_CALLBACKS == 1 */

            if (ret == BSP_ERROR_NONE)
            {
                /* STM32 QSPI interface initialization */
                (void) S25FL128S_GetFlashInfo(&pInfo);
                qspi_init.ClockPrescaler = 4;
                qspi_init.DualFlashMode  = S25FL128S_DUALFLASH_DISABLE;
                qspi_init.FlashSize      = (uint32_t) POSITION_VAL((uint32_t) pInfo.FlashSize) - 1U;
                qspi_init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

                if (MX_QSPI_Init(&hqspi, &qspi_init) != HAL_OK)
                {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                } /* QSPI memory reset */
                else if (QSPI_ResetMemory(Instance) != BSP_ERROR_NONE)
                {
                    ret = BSP_ERROR_COMPONENT_FAILURE;
                } /* Force Flash enter 4 Byte address mode */
                else if (S25FL128S_AutoPollingMemReady(&hqspi, Init->InterfaceMode) != S25FL128S_OK)
                {
                    ret = BSP_ERROR_COMPONENT_FAILURE;
                }
                else if (S25FL128S_Enter4BytesAddressMode(&hqspi, Init->InterfaceMode) != S25FL128S_OK)
                {
                    ret = BSP_ERROR_COMPONENT_FAILURE;
                } /* Configuration of the dummy cycles on QSPI memory side */
                else if (QSPI_DummyCyclesCfg(Instance) != BSP_ERROR_NONE)
                {
                    ret = BSP_ERROR_COMPONENT_FAILURE;
                }
                else
                {
                    QSPI_Ctx[Instance].InterfaceMode = Init->InterfaceMode;
                }
            }
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  De-Initializes the QSPI interface.
 * @param  Instance   QSPI Instance
 * @retval BSP status
 */
int32_t BSP_QSPI_DeInit(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        if (QSPI_Ctx[Instance].IsInitialized == QSPI_ACCESS_MMP)
        {
            if (BSP_QSPI_DisableMemoryMappedMode(Instance) != BSP_ERROR_NONE)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
        }

        if (ret == BSP_ERROR_NONE)
        {
            /* Set default QSPI_Ctx values */
            QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_NONE;
            QSPI_Ctx[Instance].InterfaceMode = BSP_QSPI_SPI_MODE;
            QSPI_Ctx[Instance].TransferRate  = BSP_QSPI_STR_TRANSFER;
            QSPI_Ctx[Instance].DualFlashMode = 0;

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 0)
            QSPI_MspDeInit(&hqspi);
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 0) */

            /* Call the DeInit function to reset the driver */
            if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
            {
                ret = BSP_ERROR_PERIPH_FAILURE;
            }
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Initializes the QSPI interface.
 * @param  hQspi       QSPI handle
 * @param  Config      QSPI configuration structure
 * @retval BSP status
 */
__weak HAL_StatusTypeDef MX_QSPI_Init(QSPI_HandleTypeDef * hQspi, MX_QSPI_Init_t * Config)
{
    /* QSPI initialization */
    /* QSPI freq = SYSCLK /(1 + ClockPrescaler) Mhz */
    hQspi->Instance                = QUADSPI;
    hQspi->Init.ClockPrescaler     = Config->ClockPrescaler;
    hQspi->Init.FifoThreshold      = 1;
    hQspi->Init.SampleShifting     = Config->SampleShifting;
    hQspi->Init.FlashSize          = Config->FlashSize;
    hQspi->Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_8_CYCLE;
    hQspi->Init.ClockMode          = QSPI_CLOCK_MODE_0;

    return HAL_QSPI_Init(hQspi);
}

#if (USE_HAL_QSPI_REGISTER_CALLBACKS == 1)
/**
 * @brief Default BSP QSPI Msp Callbacks
 * @param Instance      QSPI Instance
 * @retval BSP status
 */
int32_t BSP_QSPI_RegisterDefaultMspCallbacks(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        /* Register MspInit/MspDeInit Callbacks */
        if (HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPINIT_CB_ID, QSPI_MspInit) != HAL_OK)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if (HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPDEINIT_CB_ID, QSPI_MspDeInit) != HAL_OK)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
            QSPI_Ctx[Instance].IsMspCallbacksValid = 1U;
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief BSP QSPI Msp Callback registering
 * @param Instance   QSPI Instance
 * @param CallBacks  pointer to MspInit/MspDeInit callbacks functions
 * @retval BSP status
 */
int32_t BSP_QSPI_RegisterMspCallbacks(uint32_t Instance, BSP_QSPI_Cb_t * CallBacks)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        /* Register MspInit/MspDeInit Callbacks */
        if (HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPINIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if (HAL_QSPI_RegisterCallback(&hqspi, HAL_QSPI_MSPDEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
            QSPI_Ctx[Instance].IsMspCallbacksValid = 1U;
        }
    }

    /* Return BSP status */
    return ret;
}
#endif /* (USE_HAL_QSPI_REGISTER_CALLBACKS == 1) */

/**
 * @brief  Reads an amount of data from the QSPI memory.
 * @param  Instance  QSPI instance
 * @param  pData     Pointer to data to be read
 * @param  ReadAddr  Read start address
 * @param  Size      Size of data to read
 * @retval BSP status
 */
int32_t BSP_QSPI_Read(uint32_t Instance, uint8_t * pData, uint32_t ReadAddr, uint32_t Size)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        if (QSPI_Ctx[Instance].TransferRate == BSP_QSPI_STR_TRANSFER)
        {
            if (S25FL128S_ReadSTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode, pData, ReadAddr, Size) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Writes an amount of data to the QSPI memory.
 * @param  Instance   QSPI instance
 * @param  pData      Pointer to data to be written
 * @param  WriteAddr  Write start address
 * @param  Size       Size of data to write
 * @retval BSP status
 */
int32_t BSP_QSPI_Write(uint32_t Instance, uint8_t * pData, uint32_t WriteAddr, uint32_t Size)
{
    int32_t ret = BSP_ERROR_NONE;
    uint32_t end_addr, current_size, current_addr;
    uint8_t * write_data;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        /* Calculation of the size between the write address and the end of the page */
        current_size = S25FL128S_PAGE_SIZE - (WriteAddr % S25FL128S_PAGE_SIZE);

        /* Check if the size of the data is less than the remaining place in the page */
        if (current_size > Size)
        {
            current_size = Size;
        }

        /* Initialize the address variables */
        current_addr = WriteAddr;
        end_addr     = WriteAddr + Size;
        write_data   = pData;

        /* Perform the write page by page */
        do
        {
            /* Check if Flash busy ? */
            if (S25FL128S_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            } /* Enable write operations */
            else if (S25FL128S_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            } /* Issue page program command */
            else if (S25FL128S_PageProgram(&hqspi, QSPI_Ctx[Instance].InterfaceMode, write_data, current_addr, current_size) !=
                     S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            } /* Configure automatic polling mode to wait for end of program */
            else if (S25FL128S_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
            else
            {
                /* Update the address and size variables for next page programming */
                current_addr += current_size;
                write_data += current_size;
                current_size = ((current_addr + S25FL128S_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : S25FL128S_PAGE_SIZE;
            }
        } while ((current_addr < end_addr) && (ret == BSP_ERROR_NONE));
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Erases the specified block of the QSPI memory.
 *         S25FL128S support 4K, 64K size block erase commands.
 * @param  Instance     QSPI instance
 * @param  BlockAddress Block address to erase
 * @param  BlockSize    Erase Block size
 * @retval BSP status
 */
int32_t BSP_QSPI_EraseBlock(uint32_t Instance, uint32_t BlockAddress, BSP_QSPI_Erase_t BlockSize)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        /* Check Flash busy ? */
        if (S25FL128S_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        } /* Enable write operations */
        else if (S25FL128S_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            /* Issue Block Erase command */
            if (S25FL128S_BlockErase(&hqspi, QSPI_Ctx[Instance].InterfaceMode, BlockAddress, BlockSize) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
            if (S25FL128S_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Erases the entire QSPI memory.
 * @param  Instance  QSPI instance
 * @retval BSP status
 */
int32_t BSP_QSPI_EraseChip(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        /* Check Flash busy ? */
        if (S25FL128S_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        } /* Enable write operations */
        else if (S25FL128S_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            /* Issue Chip erase command */
            if (S25FL128S_ChipErase(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Reads current status of the QSPI memory.
 *         If WIP != 0 then return busy.
 * @param  Instance  QSPI instance
 * @retval QSPI memory status: whether busy or not
 */
int32_t BSP_QSPI_GetStatus(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;
    uint8_t reg;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        if (S25FL128S_ReadStatusRegister(&hqspi, QSPI_Ctx[Instance].InterfaceMode, &reg) != S25FL128S_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            /* Check the value of the register */
            if ((reg & S25FL128S_SR1_WIP) != 0U)
            {
                ret = BSP_ERROR_BUSY;
            }
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Return the configuration of the QSPI memory.
 * @param  Instance  QSPI instance
 * @param  pInfo     pointer on the configuration structure
 * @retval BSP status
 */
int32_t BSP_QSPI_GetInfo(uint32_t Instance, BSP_QSPI_Info_t * pInfo)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        (void) S25FL128S_GetFlashInfo(pInfo);
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Configure the QSPI in memory-mapped mode
 *         Only 1 Instance can running MMP mode. And it will lock system at this mode.
 * @param  Instance  QSPI instance
 * @retval BSP status
 */
int32_t BSP_QSPI_EnableMemoryMappedMode(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        if (QSPI_Ctx[Instance].TransferRate == BSP_QSPI_STR_TRANSFER)
        {
            if (S25FL128S_EnableMemoryMappedModeSTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
            else /* Update QSPI context if all operations are well done */
            {
                QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_MMP;
            }
        }
        else
        {
            /* Update QSPI context if all operations are well done */
            QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_MMP;
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  Exit form memory-mapped mode
 *         Only 1 Instance can run MMP mode. And it will lock system at this mode.
 * @param  Instance  QSPI instance
 * @retval BSP status
 */
int32_t BSP_QSPI_DisableMemoryMappedMode(uint32_t Instance)
{
    uint8_t Dummy;
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        if (QSPI_Ctx[Instance].IsInitialized != QSPI_ACCESS_MMP)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        } /* Abort MMP back to indirect mode */
        else if (HAL_QSPI_Abort(&hqspi) != HAL_OK)
        {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
            /* Force QSPI interface Sampling Shift to half cycle */
            hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;

            if (HAL_QSPI_Init(&hqspi) != HAL_OK)
            {
                ret = BSP_ERROR_PERIPH_FAILURE;
            }
            /* Dummy read for exit from Performance Enhance mode */
            else if (S25FL128S_ReadSTR(&hqspi, QSPI_Ctx[Instance].InterfaceMode, &Dummy, 0, 1) != S25FL128S_OK)
            {
                ret = BSP_ERROR_COMPONENT_FAILURE;
            }
            else /* Update QSPI context if all operations are well done */
            {
                QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_INDIRECT;
            }
        }
    }
    /* Return BSP status */
    return ret;
}

/**
 * @brief  Get flash ID, 3 Bytes
 *         Manufacturer ID, Memory type, Memory density
 * @param  Instance  QSPI instance
 * @param  Id QSPI Identifier
 * @retval BSP status
 */
int32_t BSP_QSPI_ReadID(uint32_t Instance, uint8_t * Id)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Check if the instance is supported */
    if (Instance >= QSPI_INSTANCES_NUMBER)
    {
        ret = BSP_ERROR_WRONG_PARAM;
    }
    else
    {
        if (S25FL128S_ReadID(&hqspi, QSPI_Ctx[Instance].InterfaceMode, Id) != S25FL128S_OK)
        {
            ret = BSP_ERROR_COMPONENT_FAILURE;
        }
    }

    /* Return BSP status */
    return ret;
}

/**
 * @}
 */

/** @defgroup STM32WB5MM_DK_QSPI_Private_Functions Private Functions
 * @{
 */

/**
 * @brief QSPI MSP Initialization
 * @param hQspi : QSPI handle
 *        This function configures the hardware resources used in this example:
 *           - Peripheral's clock enable
 *           - Peripheral's GPIO Configuration
 *           - NVIC configuration for QSPI interrupt
 * @retval None
 */
static void QSPI_MspInit(QSPI_HandleTypeDef * hQspi)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hQspi);
    GPIO_InitTypeDef gpio_init_structure;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable the QuadSPI memory interface clock */
    QSPI_CLK_ENABLE();
    /* Reset the QuadSPI memory interface */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();
    /* Enable GPIO clocks */
    QSPI_CS_GPIO_CLK_ENABLE();
    QSPI_CLK_GPIO_CLK_ENABLE();
    QSPI_D0_GPIO_CLK_ENABLE();
    QSPI_D1_GPIO_CLK_ENABLE();
    QSPI_D2_GPIO_CLK_ENABLE();
    QSPI_D3_GPIO_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* QSPI CS GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_CS_PIN;
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &gpio_init_structure);

    /* QSPI CLK GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_CLK_PIN;
    gpio_init_structure.Pull      = GPIO_PULLUP;
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

    /*##-3- Configure the NVIC for QSPI #########################################*/
    /* NVIC configuration for QSPI interrupt */
    HAL_NVIC_SetPriority(QUADSPI_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
}

/**
 * @brief QSPI MSP De-Initialization
 * @param hQspi  QSPI handle
 *        This function frees the hardware resources used in this example:
 *          - Disable the Peripheral's clock
 *          - Revert GPIO and NVIC configuration to their default state
 * @retval None
 */
static void QSPI_MspDeInit(QSPI_HandleTypeDef * hQspi)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hQspi);

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* De-Configure QSPI pins */
    /* De-Configure QSPI pins */
    HAL_GPIO_DeInit(QSPI_CS_GPIO_PORT, QSPI_CS_PIN);
    HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
    HAL_GPIO_DeInit(QSPI_D0_GPIO_PORT, QSPI_D0_PIN);
    HAL_GPIO_DeInit(QSPI_D1_GPIO_PORT, QSPI_D1_PIN);
    HAL_GPIO_DeInit(QSPI_D2_GPIO_PORT, QSPI_D2_PIN);
    HAL_GPIO_DeInit(QSPI_D3_GPIO_PORT, QSPI_D3_PIN);

    /*##-3- Reset peripherals ##################################################*/
    /* Reset the QuadSPI memory interface */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();

    /* Disable the QuadSPI memory interface clock */
    QSPI_CLK_DISABLE();
}

/**
 * @brief  This function reset the QSPI Flash memory.
 *         For SPI reset to avoid system come from unknown status.
 *         Flash accept 1-1-1, 1-1-2, 1-2-2 commands after reset.
 * @param  Instance  QSPI instance
 * @retval BSP status
 */
static int32_t QSPI_ResetMemory(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;

    /* Wait Flash ready */
    if (S25FL128S_AutoPollingMemReady(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Send RESET ENABLE command in SPI mode (1-1-1) */
    else if (S25FL128S_ResetEnable(&hqspi, BSP_QSPI_SPI_MODE) != S25FL128S_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    } /* Send RESET memory command in SPI mode (1-1-1) */
    else if (S25FL128S_ResetMemory(&hqspi, BSP_QSPI_SPI_MODE) != S25FL128S_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        QSPI_Ctx[Instance].IsInitialized = QSPI_ACCESS_INDIRECT;  /* After reset S/W setting to indirect access   */
        QSPI_Ctx[Instance].InterfaceMode = BSP_QSPI_SPI_MODE;     /* After reset H/W back to SPI mode by default  */
        QSPI_Ctx[Instance].TransferRate  = BSP_QSPI_STR_TRANSFER; /* After reset S/W setting to STR mode          */
    }

    /* Return BSP status */
    return ret;
}

/**
 * @brief  This function configures the dummy cycles on memory side.
 *         Dummy cycle bit locate in Configuration Register[7:6]
 * @param  Instance  QSPI instance
 * @retval BSP status
 */
static int32_t QSPI_DummyCyclesCfg(uint32_t Instance)
{
    int32_t ret = BSP_ERROR_NONE;
    QSPI_CommandTypeDef s_command;
    uint8_t reg[2];

    /* Initialize the read configuration register command */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = S25FL128S_READ_CONFIGURATION_REG1_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 1;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Reception of the data */
    if (HAL_QSPI_Receive(&hqspi, &reg[1], HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Initialize the read status register1 command */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = S25FL128S_READ_STATUS_REG1_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 1;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    /* Configure the command */
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Reception of the data */
    if (HAL_QSPI_Receive(&hqspi, &reg[0], HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Enable write operations */
    if (S25FL128S_WriteEnable(&hqspi, QSPI_Ctx[Instance].InterfaceMode) != S25FL128S_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Update configuration register (with new Latency Code) */
    s_command.Instruction = S25FL128S_WRITE_STATUS_CMD_REG_CMD;
    s_command.NbData      = 2;
    MODIFY_REG(reg[1], S25FL128S_CR1_LC_MASK, S25FL128S_CR1_LC1);

    /* Configure the write volatile configuration register command */
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Transmission of the data Status Register 1 */
    if (HAL_QSPI_Transmit(&hqspi, reg, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        ret = BSP_ERROR_COMPONENT_FAILURE;
    }

    /* Return BSP status */
    return ret;
}

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
