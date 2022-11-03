/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Lighting Example App Config ----

#define LIGHTING_BUTTON DK_BTN2
#define LIGHTING_BUTTON_MASK DK_BTN2_MSK
#define FUNCTION_BUTTON DK_BTN1
#define FUNCTION_BUTTON_MASK DK_BTN1_MSK
#define BLE_ADVERTISEMENT_START_BUTTON DK_BTN4
#define BLE_ADVERTISEMENT_START_BUTTON_MASK DK_BTN4_MSK

#define SYSTEM_STATE_LED DK_LED1 // led0 in device tree

// Time it takes in ms for the simulated actuator to move from one state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000
