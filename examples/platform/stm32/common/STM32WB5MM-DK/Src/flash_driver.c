/**
 ******************************************************************************
 * @file           : flash_driver.c
 * @author 		: MCD Application Team
 * @brief          : Dual core Flash driver
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_driver.h"
#include "app_common.h"
#include "main.h"
#include "shci.h"
#include "utilities_conf.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    SEM_LOCK_SUCCESSFUL,
    SEM_LOCK_BUSY,
} SemStatus_t;

typedef enum
{
    FLASH_ERASE,
    FLASH_WRITE,
} FlashOperationType_t;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static SingleFlashOperationStatus_t ProcessSingleFlashOperation(FlashOperationType_t FlashOperationType,
                                                                uint32_t SectorNumberOrDestAddress, uint64_t Data);
/* Public functions ----------------------------------------------------------*/
uint32_t FD_EraseSectors(uint32_t FirstSector, uint32_t NbrOfSectors)
{
    uint32_t loop_flash;
    uint32_t return_value;
    SingleFlashOperationStatus_t single_flash_operation_status;

    single_flash_operation_status = SINGLE_FLASH_OPERATION_DONE;

    /**
     *  Take the semaphore to take ownership of the Flash IP
     */
    while (LL_HSEM_1StepLock(HSEM, CFG_HW_FLASH_SEMID))
        ;

    HAL_FLASH_Unlock();

    /**
     *  Notify the CPU2 that some flash erase activity may be executed
     *  On reception of this command, the CPU2 enables the BLE timing protection versus flash erase processing
     *  The Erase flash activity will be executed only when the BLE RF is idle for at least 25ms
     *  The CPU2 will prevent all flash activity (write or erase) in all cases when the BL RF Idle is shorter than 25ms.
     */
    SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_ON);

    for (loop_flash = 0; (loop_flash < NbrOfSectors) && (single_flash_operation_status == SINGLE_FLASH_OPERATION_DONE);
         loop_flash++)
    {
        single_flash_operation_status = FD_EraseSingleSector(FirstSector + loop_flash);
    }

    if (single_flash_operation_status != SINGLE_FLASH_OPERATION_DONE)
    {
        return_value = NbrOfSectors - loop_flash + 1;
    }
    else
    {
        /**
         *  Notify the CPU2 there will be no request anymore to erase the flash
         *  On reception of this command, the CPU2 will disables the BLE timing protection versus flash erase processing
         *  The protection is active until next end of radio event.
         */
        SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_OFF);

        HAL_FLASH_Lock();

        /**
         *  Release the ownership of the Flash IP
         */
        LL_HSEM_ReleaseLock(HSEM, CFG_HW_FLASH_SEMID, 0);

        return_value = 0;
    }

    return return_value;
}

uint32_t FD_WriteData(uint32_t DestAddress, uint64_t * pSrcBuffer, uint32_t NbrOfData)
{
    uint32_t loop_flash;
    uint32_t return_value;
    SingleFlashOperationStatus_t single_flash_operation_status;

    single_flash_operation_status = SINGLE_FLASH_OPERATION_DONE;

    /**
     *  Take the semaphore to take ownership of the Flash IP
     */
    while (LL_HSEM_1StepLock(HSEM, CFG_HW_FLASH_SEMID))
        ;

    HAL_FLASH_Unlock();

    for (loop_flash = 0; (loop_flash < NbrOfData) && (single_flash_operation_status == SINGLE_FLASH_OPERATION_DONE); loop_flash++)
    {
        single_flash_operation_status = FD_WriteSingleData(DestAddress + (8 * loop_flash), *(pSrcBuffer + loop_flash));
    }

    if (single_flash_operation_status != SINGLE_FLASH_OPERATION_DONE)
    {
        return_value = NbrOfData - loop_flash + 1;
    }
    else
    {
        HAL_FLASH_Lock();

        /**
         *  Release the ownership of the Flash IP
         */
        LL_HSEM_ReleaseLock(HSEM, CFG_HW_FLASH_SEMID, 0);

        return_value = 0;
    }

    return return_value;
}

SingleFlashOperationStatus_t FD_EraseSingleSector(uint32_t SectorNumber)
{
    SingleFlashOperationStatus_t return_value;

    /* The last parameter is unused in that case and set to 0 */
    return_value = ProcessSingleFlashOperation(FLASH_ERASE, SectorNumber, 0);

    return return_value;
}

SingleFlashOperationStatus_t FD_WriteSingleData(uint32_t DestAddress, uint64_t Data)
{
    SingleFlashOperationStatus_t return_value;

    return_value = ProcessSingleFlashOperation(FLASH_WRITE, DestAddress, Data);

    return return_value;
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static SingleFlashOperationStatus_t ProcessSingleFlashOperation(FlashOperationType_t FlashOperationType,
                                                                uint32_t SectorNumberOrDestAddress, uint64_t Data)
{
    SemStatus_t cpu1_sem_status;
    SemStatus_t cpu2_sem_status;
    WaitedSemStatus_t waited_sem_status;
    SingleFlashOperationStatus_t return_status;

    uint32_t page_error;
    FLASH_EraseInitTypeDef p_erase_init;

    waited_sem_status = WAITED_SEM_FREE;

    p_erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    p_erase_init.NbPages   = 1;
    p_erase_init.Page      = SectorNumberOrDestAddress;

    do
    {
        /**
         * When the PESD bit mechanism is used by CPU2 to protect its timing, the PESD bit should be polled here.
         * If the PESD is set, the CPU1 will be stalled when reading literals from an ISR that may occur after
         * the flash processing has been requested but suspended due to the PESD bit.
         *
         * Note: This code is required only when the PESD mechanism is used to protect the CPU2 timing.
         * However, keeping that code make it compatible with the two mechanisms.
         */
        while (LL_FLASH_IsActiveFlag_OperationSuspended())
            ;

        UTILS_ENTER_CRITICAL_SECTION();

        /**
         *  Depending on the application implementation, in case a multitasking is possible with an OS,
         *  it should be checked here if another task in the application disallowed flash processing to protect
         *  some latency in critical code execution
         *  When flash processing is ongoing, the CPU cannot access the flash anymore.
         *  Trying to access the flash during that time stalls the CPU.
         *  The only way for CPU1 to disallow flash processing is to take CFG_HW_BLOCK_FLASH_REQ_BY_CPU1_SEMID.
         */
        cpu1_sem_status = (SemStatus_t) LL_HSEM_GetStatus(HSEM, CFG_HW_BLOCK_FLASH_REQ_BY_CPU1_SEMID);
        if (cpu1_sem_status == SEM_LOCK_SUCCESSFUL)
        {
            /**
             *  Check now if the CPU2 disallows flash processing to protect its timing.
             *  If the semaphore is locked, the CPU2 does not allow flash processing
             *
             *  Note: By default, the CPU2 uses the PESD mechanism to protect its timing,
             *  therefore, it is useless to get/release the semaphore.
             *
             *  However, keeping that code make it compatible with the two mechanisms.
             *  The protection by semaphore is enabled on CPU2 side with the command SHCI_C2_SetFlashActivityControl()
             *
             */
            cpu2_sem_status = (SemStatus_t) LL_HSEM_1StepLock(HSEM, CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID);
            if (cpu2_sem_status == SEM_LOCK_SUCCESSFUL)
            {
                /**
                 * When CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID is taken, it is allowed to only erase one sector or
                 * write one single 64bits data
                 * When either several sectors need to be erased or several 64bits data need to be written,
                 * the application shall first exit from the critical section and try again.
                 */
                if (FlashOperationType == FLASH_ERASE)
                {
                    HAL_FLASHEx_Erase(&p_erase_init, &page_error);
                }
                else
                {
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, SectorNumberOrDestAddress, Data);
                }
                /**
                 *  Release the semaphore to give the opportunity to CPU2 to protect its timing versus the next flash operation
                 *  by taking this semaphore.
                 *  Note that the CPU2 is polling on this semaphore so CPU1 shall release it as fast as possible.
                 *  This is why this code is protected by a critical section.
                 */
                LL_HSEM_ReleaseLock(HSEM, CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, 0);
            }
        }

        UTILS_EXIT_CRITICAL_SECTION();

        if (cpu1_sem_status != SEM_LOCK_SUCCESSFUL)
        {
            /**
             * To avoid looping in ProcessSingleFlashOperation(), FD_WaitForSemAvailable() should implement a mechanism to
             * continue only when CFG_HW_BLOCK_FLASH_REQ_BY_CPU1_SEMID is free
             */
            waited_sem_status = FD_WaitForSemAvailable(WAIT_FOR_SEM_BLOCK_FLASH_REQ_BY_CPU1);
        }
        else if (cpu2_sem_status != SEM_LOCK_SUCCESSFUL)
        {
            /**
             * To avoid looping in ProcessSingleFlashOperation(), FD_WaitForSemAvailable() should implement a mechanism to
             * continue only when CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID is free
             */
            waited_sem_status = FD_WaitForSemAvailable(WAIT_FOR_SEM_BLOCK_FLASH_REQ_BY_CPU2);
        }
    } while (((cpu2_sem_status != SEM_LOCK_SUCCESSFUL) || (cpu1_sem_status != SEM_LOCK_SUCCESSFUL)) &&
             (waited_sem_status != WAITED_SEM_BUSY));

    /**
     * In most BLE application, the flash should not be blocked by the CPU2 longer than FLASH_TIMEOUT_VALUE (1000ms)
     * However, it could be that for some marginal application, this time is longer.
     * In that case either HAL_FLASHEx_Erase() or HAL_FLASH_Program() will exit with FLASH_TIMEOUT_VALUE value.
     * This is not a failing case and there is no other way than waiting the operation to be completed.
     * If for any reason this test is never passed, this means there is a failure in the system and there is no other
     * way to recover than applying a device reset.
     *
     * Note: This code is required only when the PESD mechanism is used to protect the CPU2 timing.
     * However, keeping that code make it compatible with the two mechanisms.
     */
    while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_CFGBSY))
        ;

    if (waited_sem_status != WAITED_SEM_BUSY)
    {
        /**
         * The flash processing has been done. It has not been checked whether it has been successful or not.
         * The only commitment is that it is possible to request a new flash processing
         */
        return_status = SINGLE_FLASH_OPERATION_DONE;
    }
    else
    {
        /**
         * The flash processing has not been executed due to timing protection from either the CPU1 or the CPU2.
         * This status is reported up to the user that should retry after checking that each CPU do not
         * protect its timing anymore.
         */
        return_status = SINGLE_FLASH_OPERATION_NOT_EXECUTED;
    }

    return return_status;
}

/*************************************************************
 *
 * WEAK FUNCTIONS
 *
 *************************************************************/
__WEAK WaitedSemStatus_t FD_WaitForSemAvailable(WaitedSemId_t WaitedSemId)
{
    /**
     * The timing protection is enabled by either CPU1 or CPU2. It should be decided here if the driver shall
     * keep trying to erase/write the flash until successful or if it shall exit and report to the user that the action
     * has not been executed.
     * WAITED_SEM_BUSY returns to the user
     * WAITED_SEM_FREE keep looping in the driver until the action is executed. This will result in the current stack looping
     * until this is done. In a bare metal implementation, only the code within interrupt handler can be executed. With an OS,
     * only task with higher priority can be processed
     *
     */
    return WAITED_SEM_BUSY;
}
