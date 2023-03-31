/*
 * Copyright (c) 2019-2021 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLATFORM_IRQ_H__
#define __PLATFORM_IRQ_H__

typedef enum _IRQn_Type
{
    NonMaskableInt_IRQn            = -14, /* Non Maskable Interrupt */
    HardFault_IRQn                 = -13, /* HardFault Interrupt */
    MemoryManagement_IRQn          = -12, /* Memory Management Interrupt */
    BusFault_IRQn                  = -11, /* Bus Fault Interrupt */
    UsageFault_IRQn                = -10, /* Usage Fault Interrupt */
    SecureFault_IRQn               = -9,  /* Secure Fault Interrupt */
    SVCall_IRQn                    = -5,  /* SV Call Interrupt */
    DebugMonitor_IRQn              = -4,  /* Debug Monitor Interrupt */
    PendSV_IRQn                    = -2,  /* Pend SV Interrupt */
    SysTick_IRQn                   = -1,  /* System Tick Interrupt */
    NONSEC_WATCHDOG_RESET_REQ_IRQn = 0,   /* Non-Secure Watchdog Reset
                                           * Request Interrupt
                                           */
    NONSEC_WATCHDOG_IRQn = 1,             /* Non-Secure Watchdog Interrupt */
    SLOWCLK_TIMER_IRQn   = 2,             /* SLOWCLK Timer Interrupt */
    TIMER0_IRQn          = 3,             /* TIMER 0 Interrupt */
    TIMER1_IRQn          = 4,             /* TIMER 1 Interrupt */
    TIMER2_IRQn          = 5,             /* TIMER 2 Interrupt */
    /* Reserved                        = 6,       Reserved */
    /* Reserved                        = 7,       Reserved */
    /* Reserved                        = 8,       Reserved */
    MPC_IRQn          = 9,  /* MPC Combined (Secure) Interrupt */
    PPC_IRQn          = 10, /* PPC Combined (Secure) Interrupt */
    MSC_IRQn          = 11, /* MSC Combined (Secure) Interrput */
    BRIDGE_ERROR_IRQn = 12, /* Bridge Error Combined
                             * (Secure) Interrupt
                             */
    /* Reserved                        = 13,      Reserved */
    MGMT_PPU_IRQn = 14, /* MGMT PPU */
    SYS_PPU_IRQn  = 15, /* SYS PPU */
    CPU0_PPU_IRQn = 16, /* CPU0 PPU */
    /* Reserved                        = 17,      Reserved */
    /* Reserved                        = 18,      Reserved */
    /* Reserved                        = 19,      Reserved */
    /* Reserved                        = 20,      Reserved */
    /* Reserved                        = 21,      Reserved */
    /* Reserved                        = 22,      Reserved */
    /* Reserved                        = 23,      Reserved */
    /* Reserved                        = 24,      Reserved */
    /* Reserved                        = 25,      Reserved */
    DEBUG_PPU_IRQn  = 26, /* DEBUG PPU */
    TIMER3_AON_IRQn = 27, /* TIMER 3 AON Interrupt */
    CPU0_CTI_0_IRQn = 28, /* CPU0 CTI IRQ 0 */
    CPU0_CTI_1_IRQn = 29, /* CPU0 CTI IRQ 1 */
    /* Reserved                        = 30,      Reserved */
    /* Reserved                        = 31,      Reserved */
    System_Timestamp_Counter_IRQn = 32, /* System timestamp counter Interrupt */
    UARTRX0_IRQn                  = 33, /* UART 0 RX Interrupt */
    UARTTX0_IRQn                  = 34, /* UART 0 TX Interrupt */
    UARTRX1_IRQn                  = 35, /* UART 1 RX Interrupt */
    UARTTX1_IRQn                  = 36, /* UART 1 TX Interrupt */
    UARTRX2_IRQn                  = 37, /* UART 2 RX Interrupt */
    UARTTX2_IRQn                  = 38, /* UART 2 TX Interrupt */
    UARTRX3_IRQn                  = 39, /* UART 3 RX Interrupt */
    UARTTX3_IRQn                  = 40, /* UART 3 TX Interrupt */
    UARTRX4_IRQn                  = 41, /* UART 4 RX Interrupt */
    UARTTX4_IRQn                  = 42, /* UART 4 TX Interrupt */
    UART0_Combined_IRQn           = 43, /* UART 0 Combined Interrupt */
    UART1_Combined_IRQn           = 44, /* UART 1 Combined Interrupt */
    UART2_Combined_IRQn           = 45, /* UART 2 Combined Interrupt */
    UART3_Combined_IRQn           = 46, /* UART 3 Combined Interrupt */
    UART4_Combined_IRQn           = 47, /* UART 4 Combined Interrupt */
    UARTOVF_IRQn                  = 48, /* UART 0, 1, 2, 3, 4 & 5 Overflow Interrupt */
    ETHERNET_IRQn                 = 49, /* Ethernet Interrupt */
    I2S_IRQn                      = 50, /* Audio I2S Interrupt */
    TOUCH_SCREEN_IRQn             = 51, /* Touch Screen Interrupt */
    USB_IRQn                      = 52, /* USB Interrupt */
    SPI_ADC_IRQn                  = 53, /* SPI ADC Interrupt */
    SPI_SHIELD0_IRQn              = 54, /* SPI (Shield 0) Interrupt */
    SPI_SHIELD1_IRQn              = 55, /* SPI (Shield 1) Interrupt */
    ETHOS_U55_IRQn                = 56, /* Ethos-U55 Interrupt */
    /* Reserved                        = 57:68    Reserved */
    GPIO0_Combined_IRQn = 69, /* GPIO 0 Combined Interrupt */
    GPIO1_Combined_IRQn = 70, /* GPIO 1 Combined Interrupt */
    GPIO2_Combined_IRQn = 71, /* GPIO 2 Combined Interrupt */
    GPIO3_Combined_IRQn = 72, /* GPIO 3 Combined Interrupt */
    GPIO0_0_IRQn        = 73, /* GPIO0 has 16 pins with IRQs */
    GPIO0_1_IRQn        = 74,
    GPIO0_2_IRQn        = 75,
    GPIO0_3_IRQn        = 76,
    GPIO0_4_IRQn        = 77,
    GPIO0_5_IRQn        = 78,
    GPIO0_6_IRQn        = 79,
    GPIO0_7_IRQn        = 80,
    GPIO0_8_IRQn        = 81,
    GPIO0_9_IRQn        = 82,
    GPIO0_10_IRQn       = 83,
    GPIO0_11_IRQn       = 84,
    GPIO0_12_IRQn       = 85,
    GPIO0_13_IRQn       = 86,
    GPIO0_14_IRQn       = 87,
    GPIO0_15_IRQn       = 88,
    GPIO1_0_IRQn        = 89, /* GPIO1 has 16 pins with IRQs */
    GPIO1_1_IRQn        = 90,
    GPIO1_2_IRQn        = 91,
    GPIO1_3_IRQn        = 92,
    GPIO1_4_IRQn        = 93,
    GPIO1_5_IRQn        = 94,
    GPIO1_6_IRQn        = 95,
    GPIO1_7_IRQn        = 96,
    GPIO1_8_IRQn        = 97,
    GPIO1_9_IRQn        = 98,
    GPIO1_10_IRQn       = 99,
    GPIO1_11_IRQn       = 100,
    GPIO1_12_IRQn       = 101,
    GPIO1_13_IRQn       = 102,
    GPIO1_14_IRQn       = 103,
    GPIO1_15_IRQn       = 104,
    GPIO2_0_IRQn        = 105, /* GPIO2 has 16 pins with IRQs */
    GPIO2_1_IRQn        = 106,
    GPIO2_2_IRQn        = 107,
    GPIO2_3_IRQn        = 108,
    GPIO2_4_IRQn        = 109,
    GPIO2_5_IRQn        = 110,
    GPIO2_6_IRQn        = 111,
    GPIO2_7_IRQn        = 112,
    GPIO2_8_IRQn        = 113,
    GPIO2_9_IRQn        = 114,
    GPIO2_10_IRQn       = 115,
    GPIO2_11_IRQn       = 116,
    GPIO2_12_IRQn       = 117,
    GPIO2_13_IRQn       = 118,
    GPIO2_14_IRQn       = 119,
    GPIO2_15_IRQn       = 120,
    GPIO3_0_IRQn        = 121, /* GPIO3 has 4 pins with IRQs */
    GPIO3_1_IRQn        = 122,
    GPIO3_2_IRQn        = 123,
    GPIO3_3_IRQn        = 124,
    UARTRX5_IRQn        = 125, /* UART 5 RX Interrupt */
    UARTTX5_IRQn        = 126, /* UART 5 TX Interrupt */
    UART5_IRQn          = 127, /* UART 5 combined Interrupt */
    /* Reserved                        = 128:130   Reserved */
} IRQn_Type;

#endif /* __PLATFORM_IRQ_H__ */
