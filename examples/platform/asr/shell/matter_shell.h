/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file provides the Shell implementation of Matter.
 *      It can be also used in ASR AT Command implementation.
 */

#ifndef __MATTER_SHELL_H__
#define __MATTER_SHELL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    WIFI_RESET = 0,
    FACTORY_RESET,
    COMMISSIONING_RESET,
} Reset_t;

void ShutdownChip();
void asr_matter_reset(Reset_t type);
void asr_matter_onoff(int value);
void asr_matter_sensors(bool enable, int temp, int humi, int pressure);
void asr_matter_ota(uint32_t timeout);
#if CONFIG_ENABLE_CHIP_SHELL
void RegisterLightCommands();
#endif
#ifdef __cplusplus
}
#endif
#endif // __MATTER_SHELL_H__
