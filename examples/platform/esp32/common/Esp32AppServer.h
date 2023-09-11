/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/server/AppDelegate.h>
#include <stdint.h>

namespace Esp32AppServer {
void DeInitBLEIfCommissioned(void);
void Init(AppDelegate * context = nullptr);
} // namespace Esp32AppServer
