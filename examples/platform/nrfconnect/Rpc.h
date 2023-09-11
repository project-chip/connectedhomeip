/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/kernel.h>

namespace chip {
namespace rpc {

class NrfButton;

void RunRpcService(void *, void *, void *);

k_tid_t Init();

} // namespace rpc
} // namespace chip
