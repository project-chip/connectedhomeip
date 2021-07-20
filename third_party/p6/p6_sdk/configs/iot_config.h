/*
 * Copyright 2020-2021, Cypress Semiconductor Corporation or a subsidiary of
 * Cypress Semiconductor Corporation. All Rights Reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software"), is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
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
/*
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iot_config.h
 * @brief This file contains configuration settings for the MQTT Library.
 */

#ifndef IOT_CONFIG_H_
#define IOT_CONFIG_H_

#include "cy_iot_platform_types.h"

#include <assert.h>
#include <stdlib.h>

/**
 * @addtogroup mqtt_cyport_config
 * @{
 */
/**
 * @brief Default thread priority for the threads created by AWS IoT Device SDK.
 */
#define IOT_THREAD_DEFAULT_PRIORITY (CY_RTOS_PRIORITY_NORMAL)
/**
 * @brief Default thread stack size for the threads created by AWS IoT Device SDK.
 * The stack size may be tuned to suit the desired use case.
 */
#define IOT_THREAD_DEFAULT_STACK_SIZE (2048)

/**
 * @brief Default wait time (in milliseconds) to receive keep-alive responses from the MQTT broker. This value may be adjusted to
 * suit the use case and network environment. Refer aws-iot-device-sdk-embedded-C/doc/lib/mqtt.txt for additional info.
 */
#define IOT_MQTT_RESPONSE_WAIT_MS (5000U)

/**
 * \cond
 * @brief Macros to enable/disable asserts in the IoT Device SDK library.
 * Asserts are disabled by default; to enable asserts, modify these macros to 1.
 */
#define IOT_CONTAINERS_ENABLE_ASSERTS (0)
#define IOT_MQTT_ENABLE_ASSERTS (0)
#define IOT_TASKPOOL_ENABLE_ASSERTS (0)
#define AWS_IOT_SHADOW_ENABLE_ASSERTS (0)
#define AWS_IOT_DEFENDER_ENABLE_ASSERTS (0)
#define AWS_IOT_JOBS_ENABLE_ASSERTS (0)
/**
 * \endcond
 */

/**
 * @brief Insert program diagnostics. This function should have the same signature as
 * [assert](https://pubs.opengroup.org/onlinepubs/9699919799/functions/assert.html)
 */
#if (IOT_CONTAINERS_ENABLE_ASSERTS == 1) || (IOT_MQTT_ENABLE_ASSERTS == 1) || (IOT_TASKPOOL_ENABLE_ASSERTS == 1) ||                \
    (AWS_IOT_SHADOW_ENABLE_ASSERTS == 1) || (AWS_IOT_DEFENDER_ENABLE_ASSERTS == 1) || (AWS_IOT_JOBS_ENABLE_ASSERTS == 1)
#define Iot_DefaultAssert assert
#else
#define Iot_DefaultAssert
#endif

/**
 * @brief Memory allocation. This function should have the same signature as
 * [malloc](http://pubs.opengroup.org/onlinepubs/9699919799/functions/malloc.html)
 */
#define Iot_DefaultMalloc malloc

/**
 * @brief Free memory. This function should have the same signature as
 * [free](http://pubs.opengroup.org/onlinepubs/9699919799/functions/free.html)
 */
#define Iot_DefaultFree free

/**
 * \cond
 * @brief Library logging configuration. Configure the below macros to enable/disable debug logs in the library
 * Refer aws-iot-device-sdk-embedded-C/libraries/standard/common/include/iot_logging.h
 * for supported log levels.
 */
#define IOT_LOG_LEVEL_GLOBAL IOT_LOG_ERROR
#define IOT_LOG_LEVEL_DEMO IOT_LOG_ERROR
#define IOT_LOG_LEVEL_PLATFORM IOT_LOG_ERROR
#define IOT_LOG_LEVEL_NETWORK IOT_LOG_ERROR
#define IOT_LOG_LEVEL_TASKPOOL IOT_LOG_ERROR
#define IOT_LOG_LEVEL_MQTT IOT_LOG_ERROR
#define AWS_IOT_LOG_LEVEL_SHADOW IOT_LOG_ERROR
#define AWS_IOT_LOG_LEVEL_DEFENDER IOT_LOG_ERROR
#define AWS_IOT_LOG_LEVEL_JOBS IOT_LOG_ERROR
/**
 * \endcond
 */

/**
 * @}
 */

#endif /* ifndef IOT_CONFIG_H_ */
