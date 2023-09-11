/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "wiced.h"

typedef enum
{
    ON_OFF_BUTTON,
    APP_MAX_BUTTON,
} application_button_t;
#define APP_MAX_BUTTON_DEF 1 // define for preprocessor

wiced_result_t app_button_init(void);
