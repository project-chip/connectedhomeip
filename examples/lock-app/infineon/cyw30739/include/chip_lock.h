/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>

// Application-defined error codes in the CHIP_ERROR space
#define APP_ERROR_INIT_TIMER_FAILED CHIP_ERROR_NO_MEMORY // should use CHIP_APPLICATION_ERROR when it's ready
#define APP_ERROR_ALLOCATION_FAILED CHIP_APPLICATION_ERROR(0x07)

// Application configurations
#define ACTUATOR_MOVEMENT_PERIOS_MS 1000
