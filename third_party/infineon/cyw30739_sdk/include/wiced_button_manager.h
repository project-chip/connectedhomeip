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
 * Button manager defines generic interface for button events and button type configurations.
 * It exposes interface to configure platform button events (like click,long press) with user configurable timing.
 */
#pragma once

#include "platform_button.h"
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
 * @brief Button Manager events.
 *
 */
typedef enum
{
    BUTTON_CLICK_EVENT = (1 << 0), /**< A click is a combination of press and release button events. Typically ~ < 200 ms.  */
    BUTTON_SHORT_DURATION_EVENT     = (1 << 1), /**< Short duration click event */
    BUTTON_MEDIUM_DURATION_EVENT    = (1 << 2), /**< Medium duration click event */
    BUTTON_LONG_DURATION_EVENT      = (1 << 3), /**< Long duration click event */
    BUTTON_VERY_LONG_DURATION_EVENT = (1 << 4), /**< Very long duration click event */
    BUTTON_DOUBLE_CLICK_EVENT =
        (1 << 5),                    /**< A double click is a combination of two single clicks with some delay between them   */
    BUTTON_HOLDING_EVENT = (1 << 6), /**< Button is holding over at least two long duration. */
} button_manager_event_t;

/**
 * @brief Button states.
 *
 */
typedef enum
{
    BUTTON_STATE_HELD     = (0), /**< Button state held                                                          */
    BUTTON_STATE_RELEASED = (1), /**< Button state released                                                      */
} button_manager_button_state_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/**
 * @brief Worker thread structure.
 *
 */
typedef struct
{
    wiced_thread_t * thread;     /**< Button event thread                                                          */
    wiced_queue_t * event_queue; /**< Button event queue                                                           */
} button_worker_thread_t;

/**
 * @brief Button Event Message.
 *
 */
typedef struct
{
    event_handler_t function; /**< Button event handler function                 */
    void * arg;               /**< Button event message arguments to be passed   */
} button_event_message_t;
/******************************************************
 *                    Structures
 ******************************************************/

/**
 * @brief Button configuration.
 *
 */
typedef struct
{
    platform_button_t button;   /**< Logical Button-ids which map to physical buttons on the board    */
    uint16_t button_event_mask; /**< Button event mask                                                */
    uint32_t application_event; /**< Application event                                                */
} wiced_button_configuration_t;

/**
 * @brief Button Information.
 *
 */
typedef struct
{
    const wiced_button_configuration_t * configuration; /**< Button configuration               */
    button_manager_button_state_t current_state;        /**< Button current state               */
    button_manager_event_t last_sent_event;             /**< Button last sent event             */
    uint64_t pressed_timestamp;                         /**< Button pressed time stamp (us)     */
    uint64_t released_timestamp;                        /**< Button released time stamp (us)    */
    uint32_t last_released_timestamp;                   /**< Button last released time stamp    */
    wiced_bool_t check_for_double_click;                /**< Button double click event or not   */
    uint32_t repeat;                                    /**< Button holding counter */
    int32_t debounce_counter;                           /**< Button debounce counter            */
    wiced_timer_t debounce_timer;                       /**< Button debounce timer              */
    wiced_bool_t debouncing;                            /**< Button debounce state indication   */
    wiced_timer_t long_press_timer;                     /**< Button long press detection timer  */
    uint64_t timer_timestamp;                           /**< Button event time stamp (us)       */
} button_manager_button_t;

/**
 * Button event handler
 *
 * @param button    Which button in the list has been pressed/released/held
 * @param event     What exact event the button is generating.
 * @param state     What exact state the button is in.
 *
 * @return void     Library should not care whether app handled the button-events correctly or not.
 *
 */
typedef void (*wiced_button_event_handler_t)(const button_manager_button_t * button, button_manager_event_t event,
                                             button_manager_button_state_t state);

/**
 * @brief Button manager Configuration.
 *
 */
typedef struct
{
    uint16_t short_hold_duration;        /**< held for short duration (ms)             */
    uint16_t medium_hold_duration;       /**< held for medium duration (ms)            */
    uint16_t long_hold_duration;         /**< held for long duration (ms)              */
    uint16_t very_long_hold_duration;    /**< held for very long duration (ms)         */
    uint16_t debounce_duration;          /**< duration taken to de-bounce (ms)         */
    uint16_t double_click_interval;      /**< Time interval between two RELEASE events */
    wiced_bool_t continuous_hold_detect; /**< Flag to enable button holding detection.\n
                                              If this is set to true, the BUTTON_VERY_LONG_DURATION_EVENT is disabled
                                              and a BUTTON_HOLDING_EVENT will be triggered if a button is pressed over
                                              twice the long_hold_duration.\n
                                              The BUTTON_HOLDING_EVENT will continuous be sent every long_hold_duration
                                              after the first BUTTON_HOLDING_EVENT event been sent. */
    wiced_button_event_handler_t event_handler;
} wiced_button_manager_configuration_t;

/**
 * @brief Button Manager structure.
 *
 */
typedef struct
{
    const wiced_button_manager_configuration_t * configuration; /**< Button manager configuration               */
    button_manager_button_t * buttons;                          /**< Button information                         */
    uint32_t number_of_buttons;                                 /**< Number of buttons                          */
    wiced_button_event_handler_t button_callback;               /**< Button event handler                       */
} button_manager_t;

/**
 * Button's state change call back
 *
 * @param id            Id of the button
 * @param new_state     What exact new state the button is going to.
 *
 * @return void     Library should not care whether app handled the button-events correctly or not.
 */

typedef void (*wiced_button_state_change_callback_t)(platform_button_t id, wiced_bool_t new_state);

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/**
 * The application should call this function to Initialize the Button Manager
 *
 * @param  manager            : pointer to button manager.
 * @param  configuration      : Configuration for the button.
 * @param  buttons            : Button information.
 * @param  number_of_buttons  : Number of buttons.
 * @return                    : result.
 */
extern wiced_result_t wiced_button_manager_init(button_manager_t * manager,
                                                const wiced_button_manager_configuration_t * configuration,
                                                button_manager_button_t * buttons, uint32_t number_of_buttons);

/**
 * The application should call this function to de-Initialize the Button Manager
 *
 * @param     manager   : Pointer to button manager to de-initialize.
 * @return              : result.
 */
extern wiced_result_t wiced_button_manager_deinit(button_manager_t * manager);

/**
 * Checks if there is pending event and then re-send the event.
 *
 * @return         void : no return value is expected.
 */
extern void wiced_button_manager_pending_event_handle(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
