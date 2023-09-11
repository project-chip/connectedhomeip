/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>

namespace PigweedLogger {

k_sem * GetSemaphore();

} // namespace PigweedLogger
