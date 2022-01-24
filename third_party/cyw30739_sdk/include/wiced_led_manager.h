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
/** @file
 *
 * This file provides definitions of the LED Manager library interface.
 * LED Manager library provides API's to enable/disable, blink and set brightness of a LED.
 */
#pragma once

#include "platform_led.h"
#include "wiced.h"
#include "wiced_rtos.h"
#include "wiced_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/**
 * @brief Logical LED-id's which map to physical LED's on the board
 *
 */
typedef platform_led_t wiced_led_t;

/**
 * @brief LED configuration
 *
 */
typedef struct
{
    wiced_led_t led; /**< LED id             */
    uint16_t bright; /**< in % from 1 to 100 */
} wiced_led_config_t;
/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * Function to Initialize the LED Manager
 *
 * @param  config      : Configuration for the LED.
 * @return             : result.
 */
extern wiced_result_t wiced_led_manager_init(wiced_led_config_t * config);

/**
 * Function to de-initialize the LED Manager
 *
 * @param  void        : No arguments.
 * @return             : result.
 */
extern wiced_result_t wiced_led_manager_deinit(void);

/**
 * Enables the selected LED
 *
 * @param  led      : LED to be enabled.
 * @return          : result.
 */
extern wiced_result_t wiced_led_manager_enable_led(wiced_led_t led);

/**
 * Disables the selected LED
 *
 * @param  led      : LED to be disabled.
 * @return          : result.
 */
extern wiced_result_t wiced_led_manager_disable_led(wiced_led_t led);

/**
 * Reconfigures the LED
 *
 * @param  config      : Configuration for the LED.
 * @return             : result.
 */
extern wiced_result_t wiced_led_manager_reconfig_led(wiced_led_config_t * config);

/**
 * Function called to blink a LED
 *
 * @param  led            : LED to be blinked.
 * @param  on_period      : on period (ms)
 * @param  off_period     : off period (ms)
 * @return                : result.
 */
wiced_result_t wiced_led_manager_blink_led(wiced_led_t led, uint32_t on_period, uint32_t off_period);
#ifdef __cplusplus
} /* extern "C" */
#endif
