/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/server/AppDelegate.h>
#include <stdint.h>

namespace Esp32AppServer {
void Init(AppDelegate * context = nullptr);
} // namespace Esp32AppServer
