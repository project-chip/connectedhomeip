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
 * This file provides implementation for the LED Manager library interface.
 * LED Manager library provides API's to enable/disable, blink and set brightness of a LED.
 */

#include "wiced_led_manager.h"
#include "platform_led.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_trace.h"
#include "wiced_hal_gpio.h"
#include "wiced_platform.h"
#include "wiced_rtos.h"
#include "wiced_timer.h"
/******************************************************
 *                      Macros
 ******************************************************/
#define LED_FREQ (60) /*Hz*/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
extern platform_led_config_t platform_led_config[PLATFORM_LED_MAX];
/******************************************************
 *                    Structures
 ******************************************************/

/**
 * @brief timer structure for LED manager
 *
 */
typedef struct
{
    wiced_led_t led;
    wiced_bool_t led_state;
    uint32_t on_period;
    uint32_t off_period;
    wiced_timer_t timer;
    wiced_bool_t is_init;
} led_manager_timer;

/**
 * @brief LED timer
 *
 */
static led_manager_timer led_timer[PLATFORM_LED_MAX] = { 0 };

/******************************************************
 *               Function Declarations
 ******************************************************/
void led_timer_function(uint32_t arg);

/**
 * Function to Initialize the LED Manager
 *
 * @param  config      : Configuration for the LED.
 * @return             : result.
 */
wiced_result_t wiced_led_manager_init(wiced_led_config_t * config)
{
    uint32_t i;
    uint16_t bright;
    // WICED_BT_TRACE("%s <<\n",__func__);
    if (config == NULL)
        return WICED_ERROR;

    if (config->led >= PLATFORM_LED_MAX)
    {
        WICED_BT_TRACE("Invalid LED for platform\n");
        return WICED_ERROR;
    }

    /* check whether initialized */
    if (led_timer[config->led].is_init)
    {
        WICED_BT_TRACE("Error: Init LED(%d) multiple times\n", config->led);
        return WICED_ERROR;
    }

    bright = config->bright;

    if (bright > 99)
    {
        bright = 99;
    }

    if (WICED_SUCCESS != platform_led_init(&platform_led_config[config->led], LED_FREQ, bright))
        return WICED_ERROR;

    led_timer[config->led].led = config->led;

    /* initialize timer */
    wiced_init_timer(&led_timer[config->led].timer, &led_timer_function, (uint32_t) config->led,
                     WICED_MILLI_SECONDS_PERIODIC_TIMER);

    led_timer[config->led].is_init = WICED_TRUE;

    return WICED_SUCCESS;
}

/**
 * Function to de-initialize the LED Manager
 *
 * @param  void        : No arguments.
 * @return             : result.
 */
wiced_result_t wiced_led_manager_deinit()
{
    uint32_t i;
    /*if any led active we stop all of them*/
    for (i = 0; i < PLATFORM_LED_MAX; i++)
        platform_led_deinit(&platform_led_config[i]);

    /* deinit timer */
    for (i = 0; i < PLATFORM_LED_MAX; i++)
    {
        if (led_timer[i].is_init)
        {
            wiced_deinit_timer(&led_timer[i].timer);
            led_timer[i].is_init = WICED_FALSE;
        }
    }

    return WICED_SUCCESS;
}

/**
 * Enables the selected LED
 *
 * @param  led      : LED to be enabled.
 * @return          : result.
 */
wiced_result_t wiced_led_manager_enable_led(wiced_led_t led)
{
    wiced_result_t result;
    if (led >= PLATFORM_LED_MAX)
    {
        WICED_BT_TRACE("Invalid LED for platform\n");
        return WICED_ERROR;
    }

    result = platform_led_start(&platform_led_config[led]);

    return result;
}

/**
 * Disables the selected LED
 *
 * @param  led      : LED to be disabled.
 * @return          : result.
 */
wiced_result_t wiced_led_manager_disable_led(wiced_led_t led)
{
    wiced_result_t result;
    if (led >= PLATFORM_LED_MAX)
    {
        WICED_BT_TRACE("Invalid LED for platform\n");
        return WICED_ERROR;
    }
    if (wiced_is_timer_in_use(&led_timer[led].timer))
    {
        wiced_stop_timer(&led_timer[led].timer);
        // wiced_deinit_timer(&led_timer[led].timer);
    }

    result = platform_led_stop(&platform_led_config[led]);

    return result;
}

/**
 * Reconfigures the LED
 *
 * @param  config      : Configuration for the LED.
 * @return             : result.
 */
wiced_result_t wiced_led_manager_reconfig_led(wiced_led_config_t * config)
{
    uint16_t bright;

    if (config == NULL)
        return WICED_ERROR;

    if (config->led >= PLATFORM_LED_MAX)
    {
        WICED_BT_TRACE("Invalid LED for platform\n");
        return WICED_ERROR;
    }

    bright = config->bright;

    if (bright > 99)
    {
        bright = 99;
    }

    return platform_led_reinit(&platform_led_config[config->led], LED_FREQ, bright);
}

/**
 * LED timer handler
 *
 * @param  arg            : arguments passed to the handler.
 * @return                : no return value expected.
 */
void led_timer_function(uint32_t arg)
{
    if (led_timer[arg].led_state == WICED_TRUE)
    {
        // WICED_BT_TRACE("Timer led %d stop\n",led_timer[arg].led);
        platform_led_stop(&platform_led_config[led_timer[arg].led]);
        led_timer[arg].led_state = WICED_FALSE;
    }
    else
    {
        platform_led_start(&platform_led_config[led_timer[arg].led]);
        // WICED_BT_TRACE("Timer led %d start\n",led_timer[arg].led);
        led_timer[arg].led_state = WICED_TRUE;
    }

    wiced_stop_timer(&led_timer[arg].timer);
    wiced_start_timer(&led_timer[arg].timer,
                      led_timer[arg].led_state == WICED_TRUE ? led_timer[arg].on_period : led_timer[arg].off_period);
}

/**
 * Function called to blink a LED
 *
 * @param  led            : LED to be blinked.
 * @param  on_period      : on period (ms)
 * @param  off_period     : off period (ms)
 * @return                : result.
 */

wiced_result_t wiced_led_manager_blink_led(wiced_led_t led, uint32_t on_period, uint32_t off_period)
{
    // WICED_BT_TRACE("%s <<\n",__func__);
    if (led >= PLATFORM_LED_MAX)
    {
        WICED_BT_TRACE("Invalid LED for platform\n");
        return WICED_ERROR;
    }

    if (led_timer[led].is_init == WICED_FALSE)
    {
        WICED_BT_TRACE("LED(%d) un-init\n", led);
        return WICED_ERROR;
    }

    led_timer[led].on_period  = on_period;
    led_timer[led].off_period = off_period;

    if (WICED_SUCCESS != wiced_led_manager_enable_led(led))
    {
        WICED_BT_TRACE("LED enable failed\n");
        return WICED_ERROR;
    }

    led_timer[led].led_state = WICED_TRUE;
    wiced_start_timer(&led_timer[led].timer, on_period);
    // WICED_BT_TRACE("timer started\n");

    return WICED_SUCCESS;
}
