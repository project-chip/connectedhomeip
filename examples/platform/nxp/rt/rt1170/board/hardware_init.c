/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*${header:start}*/
#include "board.h"
#include "clock_config.h"
#include "pin_mux.h"

#if (defined(K32W061_TRANSCEIVER) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE) ||                                                        \
    (defined(WIFI_IW416_BOARD_AW_AM510_USD) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE) ||                                              \
    (defined(WIFI_IW416_BOARD_AW_AM457_USD) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE) ||                                              \
    (defined(WIFI_IW612_BOARD_RD_USD) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE) ||                                                    \
    (defined(WIFI_IW612_BOARD_MURATA_2EL_USD) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE)
#include "controller_hci_uart.h"
#endif
/*${header:end}*/

/*${variable:start}*/
static bool isInitialize = false;
/*${variable:end}*/

/*${function:start}*/

static void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

void BOARD_InitHardware(void)
{
    if (!isInitialize)
    {
        BOARD_ConfigMPU();
        BOARD_InitBootPins();
#if (CHIP_DEVICE_CONFIG_ENABLE_WPA || CHIP_DEVICE_CONFIG_ENABLE_THREAD)
        BOARD_InitUSDHCPins();
#else
        BOARD_InitENETPins();
#endif

        BOARD_InitBootClocks();
        BOARD_InitDebugConsole();

        isInitialize = true;
    }
}

#if (defined(K32W061_TRANSCEIVER) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE) ||                                                        \
    (defined(WIFI_IW612_BOARD_RD_USD) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE) ||                                                    \
    (defined(WIFI_IW612_BOARD_MURATA_2EL_USD) && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE)
int controller_hci_uart_get_configuration(controller_hci_uart_config_t * config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc        = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate = BOARD_BT_UART_DEFAULT_BAUDRATE;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance        = BOARD_BT_UART_INSTANCE;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance     = 0U;
    config->rx_channel       = 0U;
    config->tx_channel       = 1U;
    config->dma_mux_instance = 0U;
    config->rx_request       = kDmaRequestMuxLPUART7Rx;
    config->tx_request       = kDmaRequestMuxLPUART7Tx;
#endif
    config->enableRxRTS = 1u;
    config->enableTxCTS = 1u;
    return 0;
}
#endif

/*${function:end}*/
