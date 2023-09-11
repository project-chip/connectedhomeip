/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <wiced.h>

typedef enum
{
    ON_OFF_BUTTON,
    APP_MAX_BUTTON,
} application_button_t;

wiced_result_t app_button_init(void);
