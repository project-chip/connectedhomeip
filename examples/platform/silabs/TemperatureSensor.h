/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "sl_status.h"
#include <stdint.h>

namespace TemperatureSensor {
sl_status_t Init();
sl_status_t GetTemp(uint32_t * relativeHumidity, int16_t * temperature);
}; // namespace TemperatureSensor
