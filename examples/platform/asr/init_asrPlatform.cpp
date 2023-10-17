/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      This file provides the function to initialize the ASR platform.
 */

#include "AppConfig.h"
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "lega_wlan_api.h"
#include "soc_init.h"
#ifdef CFG_PLF_RV32
#include "asr_flash_kv.h"
#include "asr_pinmux.h"
#include "asr_sec_hw_common.h"
#include "asr_uart.h"
#elif defined CFG_PLF_DUET
#include "duet_flash_kv.h"
#include "duet_pinmux.h"
#include "duet_uart.h"
#include "duet_version.h"
#else
#include "lega_flash_kv.h"
#include "lega_pinmux.h"
#include "lega_uart.h"
#include "lega_version.h"
#endif
#include "lega_ota_utils.h"
#include "lega_rtos_api.h"
#if defined(CFG_PLF_RV32) || defined(CFG_PLF_DUET)
#include "printf_uart.h"
#endif
#include "tcpip.h"
#if (CFG_EASY_LOG_ENABLE == 1)
#include "elog.h"
#include "elog_cfg.h"
#endif

#ifdef CFG_PLF_RV32
#define LEGA_UART0_INDEX UART0_INDEX
#define LEGA_UART1_INDEX UART1_INDEX
#define LEGA_UART2_INDEX UART2_INDEX
#define duet_pinmux_config asr_pinmux_config
#define duet_uart_init printf_uart_init
#define duet_flash_kv_init alto_flash_kv_init
#elif defined CFG_PLF_DUET
#define LEGA_UART0_INDEX DUET_UART0_INDEX
#define LEGA_UART1_INDEX DUET_UART1_INDEX
#define LEGA_UART2_INDEX DUET_UART2_INDEX
#else
#define duet_pinmux_config lega_pinmux_config
#define duet_uart_init lega_uart_init
#define duet_flash_kv_init lega_flash_kv_init
#define UART1_INDEX LEGA_UART1_INDEX
#endif

#define UART1_TX_PIN PAD2
#define UART1_RX_PIN PAD3

#ifdef CFG_PLF_RV32
extern asr_uart_dev_t lega_at_uart;
#elif defined CFG_PLF_DUET
extern duet_uart_dev_t lega_at_uart;
#else
extern lega_uart_dev_t lega_at_uart;
#endif

#ifdef __cplusplus
extern "C" {
#endif
void lega_sram_rf_pta_init(void);
void lega_recovery_phy_fsm_config(void);
void lega_wlan_efuse_read(void);
int lega_wlan_init(void);
void ota_roll_back_pro(void);
int32_t duet_flash_kv_init(void);
void lega_at_cmd_register_all(void);
int lega_at_init(void);
void at_handle_uartirq(char ch);
int asr_security_engine_init();
void at_uart_init(void);
#ifdef __cplusplus
}
#endif

void at_uart_init(void)
{
    memset(&lega_at_uart, 0, sizeof(lega_at_uart));

    lega_at_uart.config.baud_rate    = UART_BAUDRATE_115200;
    lega_at_uart.config.data_width   = DATA_8BIT;
    lega_at_uart.config.flow_control = FLOW_CTRL_DISABLED;
    lega_at_uart.config.parity       = PARITY_NO;
    lega_at_uart.config.stop_bits    = STOP_1BIT;
    lega_at_uart.config.mode         = TX_RX_MODE;
    lega_at_uart.port                = UART1_INDEX;
#if defined CFG_PLF_RV32 || defined CFG_PLF_DUET
    duet_pinmux_config(UART1_TX_PIN, PF_UART1);
    duet_pinmux_config(UART1_RX_PIN, PF_UART1);
#endif
    // register uart callback func for receiving at command
    lega_at_uart.priv = (void *) (at_handle_uartirq);
    duet_uart_init(&lega_at_uart);
}

void init_asrPlatform(void)
{
    // don't run any code before soc_pre_init.
    soc_pre_init();

    soc_init();

    duet_flash_kv_init();

    // uart init and register uart for receiving at command
    at_uart_init();

    // ota roll back,just for flash_remapping
    ota_roll_back_pro();

    // disable wdg
    ota_wdg_disable();

    // register uart for printf log, the used uart should be init before.
    printf_uart_register(LEGA_UART1_INDEX);
    // register uart for at log, the used uart should be init before.
    printf2_uart_register(LEGA_UART1_INDEX);

    // printf("\napp version: %s\n",LEGA_VERSION_STR);

    /* set EasyLogger log format */
#if (CFG_EASY_LOG_ENABLE == 1)
    elog_init();
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_start();
#endif
#if defined CFG_PLF_RV32 || defined CFG_PLF_DUET
    lega_wlan_efuse_read();

#ifdef CFG_PLF_DUET
    // set certifacation type before rf init, param 0x00 is old srrc, 0x10 is new srrc
    lega_wlan_set_cert_type(0x00);
#endif

    lega_sram_rf_pta_init();

    lega_recovery_phy_fsm_config();
#endif
    asr_security_engine_init();
#ifndef CONFIG_ENABLE_CHIP_SHELL
    lega_at_init();
#endif
    lega_at_cmd_register_all();

    lega_wlan_init();

    tcpip_init(NULL, NULL);
}
