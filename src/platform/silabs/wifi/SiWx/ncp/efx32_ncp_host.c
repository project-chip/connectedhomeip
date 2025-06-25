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
#include "sl_spidrv_exp_config.h"
#include "sl_spidrv_instances.h"
#include "sl_status.h"
#include "sl_wifi_constants.h"
#include "spidrv.h"
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

#define LDMA_MAX_TRANSFER_LENGTH 4096
#define LDMA_DESCRIPTOR_ARRAY_LENGTH (LDMA_MAX_TRANSFER_LENGTH / 2048)
#define SPI_HANDLE sl_spidrv_exp_handle
#define MAX_DATA_PACKET_SIZE 1800

// use SPI handle for EXP header (configured in project settings)
extern SPIDRV_Handle_t sl_spidrv_exp_handle;
static uint8_t dummy_buffer[MAX_DATA_PACKET_SIZE]   = { 0 };
static sl_si91x_host_init_configuration init_config = { 0 };

uint32_t rx_ldma_channel;
uint32_t tx_ldma_channel;
osMutexId_t ncp_transfer_mutex = 0;

static osSemaphoreId_t transfer_done_semaphore = NULL;

static void gpio_interrupt([[maybe_unused]] uint8_t interrupt_number)
{
    if (NULL != init_config.rx_irq)
    {
        init_config.rx_irq();
    }
}

static void spi_dma_callback(struct SPIDRV_HandleData * handle, Ecode_t transferStatus, int itemsTransferred)
{
    UNUSED_PARAMETER(handle);
    UNUSED_PARAMETER(transferStatus);
    UNUSED_PARAMETER(itemsTransferred);
#if defined(SL_CATLOG_POWER_MANAGER_PRESENT)
    sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
    osSemaphoreRelease(transfer_done_semaphore);
    return;
}

static void efx32_spi_init(void)
{
    SPIDRV_SetBitrate(SPI_HANDLE, USART_INITSYNC_BAUDRATE);

    // Configure SPI bus pins
    GPIO_PinModeSet(SPI_MISO_PIN.port, SPI_MISO_PIN.pin, gpioModeInput, 0);
    GPIO_PinModeSet(SPI_MOSI_PIN.port, SPI_MOSI_PIN.pin, gpioModePushPull, 0);
    GPIO_PinModeSet(SPI_CLOCK_PIN.port, SPI_CLOCK_PIN.pin, gpioModePushPullAlternate, 0);
    GPIO_PinModeSet(SPI_CS_PIN.port, SPI_CS_PIN.pin, gpioModePushPull, 1);

    // configure packet pending interrupt priority
    NVIC_SetPriority(GPIO_ODD_IRQn, PACKET_PENDING_INT_PRI);
    GPIOINT_CallbackRegister(INTERRUPT_PIN.pin, gpio_interrupt);
    GPIO_PinModeSet(INTERRUPT_PIN.port, INTERRUPT_PIN.pin, gpioModeInputPullFilter, 0);
    GPIO_ExtIntConfig(INTERRUPT_PIN.port, INTERRUPT_PIN.pin, INTERRUPT_PIN.pin, true, false, true);
}

Ecode_t si91x_SPIDRV_MTransfer(SPIDRV_Handle_t handle, const void * txBuffer, void * rxBuffer, int count,
                               SPIDRV_Callback_t callback)
{
    USART_TypeDef * usart = handle->initData.port;
    uint8_t * tx          = (txBuffer != NULL) ? (uint8_t *) txBuffer : dummy_buffer;
    uint8_t * rx          = (rxBuffer != NULL) ? (uint8_t *) rxBuffer : dummy_buffer;

    // For transfers less than 16 bytes, directly interacting with USART buffers is faster than using DMA
    if (count < 16)
    {
        while (count > 0)
        {
            while (!(usart->STATUS & USART_STATUS_TXBL))
            {
            }
            usart->TXDATA = (uint32_t) *tx;
            while (!(usart->STATUS & USART_STATUS_TXC))
            {
            }
            *rx = (uint8_t) usart->RXDATA;
            if (txBuffer != NULL)
            {
                tx++;
            }
            if (rxBuffer != NULL)
            {
                rx++;
            }
            count--;
        }
        // callback(handle, ECODE_EMDRV_SPIDRV_OK, 0);
        return ECODE_EMDRV_SPIDRV_OK;
    }
    else
    {
        SPIDRV_MTransfer(handle, tx, rx, count, callback);
    }
    return ECODE_EMDRV_SPIDRV_BUSY;
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
    init_config.rx_irq      = config->rx_irq;
    init_config.rx_done     = config->rx_done;
    init_config.boot_option = config->boot_option;

    // Enable clock (not needed on xG21)
    CMU_ClockEnable(cmuClock_GPIO, true);

#if SL_SPICTRL_MUX
    spi_board_init();
#endif // SL_SPICTRL_MUX

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

    if (ECODE_EMDRV_SPIDRV_BUSY == si91x_SPIDRV_MTransfer(SPI_HANDLE, tx_buffer, rx_buffer, buffer_length, spi_dma_callback))
    {
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
