/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <lib/core/CHIPError.h>

namespace chip {
namespace rpc {

CHIP_ERROR Init();
void RunRpcService(void *);

} // namespace rpc
} // namespace chip
