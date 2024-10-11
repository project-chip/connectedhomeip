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

#ifndef _RSI_BOARD_CONFIGURATION_H_
#define _RSI_BOARD_CONFIGURATION_H_

typedef struct
{
    unsigned char port;
    unsigned char pin;
} sl_pin_t;

#define PIN(port_id, pin_id)                                                                                                       \
    (sl_pin_t)                                                                                                                     \
    {                                                                                                                              \
        .port = gpioPort##port_id, .pin = pin_id                                                                                   \
    }

#define PACKET_PENDING_INT_PRI 3
#if defined(EFR32MG24_BRD4186C) || defined(BRD4186C)
#include "brd4186c.h"
#elif defined(EFR32MG24_BRD4187C) || defined(BRD4187C)
#include "brd4187c.h"
#else
#include "sl_custom_board.h"
#warning "Modify sl_custom_board.h configuration file to match your hardware SPIDRV USART peripheral"
#endif
#if EXP_BOARD
#define RESET_PIN PIN(A, 6)
#define INTERRUPT_PIN PIN(A, 7)
#define SLEEP_CONFIRM_PIN PIN(A, 5) /* Exp hdr 7 */
#endif
#endif /* _RSI_BOARD_CONFIGURATION_H_ */
