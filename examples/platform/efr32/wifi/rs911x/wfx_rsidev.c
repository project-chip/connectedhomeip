#ifdef _WFX_NOT_USED_USING_HAL_INSTEAD_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "gpiointerrupt.h"

/* Need Lwip stuff before rsi is included */
#include "wfx_host_events.h"

#include "rsi_wlan_config.h"
#include "rsi_data_types.h"
#include "rsi_common_apis.h"
#include "rsi_wlan_apis.h"
#include "rsi_wlan.h"
#include "rsi_utils.h"
#include "rsi_socket.h"
#include "rsi_nwk.h"
//#include "rsi_wlan_non_rom.h"
#include "rsi_error.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "wfx_rsi.h"
#include "wfx_host_pinout.h"

/* The following stuff comes from hal/rsi_hal_mcu_interrupt.c */
static void (*rsi_intr_cb) (void);
void
rsi_hal_intr_config(void (* rsi_interrupt_handler)(void))
{
	rsi_intr_cb = rsi_interrupt_handler;
}
/* end of stuff from hal/rsi_hal_mcu_interrupt.c */
static void
wfx_spi_wakeup_irq_callback(uint8_t irqNumber)
{
    BaseType_t bus_task_woken;
    uint32_t interrupt_mask;

    if (irqNumber != SL_WFX_HOST_PINOUT_SPI_IRQ)
        return;
    // Get and clear all pending GPIO interrupts
    interrupt_mask = GPIO_IntGet();
    GPIO_IntClear (interrupt_mask);
    if (rsi_intr_cb) (*rsi_intr_cb) ();
    //bus_task_woken = pdFALSE;
    //xSemaphoreGiveFromISR(wfx_wakeup_sem, &bus_task_woken);
    //vTaskNotifyGiveFromISR(wfx_bus_task_handle, &bus_task_woken);
    //portYIELD_FROM_ISR(bus_task_woken);
}
static void
wfx_host_gpio_init(void)
{
    // Enable GPIO clock.
    CMU_ClockEnable (cmuClock_GPIO, true);

    // Configure WF200 reset pin.
    GPIO_PinModeSet (SL_WFX_HOST_PINOUT_RESET_PORT, SL_WFX_HOST_PINOUT_RESET_PIN, gpioModePushPull, 0);
    // Configure WF200 WUP pin.
    GPIO_PinModeSet (SL_WFX_HOST_PINOUT_WUP_PORT, SL_WFX_HOST_PINOUT_WUP_PIN, gpioModePushPull, 0);

    // GPIO used as IRQ.
    GPIO_PinModeSet (SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN, gpioModeInputPull, 0);
    CMU_OscillatorEnable (cmuOsc_LFXO, true, true);

    // Set up interrupt based callback function - trigger on both edges.
    GPIOINT_Init();
    GPIO_ExtIntConfig (SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT, SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN,
		       SL_WFX_HOST_PINOUT_SPI_IRQ, true, false, true);
    GPIOINT_CallbackRegister (SL_WFX_HOST_PINOUT_SPI_IRQ, wfx_spi_wakeup_irq_callback);

    // Change GPIO interrupt priority (FreeRTOS asserts unless this is done here!)
    NVIC_SetPriority(GPIO_EVEN_IRQn, 5);
    NVIC_SetPriority(GPIO_ODD_IRQn, 5);
}

#define USART SL_WFX_HOST_PINOUT_SPI_PERIPHERAL

#if 0
static SemaphoreHandle_t spi_sem;
static unsigned int tx_dma_channel;
static unsigned int rx_dma_channel;
static uint32_t dummy_rx_data;
static uint32_t dummy_tx_data;
static uint32_t usart_clock;
static uint32_t usart_rx_signal;
static uint32_t usart_tx_signal;
static bool spi_enabled = false;
uint8_t wirq_irq_nb = SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN;
#endif

static int
sl_wfx_host_spi_set_config (void * usart)
{
    int ret = -1;

    if (0)
    {
#if defined(USART0)
    }
    else if (usart == USART0)
    {
        usart_clock     = cmuClock_USART0;
        usart_tx_signal = dmadrvPeripheralSignal_USART0_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART0_RXDATAV;
        ret             = 0;
#endif
#if defined(USART1)
    }
    else if (usart == USART1)
    {
        usart_clock     = cmuClock_USART1;
        usart_tx_signal = dmadrvPeripheralSignal_USART1_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART1_RXDATAV;
        ret             = 0;
#endif
#if defined(USART2)
    }
    else if (usart == USART2)
    {
        usart_clock     = cmuClock_USART2;
        usart_tx_signal = dmadrvPeripheralSignal_USART2_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART2_RXDATAV;
        ret             = 0;
#endif
#if defined(USART3)
    }
    else if (usart == USART3)
    {
        usart_clock     = cmuClock_USART3;
        usart_tx_signal = dmadrvPeripheralSignal_USART3_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART3_RXDATAV;
        ret             = 0;
#endif
#if defined(USART4)
    }
    else if (usart == USART4)
    {
        usart_clock     = cmuClock_USART4;
        usart_tx_signal = dmadrvPeripheralSignal_USART4_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART4_RXDATAV;
        ret             = 0;
#endif
#if defined(USART5)
    }
    else if (usart == USART5)
    {
        usart_clock     = cmuClock_USART5;
        usart_tx_signal = dmadrvPeripheralSignal_USART5_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USART5_RXDATAV;
        ret             = 0;
#endif
#if defined(USARTRF0)
    }
    else if (usart == USARTRF0)
    {
        usart_clock     = cmuClock_USARTRF0;
        usart_tx_signal = dmadrvPeripheralSignal_USARTRF0_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USARTRF0_RXDATAV;
        ret             = 0;
#endif
#if defined(USARTRF1)
    }
    else if (usart == USARTRF1)
    {
        usart_clock     = cmuClock_USARTRF1;
        usart_tx_signal = dmadrvPeripheralSignal_USARTRF1_TXBL;
        usart_rx_signal = dmadrvPeripheralSignal_USARTRF1_RXDATAV;
        ret             = 0;
#endif
    }

    return ret;
}

/****************************************************************************
 * Initialize SPI peripheral
 *****************************************************************************/
sl_status_t
sl_wfx_host_init_bus (void)
{
    int res;

    // Initialize and enable the USART
    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;

    res = sl_wfx_host_spi_set_config(USART);
    if (res != 0)
    {
        return SL_STATUS_FAIL;
    }

    spi_enabled        = true;
    dummy_tx_data      = 0;
    usartInit.baudrate = 36000000u;
    usartInit.msbf     = true;
    CMU_ClockEnable (cmuClock_HFPER, true);
    CMU_ClockEnable (cmuClock_GPIO, true);
    CMU_ClockEnable (usart_clock, true);
    USART_InitSync (USART, &usartInit);
    USART->CTRL |= (1u << _USART_CTRL_SMSDELAY_SHIFT);
    USART->ROUTELOC0 =
        (USART->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK | _USART_ROUTELOC0_CLKLOC_MASK)) |
        (SL_WFX_HOST_PINOUT_SPI_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT) |
        (SL_WFX_HOST_PINOUT_SPI_CLK_LOC << _USART_ROUTELOC0_CLKLOC_SHIFT);

    USART->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_CLKPEN;
    GPIO_DriveStrengthSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, gpioDriveStrengthStrongAlternateStrong);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_TX_PORT, SL_WFX_HOST_PINOUT_SPI_TX_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_RX_PORT, SL_WFX_HOST_PINOUT_SPI_RX_PIN, gpioModeInput, 0);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CLK_PORT, SL_WFX_HOST_PINOUT_SPI_CLK_PIN, gpioModePushPull, 0);


    DMADRV_Init();
    DMADRV_AllocateChannel(&tx_dma_channel, NULL);
    DMADRV_AllocateChannel(&rx_dma_channel, NULL);
    GPIO_PinModeSet(SL_WFX_HOST_PINOUT_SPI_CS_PORT, SL_WFX_HOST_PINOUT_SPI_CS_PIN, gpioModePushPull, 1);
    USART->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

    return SL_STATUS_OK;
}

void
wfx_rsidev_init (void)
{
	wfx_host_gpio_init ();
	//spi_sem = xSemaphoreCreateBinary();
	//xSemaphoreGive(spi_sem);
}
#endif /* _NOT_USED */
