/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _POWERCYCLE_COUNTING_H_
#define _POWERCYCLE_COUNTING_H_

#include "inttypes.h"

#ifdef __cplusplus
extern "C" {
#endif
void gpAppFramework_Reset_Init(void);
uint8_t gpAppFramework_Reset_GetResetCount(void);
void gpAppFramework_Reset_cbTriggerResetCountCompleted(void);

#ifdef __cplusplus
}
#endif

#endif // _POWERCYCLE_COUNTING_H_
