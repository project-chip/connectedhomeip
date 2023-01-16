/*
 * Copyright (c) 2013-2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Driver_USART.h"

#include "RTE_Device.h"
#include "cmsis_driver_config.h"

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg) (void) arg
#endif

/* Driver version */
#define ARM_USART_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(2, 2)

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = { ARM_USART_API_VERSION, ARM_USART_DRV_VERSION };

/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    0, /* Transmit completed event: \ref ARM_USARTx_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USARTx_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USARTx_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USARTx_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USARTx_EVENT_DCD */
    0, /* Signal RI change event: \ref ARM_USARTx_EVENT_RI */
    0  /* Reserved */
};

static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
    return DriverVersion;
}

static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
    return DriverCapabilities;
}

typedef struct
{
    struct uart_cmsdk_dev_t * dev;    /* UART device structure */
    uint32_t tx_nbr_bytes;            /* Number of bytes transferred */
    uint32_t rx_nbr_bytes;            /* Number of bytes recevied */
    ARM_USART_SignalEvent_t cb_event; /* Callback function for events */
} UARTx_Resources;

static int32_t ARM_USARTx_Initialize(UARTx_Resources * uart_dev)
{
    /* Initializes generic UART driver */
    uart_cmsdk_init(uart_dev->dev, PeripheralClock);

    return ARM_DRIVER_OK;
}

static int32_t ARM_USARTx_PowerControl(UARTx_Resources * uart_dev, ARM_POWER_STATE state)
{
    ARG_UNUSED(uart_dev);

    switch (state)
    {
    case ARM_POWER_OFF:
    case ARM_POWER_LOW:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    case ARM_POWER_FULL:
        /* Nothing to be done */
        return ARM_DRIVER_OK;
        /* default:  The default is not defined intentionally to force the
         *           compiler to check that all the enumeration values are
         *           covered in the switch.*/
    }
}

static int32_t ARM_USARTx_Send(UARTx_Resources * uart_dev, const void * data, uint32_t num)
{
    const uint8_t * p_data = (const uint8_t *) data;

    if ((data == NULL) || (num == 0U))
    {
        /* Invalid parameters */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Resets previous TX counter */
    uart_dev->tx_nbr_bytes = 0;

    while (uart_dev->tx_nbr_bytes != num)
    {
        /* Waits until UART is ready to transmit */
        while (!uart_cmsdk_tx_ready(uart_dev->dev))
        {
        };

        /* As UART is ready to transmit at this point, the write function can
         * not return any transmit error */
        (void) uart_cmsdk_write(uart_dev->dev, *p_data);

        uart_dev->tx_nbr_bytes++;
        p_data++;
    }

    if (uart_dev->cb_event != NULL)
    {
        uart_dev->cb_event(ARM_USART_EVENT_SEND_COMPLETE);
    }

    /* Waits until character is transmitted */
    while (!uart_cmsdk_tx_ready(uart_dev->dev))
    {
    };

    return ARM_DRIVER_OK;
}

static int32_t ARM_USARTx_Receive(UARTx_Resources * uart_dev, void * data, uint32_t num)
{
    uint8_t * p_data = (uint8_t *) data;

    if ((data == NULL) || (num == 0U))
    {
        // Invalid parameters
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* Resets previous RX counter */
    uart_dev->rx_nbr_bytes = 0;

    while (uart_dev->rx_nbr_bytes != num)
    {
        /* Waits until one character is received */
        while (!uart_cmsdk_rx_ready(uart_dev->dev))
        {
        };

        /* As UART has received one byte, the read can not
         * return any receive error at this point */
        (void) uart_cmsdk_read(uart_dev->dev, p_data);

        uart_dev->rx_nbr_bytes++;
        p_data++;
    }

    if (uart_dev->cb_event != NULL)
    {
        uart_dev->cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
    }

    return ARM_DRIVER_OK;
}

static uint32_t ARM_USARTx_GetTxCount(UARTx_Resources * uart_dev)
{
    return uart_dev->tx_nbr_bytes;
}

static uint32_t ARM_USARTx_GetRxCount(UARTx_Resources * uart_dev)
{
    return uart_dev->rx_nbr_bytes;
}

static int32_t ARM_USARTx_Control(UARTx_Resources * uart_dev, uint32_t control, uint32_t arg)
{
    switch (control & ARM_USART_CONTROL_Msk)
    {
#ifdef UART_TX_RX_CONTROL_ENABLED
    case ARM_USART_CONTROL_TX:
        if (arg == 0)
        {
            uart_cmsdk_tx_disable(uart_dev->dev);
        }
        else if (arg == 1)
        {
            if (uart_cmsdk_tx_enable(uart_dev->dev) != UART_CMSDK_ERR_NONE)
            {
                return ARM_DRIVER_ERROR;
            }
        }
        else
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
        break;
    case ARM_USART_CONTROL_RX:
        if (arg == 0)
        {
            uart_cmsdk_rx_disable(uart_dev->dev);
        }
        else if (arg == 1)
        {
            if (uart_cmsdk_rx_enable(uart_dev->dev) != UART_CMSDK_ERR_NONE)
            {
                return ARM_DRIVER_ERROR;
            }
        }
        else
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
        break;
#endif
    case ARM_USART_MODE_ASYNCHRONOUS:
        if (uart_cmsdk_set_baudrate(uart_dev->dev, arg) != UART_CMSDK_ERR_NONE)
        {
            return ARM_USART_ERROR_BAUDRATE;
        }
        break;
    /* Unsupported command */
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    /* UART Data bits */
    if (control & ARM_USART_DATA_BITS_Msk)
    {
        /* Data bit is not configurable */
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    /* UART Parity */
    if (control & ARM_USART_PARITY_Msk)
    {
        /* Parity is not configurable */
        return ARM_USART_ERROR_PARITY;
    }

    /* USART Stop bits */
    if (control & ARM_USART_STOP_BITS_Msk)
    {
        /* Stop bit is not configurable */
        return ARM_USART_ERROR_STOP_BITS;
    }

    return ARM_DRIVER_OK;
}

#if (RTE_USART0)
/* USART0 Driver wrapper functions */
static UARTx_Resources USART0_DEV = {
    .dev          = &UART0_CMSDK_DEV,
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event     = NULL,
};

static int32_t ARM_USART0_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    USART0_DEV.cb_event = cb_event;

    return ARM_USARTx_Initialize(&USART0_DEV);
}

static int32_t ARM_USART0_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART0_PowerControl(ARM_POWER_STATE state)
{
    return ARM_USARTx_PowerControl(&USART0_DEV, state);
}

static int32_t ARM_USART0_Send(const void * data, uint32_t num)
{
    return ARM_USARTx_Send(&USART0_DEV, data, num);
}

static int32_t ARM_USART0_Receive(void * data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART0_DEV, data, num);
}

static int32_t ARM_USART0_Transfer(const void * data_out, void * data_in, uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART0_GetTxCount(void)
{
    return ARM_USARTx_GetTxCount(&USART0_DEV);
}

static uint32_t ARM_USART0_GetRxCount(void)
{
    return ARM_USARTx_GetRxCount(&USART0_DEV);
}
static int32_t ARM_USART0_Control(uint32_t control, uint32_t arg)
{
    return ARM_USARTx_Control(&USART0_DEV, control, arg);
}

static ARM_USART_STATUS ARM_USART0_GetStatus(void)
{
    ARM_USART_STATUS status = { 0, 0, 0, 0, 0, 0, 0, 0 };
    return status;
}

static int32_t ARM_USART0_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART0_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = { 0, 0, 0, 0, 0 };
    return modem_status;
}

extern ARM_DRIVER_USART Driver_USART0;
ARM_DRIVER_USART Driver_USART0 = { ARM_USART_GetVersion,       ARM_USART_GetCapabilities, ARM_USART0_Initialize,
                                   ARM_USART0_Uninitialize,    ARM_USART0_PowerControl,   ARM_USART0_Send,
                                   ARM_USART0_Receive,         ARM_USART0_Transfer,       ARM_USART0_GetTxCount,
                                   ARM_USART0_GetRxCount,      ARM_USART0_Control,        ARM_USART0_GetStatus,
                                   ARM_USART0_SetModemControl, ARM_USART0_GetModemStatus };
#endif /* RTE_USART0 */

#if (RTE_USART1)
/* USART1 Driver wrapper functions */
static UARTx_Resources USART1_DEV = {
    .dev          = &UART1_CMSDK_DEV,
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event     = NULL,
};

static int32_t ARM_USART1_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    USART1_DEV.cb_event = cb_event;

    return ARM_USARTx_Initialize(&USART1_DEV);
}

static int32_t ARM_USART1_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART1_PowerControl(ARM_POWER_STATE state)
{
    return ARM_USARTx_PowerControl(&USART1_DEV, state);
}

static int32_t ARM_USART1_Send(const void * data, uint32_t num)
{
    return ARM_USARTx_Send(&USART1_DEV, data, num);
}

static int32_t ARM_USART1_Receive(void * data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART1_DEV, data, num);
}

static int32_t ARM_USART1_Transfer(const void * data_out, void * data_in, uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART1_GetTxCount(void)
{
    return ARM_USARTx_GetTxCount(&USART1_DEV);
}

static uint32_t ARM_USART1_GetRxCount(void)
{
    return ARM_USARTx_GetRxCount(&USART1_DEV);
}
static int32_t ARM_USART1_Control(uint32_t control, uint32_t arg)
{
    return ARM_USARTx_Control(&USART1_DEV, control, arg);
}

static ARM_USART_STATUS ARM_USART1_GetStatus(void)
{
    ARM_USART_STATUS status = { 0, 0, 0, 0, 0, 0, 0, 0 };
    return status;
}

static int32_t ARM_USART1_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART1_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = { 0, 0, 0, 0, 0 };
    return modem_status;
}

extern ARM_DRIVER_USART Driver_USART1;
ARM_DRIVER_USART Driver_USART1 = { ARM_USART_GetVersion,       ARM_USART_GetCapabilities, ARM_USART1_Initialize,
                                   ARM_USART1_Uninitialize,    ARM_USART1_PowerControl,   ARM_USART1_Send,
                                   ARM_USART1_Receive,         ARM_USART1_Transfer,       ARM_USART1_GetTxCount,
                                   ARM_USART1_GetRxCount,      ARM_USART1_Control,        ARM_USART1_GetStatus,
                                   ARM_USART1_SetModemControl, ARM_USART1_GetModemStatus };
#endif /* RTE_USART1 */

#if (RTE_USART2)
/* USART2 Driver wrapper functions */
static UARTx_Resources USART2_DEV = {
    .dev          = &UART2_CMSDK_DEV,
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event     = NULL,
};

static int32_t ARM_USART2_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    USART2_DEV.cb_event = cb_event;

    return ARM_USARTx_Initialize(&USART2_DEV);
}

static int32_t ARM_USART2_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART2_PowerControl(ARM_POWER_STATE state)
{
    return ARM_USARTx_PowerControl(&USART2_DEV, state);
}

static int32_t ARM_USART2_Send(const void * data, uint32_t num)
{
    return ARM_USARTx_Send(&USART2_DEV, data, num);
}

static int32_t ARM_USART2_Receive(void * data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART2_DEV, data, num);
}

static int32_t ARM_USART2_Transfer(const void * data_out, void * data_in, uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART2_GetTxCount(void)
{
    return ARM_USARTx_GetTxCount(&USART2_DEV);
}

static uint32_t ARM_USART2_GetRxCount(void)
{
    return ARM_USARTx_GetRxCount(&USART2_DEV);
}
static int32_t ARM_USART2_Control(uint32_t control, uint32_t arg)
{
    return ARM_USARTx_Control(&USART2_DEV, control, arg);
}

static ARM_USART_STATUS ARM_USART2_GetStatus(void)
{
    ARM_USART_STATUS status = { 0, 0, 0, 0, 0, 0, 0, 0 };
    return status;
}

static int32_t ARM_USART2_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART2_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = { 0, 0, 0, 0, 0 };
    return modem_status;
}

extern ARM_DRIVER_USART Driver_USART2;
ARM_DRIVER_USART Driver_USART2 = { ARM_USART_GetVersion,       ARM_USART_GetCapabilities, ARM_USART2_Initialize,
                                   ARM_USART2_Uninitialize,    ARM_USART2_PowerControl,   ARM_USART2_Send,
                                   ARM_USART2_Receive,         ARM_USART2_Transfer,       ARM_USART2_GetTxCount,
                                   ARM_USART2_GetRxCount,      ARM_USART2_Control,        ARM_USART2_GetStatus,
                                   ARM_USART2_SetModemControl, ARM_USART2_GetModemStatus };
#endif /* RTE_USART2 */

#if (RTE_USART3)
/* USART3 Driver wrapper functions */
static UARTx_Resources USART3_DEV = {
    .dev          = &UART3_CMSDK_DEV,
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event     = NULL,
};

static int32_t ARM_USART3_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    USART3_DEV.cb_event = cb_event;

    return ARM_USARTx_Initialize(&USART3_DEV);
}

static int32_t ARM_USART3_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART3_PowerControl(ARM_POWER_STATE state)
{
    return ARM_USARTx_PowerControl(&USART3_DEV, state);
}

static int32_t ARM_USART3_Send(const void * data, uint32_t num)
{
    return ARM_USARTx_Send(&USART3_DEV, data, num);
}

static int32_t ARM_USART3_Receive(void * data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART3_DEV, data, num);
}

static int32_t ARM_USART3_Transfer(const void * data_out, void * data_in, uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART3_GetTxCount(void)
{
    return ARM_USARTx_GetTxCount(&USART3_DEV);
}

static uint32_t ARM_USART3_GetRxCount(void)
{
    return ARM_USARTx_GetRxCount(&USART3_DEV);
}
static int32_t ARM_USART3_Control(uint32_t control, uint32_t arg)
{
    return ARM_USARTx_Control(&USART3_DEV, control, arg);
}

static ARM_USART_STATUS ARM_USART3_GetStatus(void)
{
    ARM_USART_STATUS status = { 0, 0, 0, 0, 0, 0, 0, 0 };
    return status;
}

static int32_t ARM_USART3_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART3_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = { 0, 0, 0, 0, 0 };
    return modem_status;
}

extern ARM_DRIVER_USART Driver_USART3;
ARM_DRIVER_USART Driver_USART3 = { ARM_USART_GetVersion,       ARM_USART_GetCapabilities, ARM_USART3_Initialize,
                                   ARM_USART3_Uninitialize,    ARM_USART3_PowerControl,   ARM_USART3_Send,
                                   ARM_USART3_Receive,         ARM_USART3_Transfer,       ARM_USART3_GetTxCount,
                                   ARM_USART3_GetRxCount,      ARM_USART3_Control,        ARM_USART3_GetStatus,
                                   ARM_USART3_SetModemControl, ARM_USART3_GetModemStatus };
#endif /* RTE_USART3 */

#if (RTE_USART4)
/* USART4 Driver wrapper functions */
static UARTx_Resources USART4_DEV = {
    .dev          = &UART4_CMSDK_DEV,
    .tx_nbr_bytes = 0,
    .rx_nbr_bytes = 0,
    .cb_event     = NULL,
};

static int32_t ARM_USART4_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    USART4_DEV.cb_event = cb_event;

    return ARM_USARTx_Initialize(&USART4_DEV);
}

static int32_t ARM_USART4_Uninitialize(void)
{
    /* Nothing to be done */
    return ARM_DRIVER_OK;
}

static int32_t ARM_USART4_PowerControl(ARM_POWER_STATE state)
{
    return ARM_USARTx_PowerControl(&USART4_DEV, state);
}

static int32_t ARM_USART4_Send(const void * data, uint32_t num)
{
    return ARM_USARTx_Send(&USART4_DEV, data, num);
}

static int32_t ARM_USART4_Receive(void * data, uint32_t num)
{
    return ARM_USARTx_Receive(&USART4_DEV, data, num);
}

static int32_t ARM_USART4_Transfer(const void * data_out, void * data_in, uint32_t num)
{
    ARG_UNUSED(data_out);
    ARG_UNUSED(data_in);
    ARG_UNUSED(num);

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART4_GetTxCount(void)
{
    return ARM_USARTx_GetTxCount(&USART4_DEV);
}

static uint32_t ARM_USART4_GetRxCount(void)
{
    return ARM_USARTx_GetRxCount(&USART4_DEV);
}
static int32_t ARM_USART4_Control(uint32_t control, uint32_t arg)
{
    return ARM_USARTx_Control(&USART4_DEV, control, arg);
}

static ARM_USART_STATUS ARM_USART4_GetStatus(void)
{
    ARM_USART_STATUS status = { 0, 0, 0, 0, 0, 0, 0, 0 };
    return status;
}

static int32_t ARM_USART4_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    ARG_UNUSED(control);
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART4_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS modem_status = { 0, 0, 0, 0, 0 };
    return modem_status;
}

extern ARM_DRIVER_USART Driver_USART4;
ARM_DRIVER_USART Driver_USART4 = { ARM_USART_GetVersion,       ARM_USART_GetCapabilities, ARM_USART4_Initialize,
                                   ARM_USART4_Uninitialize,    ARM_USART4_PowerControl,   ARM_USART4_Send,
                                   ARM_USART4_Receive,         ARM_USART4_Transfer,       ARM_USART4_GetTxCount,
                                   ARM_USART4_GetRxCount,      ARM_USART4_Control,        ARM_USART4_GetStatus,
                                   ARM_USART4_SetModemControl, ARM_USART4_GetModemStatus };
#endif /* RTE_USART4 */
