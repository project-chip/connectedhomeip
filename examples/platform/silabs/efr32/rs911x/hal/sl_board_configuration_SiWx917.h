#pragma once

#ifdef SL_UART
#include "sl_device_registers.h"
#include "sl_uart.h"

#define DEFAULT_UART USART0

#define SL_UART_VCOM_PORT SL_GPIO_PORT_D
#define SL_UART_VCOM_PIN 4
static const sl_gpio_t vcom_enable_pin = { SL_UART_VCOM_PORT, SL_UART_VCOM_PIN };

#define DEFAULT_UART_PIN_CONFIG &default_uart_pin_configuration

// XXX: HACK to get things working
#define UART_CLOCK cmuClock_USART0
#define UART_RX_IRQ USART0_RX_IRQn

/* Note: This creates a static instance for each C file that includes this header and references the variable */
static const sl_uart_pin_configuration_t default_uart_pin_configuration = {
    .tx_port     = SL_GPIO_PORT_A,
    .tx_pin      = 5,
    .rx_port     = SL_GPIO_PORT_A,
    .rx_pin      = 6,
    .cts_port    = SL_GPIO_PORT_A,
    .cts_pin     = 4,
    .rts_port    = SL_GPIO_PORT_C,
    .rts_pin     = 1,
    .uart_number = 0,
    //.route_loc = USART_ROUTELOC0_RXLOC_LOC4 | USART_ROUTELOC0_TXLOC_LOC4,
};
#endif

typedef struct
{
    unsigned char port;
    unsigned char pin;
} sl_pin_t;

#define PIN(port_id, pin_id)                                                                                                       \
    (sl_pin_t)                                                                                                                     \
    {                                                                                                                              \
        .port = gpioPort##port_id, .pin = pin_id                                                                                   \
    }

#define SLEEP_CONFIRM_PIN PIN(D, 2)
#define WAKE_INDICATOR_PIN PIN(A, 5)
#define RESET_PIN PIN(A, 6)
#define INTERRUPT_PIN PIN(A, 7)

#define SPI_CLOCK_PIN PIN(C, 3)
#define SPI_MOSI_PIN PIN(C, 1)
#define SPI_MISO_PIN PIN(C, 2)
#define SPI_CS_PIN PIN(C, 0)

#define SPI_USART USART0
#define SPI_USART_CMU_CLOCK cmuClock_USART0
#define SPI_USART_LDMA_TX ldmaPeripheralSignal_USART0_TXBL
#define SPI_USART_LDMA_RX ldmaPeripheralSignal_USART0_RXDATAV
#define SPI_USART_ROUTE_INDEX 0

#define PACKET_PENDING_INT_PRI 3
