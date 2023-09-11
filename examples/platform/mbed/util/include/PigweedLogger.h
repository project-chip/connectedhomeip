/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "rtos/Mutex.h"

namespace PigweedLogger {

rtos::Mutex * GetSemaphore();

} // namespace PigweedLogger
