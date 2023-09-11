/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "rtos/Thread.h"

namespace chip {
namespace rpc {

void RunRpcService();

rtos::Thread * Init();

} // namespace rpc
} // namespace chip
