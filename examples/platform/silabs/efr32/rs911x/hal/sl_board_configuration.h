/**
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/

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
