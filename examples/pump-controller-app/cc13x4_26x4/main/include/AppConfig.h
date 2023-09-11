/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Logging
#ifdef __cplusplus
extern "C" {
#endif

int cc13xx_26xxLogInit(void);
void cc13xx_26xxLog(const char * aFormat, ...);
#define PLAT_LOG(...) cc13xx_26xxLog(__VA_ARGS__);

#ifdef __cplusplus
}

#endif
#endif // APP_CONFIG_H
