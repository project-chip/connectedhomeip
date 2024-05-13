/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/platform.h"

#if defined(MBEDTLS_THREADING_C)
/* Threading mutex implementations for mbedTLS. */
#include "mbedtls/threading.h"
#include "threading_alt.h"
#endif

#if !defined(MBEDTLS_MCUX_FREERTOS_THREADING_ALT) && defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)
extern void CRYPTO_ConfigureThreading(void);
#endif

#include "fsl_adapter_rng.h"
#include "fsl_common.h"
#include "ksdk_mbedtls.h"

/******************************************************************************/
/*************************** FreeRTOS ********************************************/
/******************************************************************************/
#if defined(USE_RTOS) && defined(SDK_OS_FREE_RTOS) && defined(MBEDTLS_FREESCALE_FREERTOS_CALLOC_ALT)
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>

/*---------HEAP_4 calloc --------------------------------------------------*/
#if defined(configFRTOS_MEMORY_SCHEME) && (configFRTOS_MEMORY_SCHEME == 4)
void * pvPortCalloc(size_t num, size_t size)
{
    void * ptr;

    ptr = pvPortMalloc(num * size);
    if (!ptr)
    {
        extern void vApplicationMallocFailedHook(void);
        vApplicationMallocFailedHook();
    }
    else
    {
        memset(ptr, 0, num * size);
    }
    return ptr;
}
#else // HEAP_3
void * pvPortCalloc(size_t num, size_t size)
{
    void * pvReturn;

    vTaskSuspendAll();
    {
        pvReturn = calloc(num, size);
        traceMALLOC(pvReturn, size);
    }
    (void) xTaskResumeAll();

#if (configUSE_MALLOC_FAILED_HOOK == 1)
    {
        if (pvReturn == NULL)
        {
            extern void vApplicationMallocFailedHook(void);
            vApplicationMallocFailedHook();
        }
    }
#endif

    return pvReturn;
}
#endif // configFRTOS_MEMORY_SCHEME
#endif /* USE_RTOS && defined(SDK_OS_FREE_RTOS) && defined(MBEDTLS_FREESCALE_FREERTOS_CALLOC_ALT) */

/*-----------------------------------------------------------*/
/*--------- mbedTLS threading functions for FreeRTOS --------*/
/*--------------- See MBEDTLS_THREADING_ALT -----------------*/
/*-----------------------------------------------------------*/
#if defined(MBEDTLS_MCUX_FREERTOS_THREADING_ALT)
/* Threading mutex implementations for mbedTLS. */
#include "mbedtls/threading.h"
#include "threading_alt.h"

/**
 * @brief Implementation of mbedtls_mutex_init for thread-safety.
 *
 */
void mcux_mbedtls_mutex_init(mbedtls_threading_mutex_t * mutex)
{
    mutex->mutex = xSemaphoreCreateMutex();

    if (mutex->mutex != NULL)
    {
        mutex->is_valid = 1;
    }
    else
    {
        mutex->is_valid = 0;
    }
}

/**
 * @brief Implementation of mbedtls_mutex_free for thread-safety.
 *
 */
void mcux_mbedtls_mutex_free(mbedtls_threading_mutex_t * mutex)
{
    if (mutex->is_valid == 1)
    {
        vSemaphoreDelete(mutex->mutex);
        mutex->is_valid = 0;
    }
}

/**
 * @brief Implementation of mbedtls_mutex_lock for thread-safety.
 *
 * @return 0 if successful, MBEDTLS_ERR_THREADING_MUTEX_ERROR if timeout,
 * MBEDTLS_ERR_THREADING_BAD_INPUT_DATA if the mutex is not valid.
 */
int mcux_mbedtls_mutex_lock(mbedtls_threading_mutex_t * mutex)
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if (mutex->is_valid == 1)
    {
        if (xSemaphoreTake(mutex->mutex, portMAX_DELAY))
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
        }
    }

    return ret;
}

/**
 * @brief Implementation of mbedtls_mutex_unlock for thread-safety.
 *
 * @return 0 if successful, MBEDTLS_ERR_THREADING_MUTEX_ERROR if timeout,
 * MBEDTLS_ERR_THREADING_BAD_INPUT_DATA if the mutex is not valid.
 */
int mcux_mbedtls_mutex_unlock(mbedtls_threading_mutex_t * mutex)
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if (mutex->is_valid == 1)
    {
        if (xSemaphoreGive(mutex->mutex))
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
        }
    }

    return ret;
}

static void CRYPTO_ConfigureThreadingMcux(void)
{
    /* Configure mbedtls to use FreeRTOS mutexes. */
    mbedtls_threading_set_alt(mcux_mbedtls_mutex_init, mcux_mbedtls_mutex_free, mcux_mbedtls_mutex_lock, mcux_mbedtls_mutex_unlock);
}
#endif /* defined(MBEDTLS_MCUX_FREERTOS_THREADING_ALT) */

status_t CRYPTO_InitHardware(void)
{
#if defined(MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT)

    CRYPTO_ConfigureThreadingMcux();

#endif /* (MBEDTLS_THREADING_C) && defined(MBEDTLS_THREADING_ALT) */

    return kStatus_HAL_RngSuccess;
}

int mbedtls_hardware_poll(void * data, unsigned char * output, size_t len, size_t * olen)
{
    hal_rng_status_t status;

    status = HAL_RngGetData(output, len);

    if (status == kStatus_HAL_RngSuccess)
    {
        *olen = len;
        return 0;
    }
    return kStatus_Fail;
}
