/*******************************************************************************
 * @file  efx32_ncp_host.c
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "cmsis_os2.h"
#include "dmadrv.h"
#include "em_cmu.h"
#include "em_core.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "gpiointerrupt.h"
#include "sl_constants.h"
#include "sl_rsi_utility.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_ncp_utility.h"
#include "sl_si91x_status.h"
#include "sl_status.h"
#include "sl_wifi_constants.h"
#include <platform/silabs/wifi/SiWx/ncp/sl_board_configuration.h>
#include <platform/silabs/wifi/ncp/spi_multiplex.h>
#include <stdbool.h>
#include <string.h>

#if defined(SL_CATLOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#ifdef SL_BOARD_NAME
#include "sl_board_control.h"
#endif // SL_BOARD_NAME

static bool dma_callback(unsigned int channel, unsigned int sequenceNo, void * userParam);

uint32_t rx_ldma_channel;
uint32_t tx_ldma_channel;
osMutexId_t ncp_transfer_mutex = 0;

static uint32_t dummy_buffer;
static sl_si91x_host_init_configuration init_config = { 0 };

// LDMA descriptor and transfer configuration structures for USART TX channel
LDMA_Descriptor_t ldmaTXDescriptor;
LDMA_TransferCfg_t ldmaTXConfig;

// LDMA descriptor and transfer configuration structures for USART RX channel
LDMA_Descriptor_t ldmaRXDescriptor;
LDMA_TransferCfg_t ldmaRXConfig;

static osSemaphoreId_t transfer_done_semaphore = NULL;

static bool dma_callback([[maybe_unused]] unsigned int channel, [[maybe_unused]] unsigned int sequenceNo,
                         [[maybe_unused]] void * userParam)
{
#if defined(SL_CATLOG_POWER_MANAGER_PRESENT)
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
    osSemaphoreRelease(transfer_done_semaphore);
    return false;
}

static void gpio_interrupt([[maybe_unused]] uint8_t interrupt_number)
{
    if (NULL != init_config.rx_irq)
    {
        init_config.rx_irq();
    }
}

static void efx32_spi_init(void)
{
    // Default asynchronous initializer (master mode, 1 Mbps, 8-bit data)
    USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

    init.msbf         = true; // MSB first transmission for SPI compatibility
    init.autoCsEnable = false;
    init.baudrate     = USART_INITSYNC_BAUDRATE;

    // Configure SPI bus pins
    GPIO_PinModeSet(SPI_MISO_PIN.port, SPI_MISO_PIN.pin, gpioModeInput, 0);
    GPIO_PinModeSet(SPI_MOSI_PIN.port, SPI_MOSI_PIN.pin, gpioModePushPull, 0);
    GPIO_PinModeSet(SPI_CLOCK_PIN.port, SPI_CLOCK_PIN.pin, gpioModePushPullAlternate, 0);
    GPIO_PinModeSet(SPI_CS_PIN.port, SPI_CS_PIN.pin, gpioModePushPull, 1);
    // Enable clock (not needed on xG21)
    CMU_ClockEnable(SPI_USART_CMU_CLOCK, true);

    /*
     * Route USART RX, TX, and CLK to the specified pins.  Note that CS is
     * not controlled by USART so there is no write to the corresponding
     * USARTROUTE register to do this.
     */
    GPIO->USARTROUTE[SPI_USART_ROUTE_INDEX].RXROUTE =
        (SPI_MISO_PIN.port << _GPIO_USART_RXROUTE_PORT_SHIFT) | (SPI_MISO_PIN.pin << _GPIO_USART_RXROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[SPI_USART_ROUTE_INDEX].TXROUTE =
        (SPI_MOSI_PIN.port << _GPIO_USART_TXROUTE_PORT_SHIFT) | (SPI_MOSI_PIN.pin << _GPIO_USART_TXROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[SPI_USART_ROUTE_INDEX].CLKROUTE =
        (SPI_CLOCK_PIN.port << _GPIO_USART_CLKROUTE_PORT_SHIFT) | (SPI_CLOCK_PIN.pin << _GPIO_USART_CLKROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[SPI_USART_ROUTE_INDEX].CSROUTE =
        (SPI_CS_PIN.port << _GPIO_USART_CSROUTE_PORT_SHIFT) | (SPI_CS_PIN.pin << _GPIO_USART_CSROUTE_PIN_SHIFT);

    // Enable USART interface pins
    GPIO->USARTROUTE[SPI_USART_ROUTE_INDEX].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | // MISO
        GPIO_USART_ROUTEEN_TXPEN |                                               // MOSI
#if !SL_SPICTRL_MUX
        GPIO_USART_ROUTEEN_CSPEN |
#endif
        GPIO_USART_ROUTEEN_CLKPEN;

    // Set slew rate for alternate usage pins
    GPIO_SlewrateSet(SPI_CLOCK_PIN.port, 7, 7);

    // Configure and enable USART
    USART_InitSync(SPI_USART, &init);

    SPI_USART->TIMING |= USART_TIMING_TXDELAY_ONE | USART_TIMING_CSSETUP_ONE | USART_TIMING_CSHOLD_ONE;

    // SPI_USART->CTRL_SET |= USART_CTRL_SMSDELAY;

    // configure packet pending interrupt priority
    NVIC_SetPriority(GPIO_ODD_IRQn, PACKET_PENDING_INT_PRI);
    GPIOINT_CallbackRegister(INTERRUPT_PIN.pin, gpio_interrupt);
    GPIO_PinModeSet(INTERRUPT_PIN.port, INTERRUPT_PIN.pin, gpioModeInputPullFilter, 0);
    GPIO_ExtIntConfig(INTERRUPT_PIN.port, INTERRUPT_PIN.pin, INTERRUPT_PIN.pin, true, false, true);
}

void sl_si91x_host_set_sleep_indicator(void)
{
    GPIO_PinOutSet(SLEEP_CONFIRM_PIN.port, SLEEP_CONFIRM_PIN.pin);
}

void sl_si91x_host_clear_sleep_indicator(void)
{
    GPIO_PinOutClear(SLEEP_CONFIRM_PIN.port, SLEEP_CONFIRM_PIN.pin);
}

uint32_t sl_si91x_host_get_wake_indicator(void)
{
    return GPIO_PinInGet(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin);
}

sl_status_t sl_si91x_host_init(const sl_si91x_host_init_configuration * config)
{
#if SL_SPICTRL_MUX
    sl_status_t status = sl_board_disable_display();
    if (SL_STATUS_OK != status)
    {
        SILABS_LOG("sl_board_disable_display failed with error: %x", status);
        return status;
    }
#endif // SL_SPICTRL_MUX
    init_config.rx_irq  = config->rx_irq;
    init_config.rx_done = config->rx_done;

    // Enable clock (not needed on xG21)
    CMU_ClockEnable(cmuClock_GPIO, true);

#if SL_SPICTRL_MUX
    spi_board_init();
#endif

    if (transfer_done_semaphore == NULL)
    {
        transfer_done_semaphore = osSemaphoreNew(1, 0, NULL);
    }

    if (ncp_transfer_mutex == 0)
    {
        ncp_transfer_mutex = osMutexNew(NULL);
    }

    efx32_spi_init();

    // Start reset line low
    GPIO_PinModeSet(RESET_PIN.port, RESET_PIN.pin, gpioModePushPull, 0);

    // Configure interrupt, sleep and wake confirmation pins
    GPIO_PinModeSet(SLEEP_CONFIRM_PIN.port, SLEEP_CONFIRM_PIN.pin, gpioModeWiredOrPullDown, 1);
    GPIO_PinModeSet(WAKE_INDICATOR_PIN.port, WAKE_INDICATOR_PIN.pin, gpioModeWiredOrPullDown, 0);

    DMADRV_Init();
    DMADRV_AllocateChannel((unsigned int *) &rx_ldma_channel, NULL);
    DMADRV_AllocateChannel((unsigned int *) &tx_ldma_channel, NULL);

    return SL_STATUS_OK;
}

sl_status_t sl_si91x_host_deinit(void)
{
    return SL_STATUS_OK;
}

void sl_si91x_host_enable_high_speed_bus() {}

/*==================================================================*/
/**
 * @fn         sl_status_t sl_si91x_host_spi_transfer(const void *tx_buffer, void *rx_buffer, uint16_t buffer_length)
 * @param[in]  uint8_t *tx_buff, pointer to the buffer with the data to be transferred
 * @param[in]  uint8_t *rx_buff, pointer to the buffer to store the data received
 * @param[in]  uint16_t transfer_length, Number of bytes to send and receive
 * @param[in]  uint8_t mode, To indicate mode 8 BIT/32 BIT mode transfers.
 * @param[out] None
 * @return     0, 0=success
 * @section description
 * This API is used to transfer/receive data to the Wi-Fi module through the SPI interface.
 */
sl_status_t sl_si91x_host_spi_transfer(const void * tx_buffer, void * rx_buffer, uint16_t buffer_length)
{
    osMutexAcquire(ncp_transfer_mutex, 0xFFFFFFFFUL);

#if SL_SPICTRL_MUX
    sl_wfx_host_spi_cs_assert();
#endif // SL_SPICTRL_MUX

    if (buffer_length < 16)
    {
        uint8_t * tx = (tx_buffer != NULL) ? (uint8_t *) tx_buffer : (uint8_t *) &dummy_buffer;
        uint8_t * rx = (rx_buffer != NULL) ? (uint8_t *) rx_buffer : (uint8_t *) &dummy_buffer;
        while (buffer_length > 0)
        {
            while (!(SPI_USART->STATUS & USART_STATUS_TXBL))
            {
            }
            SPI_USART->TXDATA = (uint32_t) *tx;
            while (!(SPI_USART->STATUS & USART_STATUS_TXC))
            {
            }
            *rx = (uint8_t) SPI_USART->RXDATA;
            if (tx_buffer != NULL)
            {
                tx++;
            }
            if (rx_buffer != NULL)
            {
                rx++;
            }
            buffer_length--;
        }
    }
    else
    {
        if (tx_buffer == NULL)
        {
            dummy_buffer = 0;
            ldmaTXDescriptor =
                (LDMA_Descriptor_t) LDMA_DESCRIPTOR_SINGLE_P2P_BYTE(&dummy_buffer, &(SPI_USART->TXDATA), buffer_length);
        }
        else
        {
            ldmaTXDescriptor = (LDMA_Descriptor_t) LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(tx_buffer, &(SPI_USART->TXDATA), buffer_length);
        }

        if (rx_buffer == NULL)
        {
            ldmaRXDescriptor =
                (LDMA_Descriptor_t) LDMA_DESCRIPTOR_SINGLE_P2P_BYTE(&(SPI_USART->RXDATA), &dummy_buffer, buffer_length);
        }
        else
        {
            ldmaRXDescriptor = (LDMA_Descriptor_t) LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(SPI_USART->RXDATA), rx_buffer, buffer_length);
        }

        // Transfer a byte on free space in the USART buffer
        ldmaTXConfig = (LDMA_TransferCfg_t) LDMA_TRANSFER_CFG_PERIPHERAL(SPI_USART_LDMA_TX);

        // Transfer a byte on receive data valid
        ldmaRXConfig = (LDMA_TransferCfg_t) LDMA_TRANSFER_CFG_PERIPHERAL(SPI_USART_LDMA_RX);

#if defined(SL_CATLOG_POWER_MANAGER_PRESENT)
        sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif

        // Start both channels
        DMADRV_LdmaStartTransfer(rx_ldma_channel, &ldmaRXConfig, &ldmaRXDescriptor, dma_callback, NULL);
        DMADRV_LdmaStartTransfer(tx_ldma_channel, &ldmaTXConfig, &ldmaTXDescriptor, NULL, NULL);

        if (osSemaphoreAcquire(transfer_done_semaphore, 1000) != osOK)
        {
            BREAKPOINT();
        }
    }

    osMutexRelease(ncp_transfer_mutex);
#if SL_SPICTRL_MUX
    sl_wfx_host_spi_cs_deassert();
#endif // SL_SPICTRL_MUX
    return SL_STATUS_OK;
}

void sl_si91x_host_hold_in_reset(void)
{
    GPIO_PinModeSet(RESET_PIN.port, RESET_PIN.pin, gpioModePushPull, 1);
    GPIO_PinOutClear(RESET_PIN.port, RESET_PIN.pin);
}

void sl_si91x_host_release_from_reset(void)
{
    GPIO_PinModeSet(RESET_PIN.port, RESET_PIN.pin, gpioModeWiredOrPullDown, 1);
}

void sl_si91x_host_enable_bus_interrupt(void)
{
    NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

void sl_si91x_host_disable_bus_interrupt(void)
{
    NVIC_DisableIRQ(GPIO_ODD_IRQn);
}

bool sl_si91x_host_is_in_irq_context(void)
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U;
}
