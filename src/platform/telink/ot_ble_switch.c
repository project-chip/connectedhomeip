/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include "ble_common.h"
#include "controller/ll/ll.h"
#include "openthread-system.h"
#include <openthread/tasklet.h>

#define OT_AFTER_TIME (16 * 1000 * 5) // 4ms
#define BLE_IDLE_TIME (16 * 1000 * 5) // 5ms
#define APP_BLE_STATE_SET(state) g_bleState = state
#define APP_BLE_STATE_GET() g_bleState
#define APP_OT_RX_FIFO_SIZE 144
#define APP_OT_RX_BUF_SIZE 160

#define LOGICCHANNEL_TO_PHYSICAL(p) (((p) -10) * 5)

#define OT_RADIO_RX_BUF_CLEAR(p)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        p[0] = 0;                                                                                                                  \
        p[4] = 0;                                                                                                                  \
    } while (0)

extern u8 g_bleState;
extern unsigned char current_channel;
extern void rf_set_channel(uint8_t channel);

u8 g_ble_txPowerSet                                                     = RF_POWER_INDEX_P3p25dBm;
static u32 g_bleTaskTick                                                = 0;
volatile u8 ot_process                                                  = 0;
u8 g_bleState                                                           = BLS_LINK_STATE_IDLE;
unsigned char rx_buffer[APP_OT_RX_BUF_SIZE] __attribute__((aligned(4))) = { 0 };
unsigned char current_channel;

void rf_set_channel(uint8_t channel)
{
    current_channel = channel;
    rf_set_chn(LOGICCHANNEL_TO_PHYSICAL(channel));
}

static void restore_ot_rf_context(void)
{
    rf_baseband_reset();

    OT_RADIO_RX_BUF_CLEAR(rx_buffer);
    CLEAR_ALL_RFIRQ_STATUS;
    rf_clr_irq_mask(FLD_RF_IRQ_ALL); // the uart interrupt should always be opened.

    rf_mode_init();
    rf_set_zigbee_250K_mode();

    rf_set_power_level(RF_POWER_P9p11dBm);

    rf_set_rx_maxlen(APP_OT_RX_FIFO_SIZE);
    rf_set_rx_dma_config();
    rf_set_rx_dma_fifo_num(0);
    rf_set_rx_dma_fifo_size(APP_OT_RX_FIFO_SIZE);
    rf_set_tx_dma_config();
    rf_set_tx_dma_fifo_num(1);
    rf_set_tx_dma_fifo_size(128);
    rf_set_irq_mask(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX);
    plic_interrupt_enable(IRQ15_ZB_RT);

    rf_set_rx_buffer(rx_buffer);

    rf_set_channel(current_channel);

    rf_set_rxmode();
    delay_us(85);
}

__attribute__((section(".ram_code"))) __attribute__((noinline)) static void switch_to_ot_context(void)
{
    dma_chn_dis(DMA1); // disable the rx

    backup_ble_rf_context();

    restore_ot_rf_context();
    // switch tx power for ot mode
    rf_set_power_level(RF_POWER_P9p11dBm);

    dma_chn_en(DMA1);

    ot_process = 1;
}

__attribute__((section(".ram_code"))) __attribute__((noinline)) static void switch_to_ble_context(void)
{
    /* disable zb rx dma to avoid un-excepted rx interrupt*/
    dma_chn_dis(DMA1);

    restore_ble_rf_context();
    /* switch tx power for ble mode */
    rf_set_power_level_index(RF_POWER_INDEX_P3p25dBm);

    dma_chn_en(DMA1);

    ot_process = 0;
}

static int is_switch_to_ble(void)
{
    if ((get_ble_next_event_tick() - (clock_time() + OT_AFTER_TIME)) > (unsigned int) BIT(31))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int is_switch_to_ot(void)
{
    if ((get_ble_next_event_tick() - (clock_time() + BLE_IDLE_TIME)) < (unsigned int) BIT(31))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void ot_task(otInstance * instance)
{
    otTaskletsProcess(instance);
    otSysProcessDrivers(instance);
}

extern bool ot_rfTxPktDoing();

static void ot_ble_switch_proc(otInstance * instance)
{
    u32 r = 0;

    APP_BLE_STATE_SET(bltParam.blt_state);

    if (ot_process == 0)
    {
        g_bleTaskTick = clock_time();

        blt_sdk_main_loop();

        r = core_interrupt_disable();

        if (((get_ble_event_state() && is_switch_to_ot()) || APP_BLE_STATE_GET() == BLS_LINK_STATE_IDLE))
        {
            switch_to_ot_context();

            core_restore_interrupt(r);
            ot_task(instance);
        }
        else
        {
            core_restore_interrupt(r);
        }
    }
    else
    {
        r = core_interrupt_disable();

        if (!ot_rfTxPktDoing() && is_switch_to_ble() && APP_BLE_STATE_GET() != BLS_LINK_STATE_IDLE)
        {
            switch_to_ble_context();

            core_restore_interrupt(r);
            return;
        }

        core_restore_interrupt(r);
        ot_task(instance);
    }
}

void concurrent_mode_main_loop(otInstance * instance)
{
    ot_ble_switch_proc(instance);
}

void cuncurrent_mode_init(void)
{
    ble_radio_init();
}
