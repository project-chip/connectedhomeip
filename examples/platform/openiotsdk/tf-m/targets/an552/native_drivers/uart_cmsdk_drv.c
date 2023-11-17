/*
 * Copyright (c) 2016-2022 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uart_cmsdk_drv.h"

#include <stddef.h>

/* UART register map structure */
struct _uart_cmsdk_reg_map_t
{
    volatile uint32_t data;  /* Offset: 0x000 (R/W) data register    */
    volatile uint32_t state; /* Offset: 0x004 (R/W) status register  */
    volatile uint32_t ctrl;  /* Offset: 0x008 (R/W) control register */
    union
    {
        volatile uint32_t intrstatus; /* Offset: 0x00c (R/ ) interrupt status
                                       *                     register
                                       */
        volatile uint32_t intrclear;  /* Offset: 0x00c ( /W) interrupt clear
                                       *                     register
                                       */
    } intr_reg;
    volatile uint32_t bauddiv; /* Offset: 0x010 (R/W) Baudrate divider
                                *                     register
                                */
};

/* CTRL Register */
#define UART_CMSDK_TX_EN (1ul << 0)
#define UART_CMSDK_RX_EN (1ul << 1)
#define UART_CMSDK_TX_INTR_EN (1ul << 2)
#define UART_CMSDK_RX_INTR_EN (1ul << 3)

/* STATE Register */
#define UART_CMSDK_TX_BF (1ul << 0)
#define UART_CMSDK_RX_BF (1ul << 1)

/* INTSTATUS Register */
#define UART_CMSDK_TX_INTR (1ul << 0)
#define UART_CMSDK_RX_INTR (1ul << 1)

/* UART state definitions */
#define UART_CMSDK_INITIALIZED (1ul << 0)

enum uart_cmsdk_error_t uart_cmsdk_init(struct uart_cmsdk_dev_t * dev, uint32_t system_clk)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;
    if (system_clk == 0)
    {
        return UART_CMSDK_ERR_INVALID_ARG;
    }

    /* Sets baudrate and system clock */
    dev->data->system_clk = system_clk;
    dev->data->baudrate   = dev->cfg->default_baudrate;

    /* Sets baudrate */
    p_uart->bauddiv = (dev->data->system_clk / dev->cfg->default_baudrate);

    /* Enables receiver and transmitter */
    p_uart->ctrl = UART_CMSDK_RX_EN | UART_CMSDK_TX_EN;

    dev->data->state = UART_CMSDK_INITIALIZED;

    return UART_CMSDK_ERR_NONE;
}

enum uart_cmsdk_error_t uart_cmsdk_set_baudrate(struct uart_cmsdk_dev_t * dev, uint32_t baudrate)
{
    uint32_t bauddiv;
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (baudrate == 0)
    {
        return UART_CMSDK_ERR_INVALID_BAUD;
    }

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return UART_CMSDK_ERR_NOT_INIT;
    }

    /* Sets baudrate */
    bauddiv             = (dev->data->system_clk / baudrate);
    dev->data->baudrate = baudrate;

    /* Minimum bauddiv value */
    if (bauddiv < 16)
    {
        return UART_CMSDK_ERR_INVALID_BAUD;
    }

    p_uart->bauddiv = bauddiv;

    return UART_CMSDK_ERR_NONE;
}

uint32_t uart_cmsdk_get_baudrate(struct uart_cmsdk_dev_t * dev)
{
    return dev->data->baudrate;
}

enum uart_cmsdk_error_t uart_cmsdk_set_clock(struct uart_cmsdk_dev_t * dev, uint32_t system_clk)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (system_clk == 0)
    {
        return UART_CMSDK_ERR_INVALID_ARG;
    }

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return UART_CMSDK_ERR_NOT_INIT;
    }

    /* Sets system clock */
    dev->data->system_clk = system_clk;

    /* Updates baudrate divider */
    p_uart->bauddiv = (dev->data->system_clk / dev->data->baudrate);

    /* Enables receiver and transmitter */
    return UART_CMSDK_ERR_NONE;
}

enum uart_cmsdk_error_t uart_cmsdk_read(struct uart_cmsdk_dev_t * dev, uint8_t * byte)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (!(p_uart->state & UART_CMSDK_RX_BF))
    {
        return UART_CMSDK_ERR_NOT_READY;
    }

    /* Reads data */
    *byte = (uint8_t) p_uart->data;

    return UART_CMSDK_ERR_NONE;
}

enum uart_cmsdk_error_t uart_cmsdk_write(struct uart_cmsdk_dev_t * dev, uint8_t byte)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (p_uart->state & UART_CMSDK_TX_BF)
    {
        return UART_CMSDK_ERR_NOT_READY;
    }

    /* Sends data */
    p_uart->data = byte;

    return UART_CMSDK_ERR_NONE;
}

enum uart_cmsdk_error_t uart_cmsdk_irq_tx_enable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return UART_CMSDK_ERR_NOT_INIT;
    }

    p_uart->ctrl |= UART_CMSDK_TX_INTR_EN;

    return UART_CMSDK_ERR_NONE;
}

void uart_cmsdk_irq_tx_disable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (dev->data->state & UART_CMSDK_INITIALIZED)
    {
        p_uart->ctrl &= ~UART_CMSDK_TX_INTR_EN;
    }
}

uint32_t uart_cmsdk_tx_ready(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return 0;
    }

    return !(p_uart->state & UART_CMSDK_TX_BF);
}

enum uart_cmsdk_error_t uart_cmsdk_irq_rx_enable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return UART_CMSDK_ERR_NOT_INIT;
    }

    p_uart->ctrl |= UART_CMSDK_RX_INTR_EN;

    return UART_CMSDK_ERR_NONE;
}

void uart_cmsdk_irq_rx_disable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (dev->data->state & UART_CMSDK_INITIALIZED)
    {
        p_uart->ctrl &= ~UART_CMSDK_RX_INTR_EN;
    }
}

uint32_t uart_cmsdk_rx_ready(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return 0;
    }

    return (p_uart->state & UART_CMSDK_RX_BF);
}

void uart_cmsdk_clear_interrupt(struct uart_cmsdk_dev_t * dev, enum uart_cmsdk_irq_t irq)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (dev->data->state & UART_CMSDK_INITIALIZED)
    {
        /* Clears pending interrupts */
        switch (irq)
        {
        case UART_CMSDK_IRQ_RX:
            p_uart->intr_reg.intrclear = UART_CMSDK_RX_INTR;
            break;
        case UART_CMSDK_IRQ_TX:
            p_uart->intr_reg.intrclear = UART_CMSDK_TX_INTR;
            break;
        case UART_CMSDK_IRQ_COMBINED:
            p_uart->intr_reg.intrclear = (UART_CMSDK_RX_INTR | UART_CMSDK_TX_INTR);
            break;
            /* default: not defined to force all cases to be handled */
        }
    }
}

enum uart_cmsdk_error_t uart_cmsdk_tx_enable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return UART_CMSDK_ERR_NOT_INIT;
    }

    p_uart->ctrl |= UART_CMSDK_TX_EN;

    return UART_CMSDK_ERR_NONE;
}

void uart_cmsdk_tx_disable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (dev->data->state & UART_CMSDK_INITIALIZED)
    {
        p_uart->ctrl &= ~UART_CMSDK_TX_EN;
    }
}

enum uart_cmsdk_error_t uart_cmsdk_rx_enable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (!(dev->data->state & UART_CMSDK_INITIALIZED))
    {
        return UART_CMSDK_ERR_NOT_INIT;
    }

    p_uart->ctrl |= UART_CMSDK_RX_EN;

    return UART_CMSDK_ERR_NONE;
}

void uart_cmsdk_rx_disable(struct uart_cmsdk_dev_t * dev)
{
    struct _uart_cmsdk_reg_map_t * p_uart = (struct _uart_cmsdk_reg_map_t *) dev->cfg->base;

    if (dev->data->state & UART_CMSDK_INITIALIZED)
    {
        p_uart->ctrl &= ~UART_CMSDK_RX_EN;
    }
}
