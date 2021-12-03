#ifndef _RSI_BOARD_CONFIGURATION_H_
#define _RSI_BOARD_CONFIGURATION_H_

typedef struct {
    unsigned char port;
    unsigned char pin;
} rsi_pin_t;

//#define CONCAT(a, b)      a ## b
//#define CONCAT3(a, b, c)  a ## b ## c
//#define CONCAT_EXPAND(a, b)      CONCAT(a,b)
//#define CONCAT3_EXPAND(a, b, c)  CONCAT3(a,b,c)

#define PIN(port_id, pin_id)  (rsi_pin_t){.port=gpioPort##port_id, .pin=pin_id}

#if defined(EFR32MG21_BRD4180A) || defined (BRD4180A)
#include "brd4180a.h"
#elif defined(EFR32MG12_BRD4161A) || defined (BRD4161A)
#include "brd4161a.h"
#else
#error "Need SPI Pins"
#endif /* EFR32MG21_BRD4180A */


#endif /* _RSI_BOARD_CONFIGURATION_H_ */
