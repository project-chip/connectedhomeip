/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "pw_rpc/server.h"

namespace chip {
namespace rpc {

class Mutex
{
public:
    virtual void Lock()   = 0;
    virtual void Unlock() = 0;
    virtual ~Mutex() {} // Virtual Destructor
};

void Start(void (*RegisterServices)(pw::rpc::Server &), ::chip::rpc::Mutex * uart_mutex_);

} // namespace rpc
} // namespace chip
