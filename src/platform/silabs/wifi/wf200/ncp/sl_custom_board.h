/*
 * This file is used to set the pins for the SPI for Custom boards
 * The SPI pins are defined in the file
 *
 *  !!!! MODIFY THIS FILE TO THE CORRECT PINS !!!!
 */

#ifndef _CUSTOM_BOARD_H_
#define _CUSTOM_BOARD_H_
/* WF200 */

#define PIN_OUT_SET 1
#define PIN_OUT_CLEAR 0

#define MY_USART USART0
#define MY_USART_CLOCK cmuClock_USART0
#define MY_USART_TX_SIGNAL dmadrvPeripheralSignal_USART0_TXBL
#define MY_USART_RX_SIGNAL dmadrvPeripheralSignal_USART0_RXDATAV

#define SL_WFX_HOST_PINOUT_RESET_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_RESET_PIN 5
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT gpioPortA /* SPI IRQ port */
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN 8          /* SPI IRQ pin */
#define SL_WFX_HOST_PINOUT_WUP_PORT gpioPortB
#define SL_WFX_HOST_PINOUT_WUP_PIN 5

#define SL_WFX_HOST_PINOUT_SPI_TX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_TX_PIN 1
#define SL_WFX_HOST_PINOUT_SPI_TX_LOC 1

#define SL_WFX_HOST_PINOUT_SPI_RX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_RX_PIN 2
#define SL_WFX_HOST_PINOUT_SPI_RX_LOC 1

#define SL_WFX_HOST_PINOUT_SPI_CLK_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_CLK_PIN 3
#define SL_WFX_HOST_PINOUT_SPI_CLK_LOC 1

#define SL_WFX_HOST_PINOUT_SPI_CS_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_CS_PIN 0
#define SL_WFX_HOST_PINOUT_SPI_CS_LOC 1

/* WF200 */
#endif /* _CUSTOM_BOARD_H_ */
