/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- OTA Requestor Example App Config ----

// Buttons config
#define BUTTON_PORT DEVICE_DT_GET(DT_NODELABEL(gpioc))

#define BUTTON_PIN_1 2
#define BUTTON_PIN_3 3
#define BUTTON_PIN_4 1
#define BUTTON_PIN_2 0

// LEDs config
// System led config
#define SYSTEM_STATE_LED_PORT DEVICE_DT_GET(DT_NODELABEL(gpiob))
#define SYSTEM_STATE_LED_PIN 7
