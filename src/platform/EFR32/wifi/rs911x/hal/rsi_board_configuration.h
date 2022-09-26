#ifndef _RSI_BOARD_CONFIGURATION_H_
#define _RSI_BOARD_CONFIGURATION_H_

typedef struct {
  unsigned char port;
  unsigned char pin;
} rsi_pin_t;


#define PIN(port_id, pin_id)                                                   \
  (rsi_pin_t) { .port = gpioPort##port_id, .pin = pin_id }

#if defined(EFR32MG12_BRD4161A) || defined(BRD4161A) ||                      \
    defined(EFR32MG12_BRD4162A) || defined(BRD4162A) ||                        \
    defined(EFR32MG12_BRD4163A) || defined(BRD4163A) ||                        \
    defined(EFR32MG12_BRD4164A) || defined(BRD4164A)
// BRD4161-63-64 are pin to pin compatible for SPI
#include "brd4161a.h"
#elif defined(EFR32MG24_BRD4186C) || defined(BRD4186C)
#include "brd4186c.h"
#elif defined(EFR32MG24_BRD4187C) || defined(BRD4187C)
#include "brd4187c.h"
#else
#error "Need SPI Pins"
#endif /* EFR32MG12_BRD4161A */

#endif /* _RSI_BOARD_CONFIGURATION_H_ */
