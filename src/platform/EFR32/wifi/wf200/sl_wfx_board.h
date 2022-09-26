#ifndef _SL_WFX_BOARD_H_
#define _SL_WFX_BOARD_H_
/*
 * Pull in the right board PINS
 */
#if defined(EFR32MG12_BRD4161A) || defined(BRD4161A) ||           \
    defined(EFR32MG12_BRD4162A) || defined(BRD4162A) ||           \
    defined(EFR32MG12_BRD4163A) || defined(BRD4163A) ||           \
    defined(EFR32MG12_BRD4164A) || defined(BRD4164A)
#include "brd4161a.h"
#elif defined(EFR32MG24_BRD4186C) || defined(BRD4186C) ||         \
      defined(EFR32MG24_BRD4186A) || defined(BRD4186A)
#include "brd4186c.h"
#elif defined(EFR32MG24_BRD4187C) || defined(BRD4187C) ||         \
      defined(EFR32MG24_BRD4187A) || defined(BRD4187A)
#include "brd4187c.h"
#else
#error "Need SPI Pins"
#endif /* EFR32MG12_BRD4161A */
#endif /* _SL_WFX_BOARD_H_ */
