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
 * Button manager implements generic interface for button events and button type configurations.
 * It exposes interface to configure platform button events (like click,long press) with user configurable timing.
 */
#include "wiced_button_manager.h"
#include "clock_timer.h"
#include "platform_button.h"
#include "string.h"
#include "wiced_bt_trace.h"
#ifdef CYW55572
#include "wiced_memory.h"
#include "wiced_misc_rtos_utils.h"
#endif
#include <wiced_sleep.h>

/******************************************************
 *                      Macros
 ******************************************************/

#define BUTTON_TIMER_TIMEOUT (100) /*msec*/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
#if BTSTACK_VER >= 0x03000001
#define TIMER_PARAM_TYPE WICED_TIMER_PARAM_TYPE
#endif

#ifdef CYW55572
#define BUTTON_EVENT_QUEUE_DEPTH 16
#endif

/******************************************************
 *                    Structures
 ******************************************************/
#ifdef CYW55572
typedef struct
{
    button_manager_button_t * p_button;
    button_manager_event_t event;
} button_event_defer_to_mpaf_t;
#endif

/******************************************************
 *               Function Declarations
 ******************************************************/
static void button_state_change_callback(platform_button_t id, wiced_bool_t new_state);
static wiced_result_t button_pressed_event_handler(void * arg);
static wiced_result_t button_released_event_handler(void * arg);
static wiced_result_t deferred_button_timer_handler(void * arg);

static wiced_bool_t button_check_event_mask(button_manager_button_t * button, uint16_t new_event);
static void button_check_for_double_click(button_manager_button_t * button, button_manager_event_t * new_event);
static button_manager_event_t button_deduce_duration_event(button_manager_button_t * button, uint32_t current_interval);
static button_manager_button_t * get_button(platform_button_t id);
#ifdef CYW55572
static void button_event_defer_to_mpaf(void * arg);
#endif

/******************************************************
 *               Variables Definitions
 ******************************************************/
static button_manager_t * button_manager;
#ifdef CYW55572
static wiced_mutex_t * p_mutex_button_event;
static wiced_bt_buffer_q_t button_event_queue;
static wiced_bt_pool_t * p_button_event_pool = NULL;
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/

/*
 * button_long_press_detect_timeout_handler
 *
 * Timeout handler for button long press detect timer.
 *
 * The execution duration of this utility is defined in BUTTON_TIMER_TIMEOUT.
 */
static void button_long_press_detect_timeout_handler(TIMER_PARAM_TYPE arg)
{
    button_manager_button_t * p_button = (button_manager_button_t *) arg;

    /* Check if button is under debouncing state. */
    if (p_button->debouncing)
    {
        return;
    }

    /* Get current button state. */
    p_button->current_state = (button_manager_button_state_t) platform_button_get_value(p_button->configuration->button);

    if (p_button->current_state != BUTTON_STATE_HELD)
    {
        WICED_BT_TRACE("Err: Button %d is already released\n", p_button->configuration->button);

        if (wiced_is_timer_in_use(&p_button->long_press_timer))
        {
            wiced_stop_timer(&p_button->long_press_timer);
        }

        return;
    }
    /* Get current timestatmp. */
    p_button->timer_timestamp = clock_SystemTimeMicroseconds64();

    deferred_button_timer_handler((void *) p_button);
}

/*
 * button_debounce_timeout_handler
 *
 * Timeout handler for button debounce timer.
 */
static void button_debounce_timeout_handler(TIMER_PARAM_TYPE arg)
{
    button_manager_button_t * p_button = (button_manager_button_t *) arg;
    wiced_result_t result;

    // WICED_BT_TRACE("button_debounce_timeout_handler (%d, %d)\n", p_button->configuration->button, p_button->debounce_counter);

    if (p_button->debounce_counter > 0)
    {
        /* Reset the button debounce counter. */
        p_button->debounce_counter = 0;

        button_pressed_event_handler((void *) p_button);
    }
    else
    {
        if (wiced_is_timer_in_use(&p_button->long_press_timer))
        {
            wiced_stop_timer(&p_button->long_press_timer);
        }
    }

    /* Reset the button debounce state. */
    p_button->debouncing = WICED_FALSE;
}

/**
 * The application should call this function to Initialize the Button Manager
 */
wiced_result_t __attribute__((weak))
wiced_button_manager_init(button_manager_t * manager, const wiced_button_manager_configuration_t * configuration,
                          button_manager_button_t * buttons, uint32_t number_of_buttons)
{
    uint32_t a;

    memset(manager, 0, sizeof(*manager));

    manager->configuration     = configuration;
    manager->buttons           = buttons;
    manager->number_of_buttons = number_of_buttons;

    button_manager = manager;

    for (a = 0; a < number_of_buttons; a++)
    {
        platform_button_init(buttons[a].configuration->button);
        platform_button_enable(buttons[a].configuration->button);
        buttons[a].current_state    = BUTTON_STATE_RELEASED;
        buttons[a].repeat           = 0;
        buttons[a].debounce_counter = 0;
        buttons[a].debouncing       = WICED_FALSE;
    }

    platform_button_register_state_change_callback(button_state_change_callback);

    /* Initialize the timers used for detecting the long press event. */
    for (a = 0; a < number_of_buttons; a++)
    {
        wiced_init_timer(&buttons[a].long_press_timer, button_long_press_detect_timeout_handler, (TIMER_PARAM_TYPE) &buttons[a],
                         WICED_MILLI_SECONDS_PERIODIC_TIMER);
    }

    /* Initialize the timers used for de-bounce. */
    for (a = 0; a < number_of_buttons; a++)
    {
        wiced_init_timer(&buttons[a].debounce_timer, button_debounce_timeout_handler, (TIMER_PARAM_TYPE) &buttons[a],
                         WICED_MILLI_SECONDS_TIMER);
    }

#ifdef CYW55572
    p_button_event_pool =
        wiced_bt_create_pool("Button Event", sizeof(button_event_defer_to_mpaf_t), BUTTON_EVENT_QUEUE_DEPTH, NULL);

    if (!p_button_event_pool)
    {
        WICED_BT_TRACE("%s: Fail to create pool.\n", __FUNCTION__);
        return WICED_ERROR;
    }

    wiced_bt_init_q(&button_event_queue, NULL);
    p_mutex_button_event = wiced_rtos_create_mutex();
    if (p_mutex_button_event == NULL)
    {
        WICED_BT_TRACE("%s: Fail to create mutex.\n", __FUNCTION__);
        return WICED_ERROR;
    }

    if (wiced_rtos_init_mutex(p_mutex_button_event) != WICED_SUCCESS)
    {
        WICED_BT_TRACE("%s: Fail to init. mutex.\n", __FUNCTION__);
        return WICED_ERROR;
    }
#endif

    return WICED_SUCCESS;
}

/**
 * The application should call this function to de-Initialize the Button Manager
 *
 * @param     manager   : Pointer to button manager to de-initialize.
 * @return              : result.
 */
wiced_result_t wiced_button_manager_deinit(button_manager_t * manager)
{
    uint32_t a;
    for (a = 0; a < manager->number_of_buttons; a++)
    {
        platform_button_disable(manager->buttons[a].configuration->button);
        platform_button_deinit(manager->buttons[a].configuration->button);
    }

    for (a = 0; a < manager->number_of_buttons; a++)
    {
        if (WICED_TRUE == wiced_is_timer_in_use(&manager->buttons[a].debounce_timer))
        {
            wiced_stop_timer(&manager->buttons[a].debounce_timer);
        }

        wiced_deinit_timer(&manager->buttons[a].debounce_timer);

        if (WICED_TRUE == wiced_is_timer_in_use(&manager->buttons[a].long_press_timer))
        {
            wiced_stop_timer(&manager->buttons[a].long_press_timer);
        }

        wiced_deinit_timer(&manager->buttons[a].long_press_timer);
    }

    button_manager = NULL;
    return WICED_SUCCESS;
}

/**
 * Deferred Handler initiated from timer handler
 *
 * @param     arg   : Arguments passed by the timer framework to timer handler
 * @return          : result
 */
static wiced_result_t deferred_button_timer_handler(void * arg)
{
    button_manager_button_t * p_button = (button_manager_button_t *) arg;
    uint64_t duration; // us
    button_manager_event_t new_held_event = 0;

    /* Check current button state. */
    if (p_button->current_state == BUTTON_STATE_RELEASED)
    {
        return WICED_SUCCESS;
    }

    /* Calculate the time difference. */
    duration = p_button->timer_timestamp - p_button->pressed_timestamp; // us
    duration = duration / 1000;                                         // ms

    /* deduce the event depending on the duration */
    new_held_event = button_deduce_duration_event(p_button, (uint32_t) duration);

    /*
     * timers should be mainly interested in duration-specific events;
     * let release_handler only report Click events to the application
     */
    if (new_held_event == BUTTON_CLICK_EVENT)
    {
        return WICED_SUCCESS;
    }

    if (button_check_event_mask(p_button, new_held_event))
    {
        if (p_button->last_sent_event != BUTTON_HOLDING_EVENT)
        {
            if (p_button->last_sent_event != new_held_event)
            {
                button_manager->configuration->event_handler(p_button, new_held_event, p_button->current_state);
                p_button->last_sent_event = new_held_event;
            }
        }
        else
        {
            button_manager->configuration->event_handler(p_button, new_held_event, p_button->current_state);
            p_button->last_sent_event = new_held_event;
        }
    }

    return WICED_SUCCESS;
}

static void button_state_change_callback_pressed(button_manager_button_t * p_button)
{
    /* Check if the button is under de-bounce state. */
    if (p_button->debouncing)
    { // under de-bounce state
        p_button->debounce_counter++;
    }
    else
    {
        /* ignore pressed event for already pressed button*/
        if (p_button->current_state == BUTTON_STATE_HELD)
        {
            return;
        }

        /* Get current timestamp for pressed event. */
        p_button->pressed_timestamp = clock_SystemTimeMicroseconds64();

        /* Start the button debounce timer. */
        wiced_start_timer(&p_button->debounce_timer, (uint32_t) button_manager->configuration->debounce_duration);

        /* Start the long pressed event detect timer. */
        wiced_start_timer(&p_button->long_press_timer, BUTTON_TIMER_TIMEOUT);

        /* Update information. */
        p_button->debouncing       = WICED_TRUE;
        p_button->debounce_counter = 1;
    }
}

static void button_state_change_callback_released(button_manager_button_t * p_button)
{
    wiced_result_t result;

    /* Check if the button is under de-bounce state. */
    if (p_button->debouncing)
    { // under de-bounce state
        p_button->debounce_counter--;
    }
    else
    {
        /* ignore released event for already released button */
        if (p_button->current_state == BUTTON_STATE_RELEASED)
        {
            return;
        }

        /* Get current timestamp for release event. */
        p_button->released_timestamp = clock_SystemTimeMicroseconds64();

        /* Stop the long pressed event detect timer. */
        if (wiced_is_timer_in_use(&p_button->long_press_timer))
        {
            wiced_stop_timer(&p_button->long_press_timer);
        }

        button_released_event_handler((void *) p_button);
    }
}

/**
 * Call back received when button state is changed.
 *
 * @param     id        : id of the button.
 * @param     new_state : new state of the button.
 * @return         void : no return value is expected.
 */
static void button_state_change_callback(platform_button_t id, wiced_bool_t new_state)
{
    button_manager_button_t * button = get_button(id);

#if 0
    WICED_BT_TRACE("button_state_change_callback (button %d %s, %s, %d, %d)\n",
                   id,
                   button->current_state == BUTTON_STATE_HELD ? "H" : "R",
                   button->debouncing ? "D" : "-",
                   new_state,
                   button->debounce_counter);
#endif

    /* Check module state.*/
    if (button == NULL || button_manager == NULL)
    {
        WICED_BT_TRACE("button manager not initialized\n");
        return;
    }

    if (new_state == WICED_TRUE)
    {
        button_state_change_callback_pressed(button);
    }
    else
    {
        button_state_change_callback_released(button);
    }
}

/**
 * Event handler for button press event.
 *
 * @param     arg   : Arguments passed by the event manager
 * @return    void  : No return value expected.
 */
static wiced_result_t button_pressed_event_handler(void * arg)
{
    button_manager_button_t * button = (button_manager_button_t *) arg;

    if (button->current_state == BUTTON_STATE_HELD)
    {
        return WICED_SUCCESS;
    }

    /** Button is pressed; update the state so that timer-handlers know it */
    button->current_state = BUTTON_STATE_HELD;

    return WICED_SUCCESS;
}

/**
 * Event handler for button release event.
 *
 * @param     arg   : Arguments passed by the event manager
 * @return    void  : No return value expected.
 */

static wiced_result_t button_released_event_handler(void * arg)
{
    button_manager_button_t * button         = (button_manager_button_t *) arg;
    button_manager_event_t new_release_event = 0;
    uint64_t duration; // us

    if (button->current_state == BUTTON_STATE_RELEASED)
    {
        return WICED_SUCCESS;
    }

    button->current_state = BUTTON_STATE_RELEASED;

    /* Calculate the time difference. */
    duration = button->released_timestamp - button->pressed_timestamp; // us
    duration = duration / 1000;                                        // ms

    /** If release event comes before debounce duration, ignore it */
    if (duration <= button_manager->configuration->debounce_duration)
    {
        return WICED_SUCCESS;
    }

    /** deduce the event depending on the duration */
    new_release_event = button_deduce_duration_event(button, (uint32_t) duration);

    /** Check if this Release is from 2nd click of a double-click event */
    button_check_for_double_click(button, &new_release_event);

    /**
     * As the new state is Release and application has asked for this kind of event,
     * send it irrespective of whether timer-handler
     * had sent it previously
     */
    if (button_check_event_mask(button, new_release_event))
    {
#ifndef CYW55572
        button_manager->configuration->event_handler(button, new_release_event, button->current_state);
#else
        /*
         * Button released event is handled by another thread, it needs defer to mpaf thread.
         * Deferred_button_timer_handler(long press) is handled by timer which is in mpaf thread,
         * it does not need defer.
         */
        wiced_result_t result;

        wiced_rtos_lock_mutex(p_mutex_button_event);
        button_event_defer_to_mpaf_t * p_data = (button_event_defer_to_mpaf_t *) wiced_bt_get_buffer_from_pool(p_button_event_pool);

        if (!p_data)
        {
            wiced_rtos_unlock_mutex(p_mutex_button_event);
            WICED_BT_TRACE("Err: release event_handler no memory \n");
            goto DEFER_MPAF_ERROR;
        }

        p_data->p_button = button;
        p_data->event    = new_release_event;
        wiced_bt_enqueue(&button_event_queue, (wiced_bt_buffer_t *) p_data);
        wiced_rtos_unlock_mutex(p_mutex_button_event);
        result = wiced_rtos_defer_execution(WICED_RTOS_DEFER_TO_MPAF_THREAD, &button_event_defer_to_mpaf, NULL);

        if (result != WICED_SUCCESS)
        {
            wiced_rtos_lock_mutex(p_mutex_button_event);
            button_event_defer_to_mpaf_t * p_buf = (button_event_defer_to_mpaf_t *) wiced_bt_dequeue(&button_event_queue);
            wiced_bt_free_buffer(p_buf);
            wiced_rtos_unlock_mutex(p_mutex_button_event);
            WICED_BT_TRACE("Err: release event_handler wiced_rtos_defer_execution (%d)\n", result);
            goto DEFER_MPAF_ERROR;
        }
#endif
    }

#ifdef CYW55572
DEFER_MPAF_ERROR:
#endif

    /** reset the button's last-sent so that a new press/held after this release is handled properly */
    button->last_sent_event = 0;

    return WICED_SUCCESS;
}

/**
 * Checks if the event is a double click event.
 *
 * @param     button    : button information.
 * @param     new_event : new event generated for the button.
 * @return         void : no return value is expected.
 */
static void button_check_for_double_click(button_manager_button_t * button, button_manager_event_t * new_event)
{
    if (!button_check_event_mask(button, BUTTON_DOUBLE_CLICK_EVENT) || *new_event != BUTTON_CLICK_EVENT)
    {
        return;
    }
    /** figure out the time-difference in two-releases */
    if ((button->released_timestamp - button->last_released_timestamp) <= button_manager->configuration->double_click_interval)
    {
        /** morph it as DOUBLE_CLICK */
        *new_event = BUTTON_DOUBLE_CLICK_EVENT;
    }

    button->last_released_timestamp = button->released_timestamp;

    return;
}

/**
 * Checks the event mask for the button
 *
 * @param     button        : button information.
 * @param     new_event     : new event generated for the button.
 * @return    wiced_bool_t  : returns true/false based on the new event.
 */
static wiced_bool_t button_check_event_mask(button_manager_button_t * button, uint16_t new_event)
{
    if (!button)
    {
        return WICED_FALSE;
    }

    return ((new_event & button->configuration->button_event_mask) ? WICED_TRUE : WICED_FALSE);
}

/**
 * Checks duration of the event
 *
 * @param   button                    : the button that been triggered
 * @param   current_interval          : current time interval
 * @return  button_manager_event_t    : returns button manager event.
 */

static button_manager_event_t button_deduce_duration_event(button_manager_button_t * button, uint32_t current_interval)
{
    button_manager_event_t new_event = 0;
    uint32_t target_hold_interval;

    if (current_interval <= button_manager->configuration->debounce_duration)
    {
        return (button_manager_event_t) 0;
    }
    else if (current_interval > button_manager->configuration->debounce_duration &&
             current_interval <= button_manager->configuration->short_hold_duration)
    {
        return BUTTON_CLICK_EVENT;
    }
    else if (current_interval > button_manager->configuration->short_hold_duration &&
             current_interval <= button_manager->configuration->medium_hold_duration)
    {
        return BUTTON_SHORT_DURATION_EVENT;
    }
    else if (current_interval > button_manager->configuration->medium_hold_duration &&
             current_interval <= button_manager->configuration->long_hold_duration)
    {
        return BUTTON_MEDIUM_DURATION_EVENT;
    }
    else if (current_interval > button_manager->configuration->long_hold_duration &&
             current_interval <= button_manager->configuration->very_long_hold_duration)
    {
        button->repeat = 0;
        return BUTTON_LONG_DURATION_EVENT;
    }
    else
    { // current_interval > button_manager->configuration->very_long_hold_duration
        if (button_manager->configuration->continuous_hold_detect == WICED_FALSE)
        {
            return BUTTON_VERY_LONG_DURATION_EVENT;
        }
        else
        {
            target_hold_interval = (button->repeat + 2) * button_manager->configuration->long_hold_duration;

            if (current_interval > target_hold_interval)
            {
                button->repeat++;

                return BUTTON_HOLDING_EVENT;
            }
            else
            {
                return (button_manager_event_t) 0;
            }
        }
    }
}

/**
 * returns button based on the button id
 *
 * @param   id                         : id of the buttonl
 * @return  button_manager_button_t    : returns button.
 */

static button_manager_button_t * get_button(platform_button_t id)
{
    uint8_t a;

    for (a = 0; a < button_manager->number_of_buttons; a++)
    {
        if (button_manager->buttons[a].configuration->button == id)
        {
            return &button_manager->buttons[a];
        }
    }

    return NULL;
}

/**
 * Checks if there is pending event and then re-send the event.
 *
 * @return         void : no return value is expected.
 */
void wiced_button_manager_pending_event_handle(void)
{
    uint32_t a;
    if (wiced_sleep_get_boot_mode() == WICED_SLEEP_FAST_BOOT)
    {
        for (a = 0; a < button_manager->number_of_buttons; a++)
        {
            if (platform_button_pending_event_get_and_clear(button_manager->buttons[a].configuration->button))
            {
                button_manager->configuration->event_handler(&button_manager->buttons[a], BUTTON_CLICK_EVENT,
                                                             BUTTON_STATE_RELEASED);
                break;
            }
        }
    }
}

#ifdef CYW55572
static void button_event_defer_to_mpaf(void * arg)
{
    button_event_defer_to_mpaf_t button_event_buf;

    wiced_rtos_lock_mutex(p_mutex_button_event);
    button_event_defer_to_mpaf_t * p_buf = (button_event_defer_to_mpaf_t *) wiced_bt_dequeue(&button_event_queue);
    memcpy(&button_event_buf, p_buf, sizeof(button_event_defer_to_mpaf_t));
    wiced_bt_free_buffer(p_buf);
    wiced_rtos_unlock_mutex(p_mutex_button_event);
    button_manager->configuration->event_handler(button_event_buf.p_button, button_event_buf.event,
                                                 button_event_buf.p_button->current_state);
}
#endif
