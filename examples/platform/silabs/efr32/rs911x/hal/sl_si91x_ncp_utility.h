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
#ifndef SL_SI91X_NCP_UTILITY_H
#define SL_SI91X_NCP_UTILITY_H

#pragma once
#include "FreeRTOS.h"
#include "semphr.h"
#include "silabs_utils.h"
#include "sl_status.h"
#include "spi_multiplex.h"

#if defined(CHIP_9117)
#include "em_usart.h"
#include "sl_board_configuration_SiWx917.h"
#include "sl_spidrv_exp_config.h"

#endif // CHIP_9117

#define USART_INITSYNC_BAUDRATE 12500000

#if SL_SPICTRL_MUX
sl_status_t spi_board_init(void);
#endif // SL_SPICTRL_MUX

extern uint32_t rx_ldma_channel;
extern uint32_t tx_ldma_channel;
#endif // SL_SI91X_NCP_UTILITY_H
