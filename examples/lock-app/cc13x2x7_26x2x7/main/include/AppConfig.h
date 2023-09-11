/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Logging
#ifdef __cplusplus
extern "C" {
#endif

int cc13x2_26x2LogInit(void);
void cc13x2_26x2Log(const char * aFormat, ...);
#define PLAT_LOG(...) cc13x2_26x2Log(__VA_ARGS__);

#ifdef __cplusplus
}
#endif
#endif // APP_CONFIG_H
