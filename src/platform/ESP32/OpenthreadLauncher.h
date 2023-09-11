/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <esp_err.h>
#include <esp_openthread_types.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t set_openthread_platform_config(esp_openthread_platform_config_t * config);
esp_err_t openthread_init_stack(void);
esp_err_t openthread_launch_task(void);

#ifdef __cplusplus
}
#endif
