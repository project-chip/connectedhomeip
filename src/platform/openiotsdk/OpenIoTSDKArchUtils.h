/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines timing helper functions.
 */

#pragma once

#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t GetTick(void);

/* Time to kernel ticks */
uint32_t sec2tick(uint32_t sec);

uint32_t ms2tick(uint32_t ms);

uint32_t us2tick(uint32_t usec);

/* Kernel ticks to time */
uint64_t tick2sec(uint64_t tick);

uint64_t tick2ms(uint64_t tick);

uint64_t tick2us(uint64_t tick);

#ifdef __cplusplus
}
#endif
