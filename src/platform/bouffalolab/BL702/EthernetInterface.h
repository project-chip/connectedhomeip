/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void ethernetInterface_init(void);
void ethernetInterface_eventGotIP(struct netif * interface);
struct netif * deviceInterface_getNetif(void);

#ifdef __cplusplus
}
#endif
