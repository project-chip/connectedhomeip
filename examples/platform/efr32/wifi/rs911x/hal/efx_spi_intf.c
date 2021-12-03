/*************************************************************************
 *
 * Copyright (c) 2019 Redpine Signals Incorporated. All Rights Reserved.
 *
 * NOTICE:  All  information  contained  herein is, and  remains  the  property of
 * Redpine Signals Incorporated. The intellectual and technical concepts contained
 * herein  are  proprietary to  Redpine Signals Incorporated and may be covered by
 * U.S. and Foreign Patents, patents in process, and are protected by trade secret
 * or copyright law. Dissemination  of this  information or reproduction  of  this
 * material is strictly forbidden unless prior written permission is obtained from
 * Redpine Signals Incorporated.
 */
/**
 * Includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "dmadrv.h"
#include "em_core.h"
#include "sl_status.h"
#include "sl_device_init_clocks.h"
#include "gpiointerrupt.h"
#include "dmadrv.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "wfx_host_events.h"
#include "wfx_rsi.h"

#include "rsi_driver.h"
#include "rsi_board_configuration.h"

static SemaphoreHandle_t spi_sem;
static unsigned int tx_dma_chan, rx_dma_chan;
/*TODO -  FIX This - It belongs somewhere else depending on which USART is used */
static uint32_t dummy_data; /* Used for DMA - when results don't matter */
extern void rsi_gpio_irq_cb (uint8_t irqnum);
//#define RS911X_USE_LDMA

static void
dma_init (void)
{
        /* Note LDMA is init'd by DMADRV_Init */
        DMADRV_Init();
        DMADRV_AllocateChannel(&tx_dma_chan, NULL);
        DMADRV_AllocateChannel(&rx_dma_chan, NULL);
}

/****************************************************************************
 * Initialize SPI peripheral
 *****************************************************************************/
void
sl_wfx_host_init_bus(void)
{
    // Initialize and enable the USART
    USART_InitSync_TypeDef config = USART_INITSYNC_DEFAULT;

    dummy_data      = 0;
    config.master       = true;            // master mode
    config.baudrate     = 10000000u;//5000000;//1000000;         // CLK freq is 1 MHz
    config.autoCsEnable = true;            // CS pin controlled by hardware, not firmware
    config.clockMode    = usartClockMode0; // clock idle low, sample on rising/first edge
    config.msbf         = true;            // send MSB first
    config.enable       = usartDisable;    // Make sure to keep USART disabled until it's all set up
    CMU_ClockEnable(cmuClock_HFPER, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(MY_USART_CLOCK, true);
    USART_InitSync(MY_USART, &config);
#if defined(EFR32MG12)
    MY_USART->CTRL |= (1u << _USART_CTRL_SMSDELAY_SHIFT);
#if 0
    MY_USART->ROUTELOC0 =
        (MY_USART->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK | _USART_ROUTELOC0_CLKLOC_MASK)) |
        (SL_WFX_HOST_PINOUT_SPI_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_CLK_LOC << _USART_ROUTELOC0_CLKLOC_SHIFT);
#else
    MY_USART->ROUTELOC0 = (USART_ROUTELOC0_CLKLOC_LOC1) | //US2_CLK n location 1 = PA8 per datasheet section 6.4 = EXP Header pin 8
            (USART_ROUTELOC0_CSLOC_LOC1)  | // US2_CS on location 1 = PA9 per datasheet section 6.4 = EXP Header pin 10
            (USART_ROUTELOC0_TXLOC_LOC1)  | // US2_TX (MOSI) on location 1 = PA6 per datasheet section 6.4 = EXP Header pin 4
            (USART_ROUTELOC0_RXLOC_LOC1);   // US2_RX (MISO) on location 1 = PA7 per datasheet section 6.4 = EXP Header pin 6
#endif

    MY_USART->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_CLKPEN | USART_ROUTEPEN_CSPEN;
    MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
    USART_Enable (MY_USART, usartEnable);
#elif defined (EFR32MG21)
        /*
         * Route USART0 RX, TX, and CLK to the specified pins.  Note that CS is
         * not controlled by USART0 so there is no write to the corresponding
         * USARTROUTE register to do this.
         */
        GPIO->USARTROUTE[2].RXROUTE = (SPI_MISO_PIN.port << _GPIO_USART_RXROUTE_PORT_SHIFT)
                | (SPI_MISO_PIN.pin << _GPIO_USART_RXROUTE_PIN_SHIFT);
        GPIO->USARTROUTE[2].TXROUTE = (SPI_MOSI_PIN.port << _GPIO_USART_TXROUTE_PORT_SHIFT)
                | (SPI_MOSI_PIN.pin << _GPIO_USART_TXROUTE_PIN_SHIFT);
        GPIO->USARTROUTE[2].CLKROUTE = (SPI_CLOCK_PIN.port << _GPIO_USART_CLKROUTE_PORT_SHIFT)
                | (SPI_CLOCK_PIN.pin << _GPIO_USART_CLKROUTE_PIN_SHIFT);
        GPIO->USARTROUTE[2].CSROUTE = (SPI_CS_PIN.port << _GPIO_USART_CSROUTE_PORT_SHIFT)
                | (SPI_CS_PIN.pin << _GPIO_USART_CSROUTE_PIN_SHIFT);

        // Enable USART interface pins
        GPIO->USARTROUTE[2].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN |    // MISO
                GPIO_USART_ROUTEEN_TXPEN |    // MOSI
                GPIO_USART_ROUTEEN_CLKPEN |
                GPIO_USART_ROUTEEN_CSPEN;

        GPIO_SlewrateSet(SPI_CLOCK_PIN.port, 6, 6);
#else
#error "EFRxx - No UART/HAL"
#endif

    GPIO_DriveStrengthSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, gpioDriveStrengthStrongAlternateStrong);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_TX_PORT, SL_WFX_HOST_PINOUT_SPI_TX_PIN, gpioModePushPull, 1);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_RX_PORT, SL_WFX_HOST_PINOUT_SPI_RX_PIN, gpioModeInput, 1);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, SL_WFX_HOST_PINOUT_SPI_CLK_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_TX_PORT, SL_WFX_HOST_PINOUT_SPI_CS_PIN, gpioModePushPull, 1);

    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CS_PORT, SL_WFX_HOST_PINOUT_SPI_CS_PIN, gpioModePushPull, 1);

}
/*
 * Deal with the PINS that are not associated with SPI -
 * Ie. RESET, Wakeup
 */
void
sl_wfx_host_gpio_init(void)
{
        // Enable GPIO clock.
        CMU_ClockEnable(cmuClock_GPIO, true);

        GPIO_PinModeSet(WFX_RESET_PIN.port, WFX_RESET_PIN.pin, gpioModePushPull, 1);
        GPIO_PinModeSet(WFX_SLEEP_CONFIRM_PIN.port, WFX_SLEEP_CONFIRM_PIN.pin, gpioModePushPull, 0);

        CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

        // Set up interrupt based callback function - trigger on both edges.
        GPIOINT_Init ();
        GPIO_PinModeSet(WFX_INTERRUPT_PIN.port, WFX_INTERRUPT_PIN.pin, gpioModeInputPull, 0);
        GPIO_ExtIntConfig (WFX_INTERRUPT_PIN.port, WFX_INTERRUPT_PIN.pin, SL_WFX_HOST_PINOUT_SPI_IRQ, true, false, true);
        GPIOINT_CallbackRegister (SL_WFX_HOST_PINOUT_SPI_IRQ, rsi_gpio_irq_cb);
        GPIO_IntDisable (1<<SL_WFX_HOST_PINOUT_SPI_IRQ); /* Will be enabled by RSI */

        // Change GPIO interrupt priority (FreeRTOS asserts unless this is done here!)
        NVIC_SetPriority(GPIO_EVEN_IRQn, 5);
        NVIC_SetPriority(GPIO_ODD_IRQn, 5);
}
/*
 * To reset the WiFi CHIP
 */
void
sl_wfx_host_reset_chip (void)
{
        // Pull it low for at least 1 ms to issue a reset sequence
        GPIO_PinOutClear (WFX_RESET_PIN.port, WFX_RESET_PIN.pin);

        // Delay for 10ms
        vTaskDelay(pdMS_TO_TICKS(10));

        // Hold pin high to get chip out of reset
        GPIO_PinOutSet (WFX_RESET_PIN.port, WFX_RESET_PIN.pin);

        // Delay for 3ms
        vTaskDelay(pdMS_TO_TICKS(3));
}
void
rsi_hal_board_init(void)
{
        spi_sem = xSemaphoreCreateBinary();
        xSemaphoreGive(spi_sem);
        WFX_RSI_LOG ("RSI_HAL: init GPIO");
        sl_wfx_host_gpio_init();
        WFX_RSI_LOG ("RSI_HAL: init SPI");
        sl_wfx_host_init_bus();
        dma_init ();
        WFX_RSI_LOG ("RSI_HAL: Reset Wifi");
        // sl_wfx_host_reset_chip ();
        WFX_RSI_LOG ("RSI_HAL: Init done");
}


static bool
rx_dma_complete(unsigned int channel, unsigned int sequenceNo, void * userParam)
{
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        //uint8_t *buf = (void *)userParam;

        (void) channel;
        (void) sequenceNo;
        (void) userParam;

        //WFX_RSI_LOG ("SPI: DMA done [%x,%x,%x,%x]", buf [0], buf [1], buf [2], buf [3]);
        xSemaphoreGiveFromISR(spi_sem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return true;
}

#ifdef RS911X_USE_LDMA
static void
do_ldma_usart (void *rx_buf, void *tx_buf, uint8_t xlen)
{
        LDMA_Descriptor_t ldmaTXDescriptor;
        LDMA_TransferCfg_t ldmaTXConfig;
        LDMA_Descriptor_t ldmaRXDescriptor;
        LDMA_TransferCfg_t ldmaRXConfig;

        WFX_RSI_LOG ("SPI: LDMA len=%d", xlen)

        ldmaTXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(tx_buf, &(MY_USART->TXDATA), xlen);
        ldmaTXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(MY_USART_TX_SIGNAL);

        ldmaRXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(MY_USART->RXDATA), rx_buf, xlen);
        ldmaRXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(MY_USART_RX_SIGNAL);

        // Start both channels
        DMADRV_LdmaStartTransfer (rx_dma_chan,  &ldmaRXConfig, &ldmaRXDescriptor, rx_dma_complete, (void *)0);
        DMADRV_LdmaStartTransfer(tx_dma_chan, &ldmaTXConfig, &ldmaTXDescriptor, (DMADRV_Callback_t)0, (void *)0);
        //LDMA_StartTransfer(RX_LDMA_CHANNEL, &ldmaRXConfig, &ldmaRXDescriptor);
        //LDMA_StartTransfer(TX_LDMA_CHANNEL, &ldmaTXConfig, &ldmaTXDescriptor);
}
#endif /*  RS911X_USE_LDMA */
/*
 * RX buf was specified
 * TX buf was not specified by caller - so we
 * transmit dummy data (typically 0
 */
static void
rx_do_dma (uint8_t *rx_buf, uint16_t xlen)
{
#ifdef RS911X_USE_LDMA
        do_ldma_usart (rx_buf, (void *)&dummy_data, xlen);
#else
        /*
         * The caller wants to receive data -
         * The xmit can be dummy data (no src increment for tx)
         */
        dummy_data = 0;
        DMADRV_PeripheralMemory(rx_dma_chan, MY_USART_RX_SIGNAL, (void *)rx_buf,
                                (void *)&(MY_USART->RXDATA), true, xlen,
                                dmadrvDataSize1, rx_dma_complete, NULL);

        // Start transmit DMA.
        DMADRV_MemoryPeripheral(tx_dma_chan, MY_USART_TX_SIGNAL,
                                (void *) &(MY_USART->TXDATA), (void *) &(dummy_data), false,
                                xlen, dmadrvDataSize1, NULL, NULL);
#endif
}
static void
tx_do_dma (uint8_t *rx_buf, uint8_t *tx_buf, uint16_t xlen)
{
#ifdef RS911X_USE_LDMA
        do_ldma_usart (rx_buf, tx_buf, xlen);
#else
        void *buf;
        bool srcinc;
        /*
         * we have a tx_buf. There are some instances where
         * a rx_buf is not specifed. If one is specified then
         * the caller wants results (auto increment src)
         * TODO - the caller specified 8/32 bit - we should use this
         * instead of dmadrvDataSize1 always
         */
        if (rx_buf == (uint8_t *)0) {
                buf = &dummy_data;
                srcinc = false;
        } else {
                buf = rx_buf;
                srcinc = true;
                /* DEBUG */rx_buf [0] = 0xAA; rx_buf [1] = 0x55;
        }
        DMADRV_PeripheralMemory(rx_dma_chan, MY_USART_RX_SIGNAL, buf,
                                (void *) &(MY_USART->RXDATA), srcinc, xlen,
                                dmadrvDataSize1, rx_dma_complete, buf);
        // Start transmit DMA.
        DMADRV_MemoryPeripheral(tx_dma_chan, MY_USART_TX_SIGNAL,
                                (void *) &(MY_USART->TXDATA), (void *)tx_buf, true, xlen,
                                dmadrvDataSize1, NULL, NULL);
#endif /* USE_LDMA */
}
/*
 * Do a SPI transfer - Mode is 8/16 bit - But every 8 bit is aligned
 */
int16_t
rsi_spi_transfer (uint8_t *tx_buf, uint8_t *rx_buf, uint16_t xlen, uint8_t mode)
{
        //WFX_RSI_LOG ("SPI: Xfer: tx=%x,rx=%x,len=%d",(uint32_t)tx_buf, (uint32_t)rx_buf, xlen);
#ifdef not_yet_tested_programmed_io
        while (!(MY_USART->STATUS & USART_STATUS_TXBL))
        {
        }
        MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

        /*
         * Short stuff should be done via programmed I/O
         */
        if (xlen < 16 && tx_buf && !rx_buf) {
                for (; xlen > 0; --xlen, ++tx_buf)
                {
                        MY_USART->TXDATA = (uint32_t)(*txbuf);

                        while (!(MY_USART->STATUS & USART_STATUS_TXC))
                        {
                        }
                }
                while (!(MY_USART->STATUS & USART_STATUS_TXBL))
                {
                }
        }
#endif /* not_yet_tested_programmed_io */
        if (xlen > 0) {
                MY_USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
                if (xSemaphoreTake(spi_sem, portMAX_DELAY) != pdTRUE) {
                        return RSI_FALSE;
                }
                if (tx_buf == (void *)0) {
                        rx_do_dma (rx_buf, xlen);
                } else {
                        tx_do_dma (rx_buf, tx_buf, xlen);
                }
                //vTaskDelay(pdMS_TO_TICKS(10));
                /*
                 * Wait for the call-back to complete
                 */
                if (xSemaphoreTake(spi_sem, portMAX_DELAY) == pdTRUE)      {
                        xSemaphoreGive(spi_sem);
                }
        }

        return RSI_ERROR_NONE;
}


