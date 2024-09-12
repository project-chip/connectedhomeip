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

#ifndef _SL_WFX_BOARD_H_
#define _SL_WFX_BOARD_H_
/*
 * Pull in the right board PINS
 */
#if defined(EFR32MG24_BRD4186C) || defined(BRD4186C) || defined(EFR32MG24_BRD4186A) || defined(BRD4186A)
#include "brd4186c.h"
#elif defined(EFR32MG24_BRD4187C) || defined(BRD4187C) || defined(EFR32MG24_BRD4187A) || defined(BRD4187A)
#include "brd4187c.h"
#else
#include "sl_custom_board.h"
#warning "Modify sl_custom_board.h configuration file to match your hardware SPIDRV USART peripheral"
#endif
#endif /* _SL_WFX_BOARD_H_ */
