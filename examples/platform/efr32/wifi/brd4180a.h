#ifndef _BRD4180A_H_
#define _BRD4180A_H_
typedef struct {
    unsigned char port;
    unsigned char pin;
} rsi_pin_t;

//#define CONCAT(a, b)      a ## b
//#define CONCAT3(a, b, c)  a ## b ## c
//#define CONCAT_EXPAND(a, b)      CONCAT(a,b)
//#define CONCAT3_EXPAND(a, b, c)  CONCAT3(a,b,c)

#define MY_USART USART2
#define MY_USART_CLOCK cmuClock_USART2
#define MY_USART_RX_SIGNAL dmadrvPeripheralSignal_USART2_TXBL
#define MY_USART_TX_SIGNAL dmadrvPeripheralSignal_USART2_RXDATAV

#define PIN(port_id, pin_id)  (rsi_pin_t){.port=gpioPort##port_id, .pin=pin_id}
#define SLEEP_CONFIRM_PIN   PIN(D, 2)
#ifndef LOGGING_STATS
#define WAKE_INDICATOR_PIN  PIN(D, 3)
#endif

#define RESET_PIN           PIN(B, 0)
#define INTERRUPT_PIN       PIN(B, 1)

#ifdef LOGGING_STATS
#define LOGGING_WAKE_INDICATOR_PIN  PIN(D, 3)
#define LOGGING_STATS_PORT    gpioPortD
#define LOGGING_STATS_PIN     03
#endif


#define SPI_CLOCK_PIN PIN(C, 2)
#define SPI_MOSI_PIN  PIN(C, 0)
#define SPI_MISO_PIN  PIN(C, 1)
#define SPI_CS_PIN    PIN(C, 3)


/* from sl_wfx_host_pinout.h */
#if BRD4180A

#define SL_WFX_HOST_PINOUT_RESET_PORT gpioPortD
#define SL_WFX_HOST_PINOUT_RESET_PIN 2

#define SL_WFX_HOST_PINOUT_SPI_TX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_TX_PIN 0
#define SL_WFX_HOST_PINOUT_SPI_TX_LOC 0

#define SL_WFX_HOST_PINOUT_SPI_RX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_RX_PIN 1
#define SL_WFX_HOST_PINOUT_SPI_RX_LOC 0

#define SL_WFX_HOST_PINOUT_SPI_CLK_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_CLK_PIN 2
#define SL_WFX_HOST_PINOUT_SPI_CLK_LOC 0

#define SL_WFX_HOST_PINOUT_SPI_CS_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_SPI_CS_PIN 3
#define SL_WFX_HOST_PINOUT_SPI_CS_LOC 0

#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL USART0
#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL_NO 0
#define SL_WFX_HOST_PINOUT_SPI_USART_CLK cmuClock_USART0
#define SL_WFX_HOST_PINOUT_SPI_RX_DMA_SIGNAL dmadrvPeripheralSignal_USART0_RXDATAV
#define SL_WFX_HOST_PINOUT_SPI_TX_DMA_SIGNAL dmadrvPeripheralSignal_USART0_TXBL

#define SL_WFX_HOST_PINOUT_WUP_PORT gpioPortD
#define SL_WFX_HOST_PINOUT_WUP_PIN 4

#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT gpioPortA /* SPI IRQ port*/
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN 5          /* SPI IRQ pin */
#define SL_WFX_HOST_PINOUT_SPI_IRQ 5
#endif /* 4180A */



#define RX_LDMA_CHANNEL   0
#define TX_LDMA_CHANNEL   1

#endif /* _BRD4180A_H_ */
