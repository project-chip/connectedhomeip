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
#ifdef CONFIG_ENABLE_CHIP_SHELL
void RegisterLightCommands();
#endif
#ifdef __cplusplus
}
#endif
#endif // __MATTER_SHELL_H__
