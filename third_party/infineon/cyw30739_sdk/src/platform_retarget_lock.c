/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

#include <sys/lock.h>
#include <wiced_rtos.h>

typedef struct __lock
{
    wiced_mutex_t * mutex;
} lock_t;

static void lock_init_recursive(lock_t * lock);

lock_t __lock___sinit_recursive_mutex;
lock_t __lock___sfp_recursive_mutex;
lock_t __lock___atexit_recursive_mutex;
lock_t __lock___malloc_recursive_mutex;
lock_t __lock___env_recursive_mutex;
lock_t __lock___tz_mutex;

void platform_retarget_lock_init(void)
{
    lock_init_recursive(&__lock___sinit_recursive_mutex);
    lock_init_recursive(&__lock___sfp_recursive_mutex);
    lock_init_recursive(&__lock___malloc_recursive_mutex);
}

void __retarget_lock_init_recursive(lock_t ** lock) {}

void __retarget_lock_close_recursive(lock_t * lock) {}

void __retarget_lock_acquire(lock_t * lock) {}

void __retarget_lock_acquire_recursive(lock_t * lock)
{
    if (lock != NULL && lock->mutex != NULL)
    {
        wiced_rtos_lock_mutex(lock->mutex);
    }
}

void __retarget_lock_release(lock_t * lock) {}

void __retarget_lock_release_recursive(lock_t * lock)
{
    if (lock != NULL && lock->mutex != NULL)
    {
        wiced_rtos_unlock_mutex(lock->mutex);
    }
}

void lock_init_recursive(lock_t * lock)
{
    lock->mutex = wiced_rtos_create_mutex();
    if (lock->mutex != NULL)
    {
        wiced_rtos_init_mutex(lock->mutex);
    }
}
