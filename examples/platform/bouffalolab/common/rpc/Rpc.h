/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {
namespace rpc {

class LightingService;

void Init();
void RunRpcService(void *);

} // namespace rpc
} // namespace chip
