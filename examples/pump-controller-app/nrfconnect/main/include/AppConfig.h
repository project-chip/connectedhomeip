/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Pump Controller Example App Config ----

#define START_BUTTON DK_BTN2
#define START_BUTTON_MASK DK_BTN2_MSK
#define FUNCTION_BUTTON DK_BTN1
#define FUNCTION_BUTTON_MASK DK_BTN1_MSK
#define BLE_ADVERTISEMENT_START_BUTTON DK_BTN4
#define BLE_ADVERTISEMENT_START_BUTTON_MASK DK_BTN4_MSK

#define SYSTEM_STATE_LED DK_LED1
#define PUMP_STATE_LED DK_LED2
#define FACTORY_RESET_SIGNAL_LED DK_LED3
#define FACTORY_RESET_SIGNAL_LED1 DK_LED4

// Time it takes in ms for the simulated pump to move from one state to another.
#define PUMP_START_PERIOS_MS 2000
