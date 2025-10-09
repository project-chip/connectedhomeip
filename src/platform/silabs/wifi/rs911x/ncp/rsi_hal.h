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

#ifndef _HAL_RSI_HAL_H_
#define _HAL_RSI_HAL_H_
#include "rsi_board_configuration.h"

#define RSI_HAL_NO_COM_PORT /* This will be done by the project */

/* Should be enums */
#define RSI_HAL_RESET_PIN (0)
#define RSI_HAL_MODULE_INTERRUPT (1)
#define RSI_HAL_SLEEP_CONFIRM_PIN (2)
#define RSI_HAL_WAKEUP_INDICATION_PIN (3)
#define RSI_HAL_MODULE_INTERRUPT_PIN (4)
#define RSI_HAL_LP_SLEEP_CONFIRM_PIN (6)

//! Timer related macros
//! Macro to configure timer type in single shot
#define RSI_HAL_TIMER_TYPE_SINGLE_SHOT (0)

// Macro to configure timer type in periodic
#define RSI_HAL_TIMER_TYPE_PERIODIC (1)

// Macro to configure timer in micro seconds mode
#define RSI_HAL_TIMER_MODE_MICRO (0)

// Macro to configure timer in milli seconds mode
#define RSI_HAL_TIMER_MODE_MILLI (1)

//! GPIO Pins related Macros
//! Macro to configure GPIO in output mode
#define RSI_HAL_GPIO_OUTPUT_MODE (1)

// Macro to configure GPIO in input mode
#define RSI_HAL_GPIO_INPUT_MODE (0)

// Macro to drive low value on GPIO
#define RSI_HAL_GPIO_LOW (0)

// Macro to drive high value on GPIO
#define RSI_HAL_GPIO_HIGH (1)

// Macro to drive LDMA channel number
#define RSI_LDMA_TRANSFER_CHANNEL_NUM (3)

// Macro to drive semaphore block minimun timer in milli seconds
#define RSI_SEM_BLOCK_MIN_TIMER_VALUE_MS (50)

/******************************************************
 * *               Function Declarations
 * ******************************************************/

void rsi_hal_board_init(void);
void rsi_switch_to_high_clk_freq(void);
void rsi_hal_intr_config(void (*rsi_interrupt_handler)(void));
void rsi_hal_intr_mask(void);
void rsi_hal_intr_unmask(void);
void rsi_hal_intr_clear(void);
uint8_t rsi_hal_intr_pin_status(void);
void rsi_hal_config_gpio(uint8_t gpio_number, uint8_t mode, uint8_t value);
void rsi_hal_set_gpio(uint8_t gpio_number);
uint8_t rsi_hal_get_gpio(uint8_t gpio_number);
void rsi_hal_clear_gpio(uint8_t gpio_number);
int16_t rsi_spi_transfer(uint8_t * tx_buff, uint8_t * rx_buff, uint16_t transfer_length, uint8_t mode);
int32_t rsi_timer_start(uint8_t timer_node, uint8_t mode, uint8_t type, uint32_t duration, void (*rsi_timer_expiry_handler)(void));
int32_t rsi_timer_stop(uint8_t timer_node);
uint32_t rsi_timer_read(uint8_t timer_node);
void rsi_delay_us(uint32_t delay_us);
void rsi_delay_ms(uint32_t delay_ms);
uint32_t rsi_hal_gettickcount(void);
void SysTick_Handler(void);
void rsi_interrupt_handler(void);

/* RTC Related API's */
uint32_t rsi_rtc_get_time(void);
int32_t rsi_rtc_set_time(uint32_t time);
/* End - RTC Related API's */

#ifdef LOGGING_STATS
void rsi_hal_log_stats_intr_config(void (*rsi_give_wakeup_indication)());
#endif /* LOGGING_STATS */
#ifdef RSI_WITH_OS
void rsi_os_delay_ms(uint32_t delay_ms);
void PORTD_IRQHandler(void);

#endif /* RSI_WITH_OS */

#endif
