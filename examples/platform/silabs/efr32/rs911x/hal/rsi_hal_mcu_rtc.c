/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * Includes
 */
#include "em_cmu.h"
#include "em_core.h"
#include "em_rtcc.h"
#include "sl_sleeptimer_config.h"
#include "sl_status.h"
#include <stdlib.h>
#include <time.h>

#define ZONE0 0
#define CH_SELECTOR 1u
#define RTC_DEFAULT_COUNTER_VALUE 0u
#define RSI_RTC_FREQ_VALUE 0
#define TIME_ZONE_OFFSET 0u

#define SLEEPTIMER_EVENT_OF (0x01)
#define SLEEPTIMER_EVENT_COMP (0x02)
#define SLEEPTIMER_ENUM(name)                                                                                                      \
    typedef uint8_t name;                                                                                                          \
    enum name##_enum
#define TIME_UNIX_EPOCH (1970u)
#define TIME_NTP_EPOCH (1900u)
#define TIME_ZIGBEE_EPOCH (2000u)
#define TIME_NTP_UNIX_EPOCH_DIFF (TIME_UNIX_EPOCH - TIME_NTP_EPOCH)
#define TIME_ZIGBEE_UNIX_EPOCH_DIFF (TIME_ZIGBEE_EPOCH - TIME_UNIX_EPOCH)
#define TIME_DAY_COUNT_NTP_TO_UNIX_EPOCH (TIME_NTP_UNIX_EPOCH_DIFF * 365u + 17u)      ///< 70 years and 17 leap days
#define TIME_DAY_COUNT_ZIGBEE_TO_UNIX_EPOCH (TIME_ZIGBEE_UNIX_EPOCH_DIFF * 365u + 7u) ///< 30 years and 7 leap days
#define TIME_SEC_PER_DAY (60u * 60u * 24u)
#define TIME_NTP_EPOCH_OFFSET_SEC (TIME_DAY_COUNT_NTP_TO_UNIX_EPOCH * TIME_SEC_PER_DAY)
#define TIME_ZIGBEE_EPOCH_OFFSET_SEC (TIME_DAY_COUNT_ZIGBEE_TO_UNIX_EPOCH * TIME_SEC_PER_DAY)
#define TIME_DAY_PER_YEAR (365u)
#define TIME_SEC_PER_YEAR (TIME_SEC_PER_DAY * TIME_DAY_PER_YEAR)
#define TIME_UNIX_TIMESTAMP_MAX (0x7FFFFFFF)

/// Time zone offset from UTC(second).
typedef int32_t sl_sleeptimer_time_zone_offset_t;
/// Timestamp, wall clock time in seconds.
typedef uint32_t sl_sleeptimer_timestamp_t;
/// @brief Time Format.
SLEEPTIMER_ENUM(sl_sleeptimer_time_format_t){
    TIME_FORMAT_UNIX = 0,           ///< Number of seconds since January 1, 1970, 00:00.
                                    ///< Type is signed, so represented on 31 bit.
    TIME_FORMAT_NTP = 1,            ///< Number of seconds since January 1, 1900, 00:00.
                                    ///< Type is unsigned, so represented on 32 bit.
    TIME_FORMAT_ZIGBEE_CLUSTER = 2, ///< Number of seconds since January 1, 2000, 00:00. Type is
                                    ///< unsigned, so represented on 32 bit.
};

// Initialization flag.
static bool is_sleeptimer_initialized = false;
// Timer frequency in Hz.
static uint32_t timer_frequency;
#if SL_SLEEPTIMER_WALLCLOCK_CONFIG
// Current time count.
static sl_sleeptimer_timestamp_t second_count;
// Tick rest when the frequency is not a divider of the timer width.
static uint32_t overflow_tick_rest = 0;
#endif

/*******************************************************************************
 * Checks if the time stamp, format and time zone are
 *  within the supported range.
 *
 * @param time Time stamp to check.
 * @param format Format of the time.
 * @param time_zone Time zone offset in second.
 *
 * @return true if the time is valid. False otherwise.
 ******************************************************************************/
static bool is_valid_time(sl_sleeptimer_timestamp_t time, sl_sleeptimer_time_format_t format,
                          sl_sleeptimer_time_zone_offset_t time_zone)
{
    bool valid_time = false;

    // Check for overflow.
    if ((time_zone < ZONE0 && time > (uint32_t) abs(time_zone)) || (time_zone >= ZONE0 && (time <= UINT32_MAX - time_zone)))
    {
        valid_time = true;
    }
    if (format == TIME_FORMAT_UNIX)
    {
        if (time > TIME_UNIX_TIMESTAMP_MAX)
        { // Check if Unix time stamp is an
          // unsigned 31 bits.
            valid_time = false;
        }
    }
    else
    {
        if ((format == TIME_FORMAT_NTP) && (time >= TIME_NTP_EPOCH_OFFSET_SEC))
        {
            valid_time &= true;
        }
        else if ((format == TIME_FORMAT_ZIGBEE_CLUSTER) && (time <= TIME_UNIX_TIMESTAMP_MAX - TIME_ZIGBEE_EPOCH_OFFSET_SEC))
        {
            valid_time &= true;
        }
        else
        {
            valid_time = false;
        }
    }
    return valid_time;
}

/*******************************************************************************
 * Gets RTCC timer frequency.
 ******************************************************************************/
uint32_t rsi_rtc_get_hal_timer_frequency(void)
{
    /* CMU_PrescToLog2 converts prescaler dividend to a logarithmic value. It only works for even
     * numbers equal to 2^n.
     * An unscaled dividend (dividend = argument + 1).
     * So we need to send argument substracted by 1
     */
    return (CMU_ClockFreqGet(cmuClock_RTCC) >> (CMU_PrescToLog2(SL_SLEEPTIMER_FREQ_DIVIDER - 1)));
}

/******************************************************************************
 * Initializes RTCC sleep timer.
 *****************************************************************************/
void rsi_rtc_init_timer(void)
{
    RTCC_Init_TypeDef rtcc_init   = RTCC_INIT_DEFAULT;
    RTCC_CCChConf_TypeDef channel = RTCC_CH_INIT_COMPARE_DEFAULT;

    CMU_ClockEnable(cmuClock_RTCC, true);

    rtcc_init.enable = false;

    /* CMU_PrescToLog2 converts prescaler dividend to a logarithmic value. It only works for even
     * numbers equal to 2^n.
     * An unscaled dividend (dividend = argument + 1).
     * So we need to send argument substracted by 1
     */
    rtcc_init.presc = (RTCC_CntPresc_TypeDef) (CMU_PrescToLog2(SL_SLEEPTIMER_FREQ_DIVIDER - 1));

    RTCC_Init(&rtcc_init);

    // Compare channel starts disabled and is enabled only when compare match
    // interrupt is enabled.
    channel.chMode = rtccCapComChModeOff;
    RTCC_ChannelInit(CH_SELECTOR, &channel);

    RTCC_IntDisable(_RTCC_IEN_MASK);
    RTCC_IntClear(_RTCC_IF_MASK);
    RTCC_CounterSet(RTC_DEFAULT_COUNTER_VALUE);

    RTCC_Enable(true);

    NVIC_ClearPendingIRQ(RTCC_IRQn);
    NVIC_EnableIRQ(RTCC_IRQn);
}

/******************************************************************************
 * Enables RTCC interrupts.
 *****************************************************************************/
void rsi_rtc_enable_int(uint8_t local_flag)
{
    uint32_t rtcc_ien = 0u;

    if (local_flag & SLEEPTIMER_EVENT_OF)
    {
        rtcc_ien |= RTCC_IEN_OF;
    }

    if (local_flag & SLEEPTIMER_EVENT_COMP)
    {
        rtcc_ien |= RTCC_IEN_CC1;
    }

    RTCC_IntEnable(rtcc_ien);
}

/*******************************************************************************
 * Get timer frequency.
 ******************************************************************************/
uint32_t rsi_rtc_get_timer_frequency(void)
{
    return timer_frequency;
}

/******************************************************************************
 * Gets RTCC counter value.
 *****************************************************************************/
uint32_t rsi_rtc_get_counter(void)
{
    return RTCC_CounterGet();
}

sl_status_t rsi_rtc_init(void)
{
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    if (!is_sleeptimer_initialized)
    {
        rsi_rtc_init_timer();
        rsi_rtc_enable_int(SLEEPTIMER_EVENT_OF);
        timer_frequency = rsi_rtc_get_hal_timer_frequency();
        if (timer_frequency == RSI_RTC_FREQ_VALUE)
        {
            CORE_EXIT_ATOMIC();
            return SL_STATUS_INVALID_PARAMETER;
        }

#if SL_SLEEPTIMER_WALLCLOCK_CONFIG
        second_count = 0;
#endif

        is_sleeptimer_initialized = true;
    }
    CORE_EXIT_ATOMIC();

    return SL_STATUS_OK;
}

#if SL_SLEEPTIMER_WALLCLOCK_CONFIG
/*******************************************************************************
 * Retrieves current time.
 ******************************************************************************/
uint32_t rsi_rtc_get_time(void)
{
    uint32_t cnt  = 0u;
    uint32_t freq = 0u;
    sl_sleeptimer_timestamp_t time;
    CORE_DECLARE_IRQ_STATE;

    cnt  = rsi_rtc_get_counter();
    freq = rsi_rtc_get_timer_frequency();

    CORE_ENTER_ATOMIC();
    time = second_count + cnt / freq;
    if (cnt % freq + overflow_tick_rest >= freq)
    {
        time++;
    }
    CORE_EXIT_ATOMIC();

    return time;
}

/*******************************************************************************
 * Sets current time.
 ******************************************************************************/
sl_status_t rsi_rtc_settime(sl_sleeptimer_timestamp_t time)
{
    uint32_t freq        = 0u;
    uint32_t counter_sec = 0u;
    uint32_t cnt         = 0;
    CORE_DECLARE_IRQ_STATE;

    if (!is_valid_time(time, TIME_FORMAT_UNIX, TIME_ZONE_OFFSET))
    {
        return SL_STATUS_INVALID_CONFIGURATION;
    }

    freq = rsi_rtc_get_timer_frequency();
    cnt  = rsi_rtc_get_counter();

    CORE_ENTER_ATOMIC();
    second_count       = time;
    overflow_tick_rest = 0;
    counter_sec        = cnt / freq;

    if (second_count >= counter_sec)
    {
        second_count -= counter_sec;
    }
    else
    {
        CORE_EXIT_ATOMIC();
        return SL_STATUS_INVALID_CONFIGURATION;
    }

    CORE_EXIT_ATOMIC();

    return SL_STATUS_OK;
}

/*******************************************************************************
 * @fn  int32_t rsi_rtc_set_time(uint32_t time)
 * @brief
 *      Init Sleeptimer and Set current time.
 * @param[in] time:
 * @return
 *      None
 ******************************************************************************/
int32_t rsi_rtc_set_time(uint32_t time)
{
    sl_status_t ret_val = SL_STATUS_OK;
    ret_val             = rsi_rtc_init();
    if (ret_val != SL_STATUS_OK)
    {
        return -1;
    }
    else
    {
        ret_val = rsi_rtc_settime(time);
        if (ret_val != SL_STATUS_OK)
        {
            return -1;
        }
    }
    return 0;
}
#endif // SL_SLEEPTIMER_WALLCLOCK_CONFIG
