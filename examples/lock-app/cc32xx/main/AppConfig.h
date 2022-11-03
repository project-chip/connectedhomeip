/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Logging
#ifdef __cplusplus
extern "C" {
#endif

int cc32xxLogInit(void);
void cc32xxLog(const char * aFormat, ...);
#define PLAT_LOG(...) cc32xxLog(__VA_ARGS__);

#ifdef __cplusplus
}
#endif
#endif // APP_CONFIG_H
