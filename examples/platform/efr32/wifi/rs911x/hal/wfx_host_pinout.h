/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/***************************************************************************
 * @file
 * @brief WFX host configuration and pinout
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in
 *compliance with the License. You may obtain a copy
 *of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in
 *writing, software distributed under the License is
 *distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 *OR CONDITIONS OF ANY KIND, either express or
 *implied. See the License for the specific language
 *governing permissions and limitations under the
 *License.
 *****************************************************************************/

#ifndef SL_WFX_HOST_PINOUT_H
#define SL_WFX_HOST_PINOUT_H

#define FRAME_RX_EVENT DEF_BIT_00
#define MAX_RX_QUEUE_SIZE 4

#if BRD4166A
#error "Board selected do not support WFX extension board"
#endif

#if (BRD4161A || BRD4163A || BRD4164A)

#define SL_WFX_HOST_PINOUT_SPI_TX_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_SPI_TX_PIN 6
#define SL_WFX_HOST_PINOUT_SPI_TX_LOC 1

#define SL_WFX_HOST_PINOUT_SPI_RX_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_SPI_RX_PIN 7
#define SL_WFX_HOST_PINOUT_SPI_RX_LOC 1

#define SL_WFX_HOST_PINOUT_SPI_CLK_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_SPI_CLK_PIN 8
#define SL_WFX_HOST_PINOUT_SPI_CLK_LOC 1

#define SL_WFX_HOST_PINOUT_SPI_CS_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_SPI_CS_PIN 9
#define SL_WFX_HOST_PINOUT_SPI_CS_LOC 1

#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL USART2
#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL_NO 2
#define SL_WFX_HOST_PINOUT_SPI_USART_CLK cmuClock_USART2
#define SL_WFX_HOST_PINOUT_SPI_RX_DMA_SIGNAL dmadrvPeripheralSignal_USART2_RXDATAV
#define SL_WFX_HOST_PINOUT_SPI_TX_DMA_SIGNAL dmadrvPeripheralSignal_USART2_TXBL

#define SL_WFX_HOST_PINOUT_WUP_PORT gpioPortD
#define SL_WFX_HOST_PINOUT_WUP_PIN 8

#ifdef RS911X_WIFI
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT gpioPortC /* SPI IRQ port*/
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN 9          /* SPI IRQ pin */
#define SL_WFX_HOST_PINOUT_RESET_PORT gpioPortD
#define SL_WFX_HOST_PINOUT_RESET_PIN 12
#else
#define SL_WFX_HOST_PINOUT_RESET_PORT gpioPortD
#define SL_WFX_HOST_PINOUT_RESET_PIN 10
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT gpioPortB /* SPI IRQ port*/
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN 6          /* SPI IRQ pin */
#endif

#if BRD4170A
#define SL_WFX_HOST_PINOUT_RESET_PORT gpioPortD
#define SL_WFX_HOST_PINOUT_RESET_PIN 10

#define SL_WFX_HOST_PINOUT_SPI_TX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_TX_PIN 0
#define SL_WFX_HOST_PINOUT_SPI_TX_LOC 18

#define SL_WFX_HOST_PINOUT_SPI_RX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_RX_PIN 1
#define SL_WFX_HOST_PINOUT_SPI_RX_LOC 18

#define SL_WFX_HOST_PINOUT_SPI_CLK_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_CLK_PIN 2
#define SL_WFX_HOST_PINOUT_SPI_CLK_LOC 18

#define SL_WFX_HOST_PINOUT_SPI_CS_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_SPI_CS_PIN 3
#define SL_WFX_HOST_PINOUT_SPI_CS_LOC 18

#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL USART3
#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL_NO 3
#define SL_WFX_HOST_PINOUT_SPI_USART_CLK cmuClock_USART3
#define SL_WFX_HOST_PINOUT_SPI_RX_DMA_SIGNAL dmadrvPeripheralSignal_USART3_RXDATAV
#define SL_WFX_HOST_PINOUT_SPI_TX_DMA_SIGNAL dmadrvPeripheralSignal_USART3_TXBL

#define SL_WFX_HOST_PINOUT_WUP_PORT gpioPortD
#define SL_WFX_HOST_PINOUT_WUP_PIN 8

#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT gpioPortC /* SPI IRQ port*/
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN 4          /* SPI IRQ pin */
#define SL_WFX_HOST_PINOUT_SPI_IRQ 5
#endif

#if BRD4304A
#define SL_WFX_HOST_PINOUT_RESET_PORT gpioPortF
#define SL_WFX_HOST_PINOUT_RESET_PIN 4

#define SL_WFX_HOST_PINOUT_SPI_TX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_TX_PIN 6
#define SL_WFX_HOST_PINOUT_SPI_TX_LOC 11

#define SL_WFX_HOST_PINOUT_SPI_RX_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_RX_PIN 7
#define SL_WFX_HOST_PINOUT_SPI_RX_LOC 11

#define SL_WFX_HOST_PINOUT_SPI_CLK_PORT gpioPortC
#define SL_WFX_HOST_PINOUT_SPI_CLK_PIN 8
#define SL_WFX_HOST_PINOUT_SPI_CLK_LOC 11

#define SL_WFX_HOST_PINOUT_SPI_CS_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_SPI_CS_PIN 9
#define SL_WFX_HOST_PINOUT_SPI_CS_LOC 11

#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL USART1
#define SL_WFX_HOST_PINOUT_SPI_PERIPHERAL_NO 1
#define SL_WFX_HOST_PINOUT_SPI_USART_CLK cmuClock_USART1
#define SL_WFX_HOST_PINOUT_SPI_RX_DMA_SIGNAL dmadrvPeripheralSignal_USART1_RXDATAV
#define SL_WFX_HOST_PINOUT_SPI_TX_DMA_SIGNAL dmadrvPeripheralSignal_USART1_TXBL

#define SL_WFX_HOST_PINOUT_WUP_PORT gpioPortA
#define SL_WFX_HOST_PINOUT_WUP_PIN 2

#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PORT gpioPortA /* SPI IRQ port*/
#define SL_WFX_HOST_PINOUT_SPI_WIRQ_PIN 0          /* SPI IRQ pin */
#define SL_WFX_HOST_PINOUT_SPI_IRQ 5
#endif

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
#endif

#endif // SL_WFX_HOST_PINOUT_H
